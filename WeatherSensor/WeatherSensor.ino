#include <SPI.h>
#include <Wire.h>
#include <RH_RF95.h>
#include <TH02.h>
#include <ArduinoJson.h>

#define LED 9

RH_RF95 rf95;
TH02 th02(TH02_I2C_ADDR);
boolean TH02_found = false;

void printhex(uint8_t c) {
  if (c<16) Serial.print('0');
  Serial.print(c, HEX);
}

int i2cScan() {
  byte error, address;
  int nDevices;

  Serial.println("Scanning I2C bus ...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(F("I2C device found at address 0x"));
      printhex(address);
      
      if (address == 0x40) {
        Serial.println(F("-> TH02 !"));
        TH02_found = true;
      } else {
        Serial.println(F("-> Unknown device !"));
      }
      nDevices++;
    } else if (error==4) {
      Serial.print(F("Unknow error at address 0x"));
      printhex(address);
    }    
  }
  if (nDevices == 0)
    Serial.println(F("No I2C devices found\n"));
  else
    Serial.println(F("Scan done"));

  return nDevices;
}

void findSensor() {
  uint8_t devID;
  uint8_t err;
  uint8_t status;
  uint8_t config;

while (!TH02_found) {
    // scan I2C bus
    i2cScan();

    // We found it ?
    if (TH02_found) {
      // TH02 ID 
      err = th02.getId(&devID);

      if (err) {
        Serial.print(F("TH02 getId error = 0x"));
        printhex(err);
        Serial.println();
      } else {
        Serial.print(F("TH02 device ID = 0x"));
        printhex(devID);
        Serial.println();

        if (devID == 0x50) {
          Serial.println(F("TH02 device ID match !"));

          if ( (err=th02.getStatus(&status)) != 0) {
            Serial.print(F("TH02 Status error = 0x"));
            printhex(err);
          } else {
            Serial.print(F("TH02 Status = 0x"));
            printhex(status);
          }
          Serial.println();

          if ( (err=th02.getConfig(&config)) != 0) {
            Serial.print(F("TH02 Config error = 0x"));
            printhex(err);
          } else {
            Serial.print(F("TH02 Config = 0x"));
            printhex(config);
          }
          Serial.println();
        }
      }
    }

    // wait until next search
    if (!TH02_found) {
      Serial.println(F("Will retry to find TH02 in 5 sec."));
      delay(5000);
    }
  } // While not found
}



void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  while (!Serial) ; // Wait for serial port to be available
  if (!rf95.init())
    Serial.println("init failed");
  rf95.setTxPower(23, false);
  rf95.setFrequency(915.0);

  // Possible configurations:
  // Bw125Cr45Sf128 (the chip default)
  // Bw500Cr45Sf128
  // Bw31_25Cr48Sf512
  // Bw125Cr48Sf4096

  Wire.begin();


  // long range configuration requires for on-air time
  boolean longRange = false;
  if (longRange) {
    RH_RF95::ModemConfig modem_config = {
      0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
      0xC4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
      0x08  // Reg 0x26: LowDataRate=On, Agc=Off.  0x0C is LowDataRate=ON, ACG=ON
    };
    rf95.setModemRegisters(&modem_config);
    if (!rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096)) {
      Serial.println("set config failed");
    }
  }

  findSensor();
}

void loop() {
  int16_t temp, rh, rh_comp;
  uint8_t duration;
  uint8_t data[64];
  StaticJsonBuffer<200> jsonBuffer;

  th02.startTempConv();
  duration = th02.waitEndConversion();
  
  // Get temperature calculated and rounded
  temp = th02.getConversionValue();

  if (temp == TH02_UNDEFINED_VALUE) {
    Serial.print(F("Error reading value="));
    Serial.println(temp);
  } else {
    Serial.print(temp/10.0);
    Serial.println(F(" C"));
  }
  
  // Convert humidity
  th02.startRHConv();
  duration = th02.waitEndConversion();

  // Get temperature calculated and rounded with no compensation
  rh = th02.getConversionValue();

  if (rh == TH02_UNDEFINED_VALUE) {
    Serial.print(F("Error reading value="));
    Serial.println(rh);
  }

  JsonObject& json = jsonBuffer.createObject();

  temp = ((temp/10.0) * 1.8) + 32; // convert to Farenheit (I'm an American)
  temp = roundf(temp * 100) / 100;
  json["temp"] = temp;
  float humidity = roundf(th02.getConpensatedRH(true)) / 10;
  Serial.println(humidity);
  json["hum"] =  humidity;
  json.printTo((char *)data, sizeof(data));
  
  Serial.print("json = ");
  Serial.println((char *)data);

  digitalWrite(LED, HIGH);
  delay(5);
  digitalWrite(LED, LOW);

  rf95.send(data, strlen((char *)data));
  rf95.waitPacketSent();
  delay(10000);
}


