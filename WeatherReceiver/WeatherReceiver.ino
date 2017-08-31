#include <SPI.h>
#include <Wire.h>
#include <RH_RF95.h>
#include <ArduinoJson.h>

#define LED 9
#define DEBUG false

RH_RF95 rf95;
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void setup() {
  pinMode(LED, OUTPUT);

  Serial.begin(115200);
  while (!Serial) ; // Wait for serial port to be available
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

  if (!rf95.init()) {
    Serial.println("RF95 init failed!");
  }
  rf95.setTxPower(23, false);
  rf95.setFrequency(915.0);
  if (!rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096)) {
    Serial.println("set config failed");
  }

}

void printData(unsigned char *buf) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf);

  float temp = json["temp"];
  float humidity = json["hum"];
  Serial.println("TEMP: ");
  Serial.print(temp);
  Serial.println(" F");
  Serial.println("HUM: ");
  Serial.print(humidity);
  Serial.println(" %");
}

void loop() {
  if (rf95.available()) {
    // Should be a message for us now   
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len)) {
      digitalWrite(LED, HIGH);
      delay(50);
      digitalWrite(LED, LOW);

      buf[len] = '\0';
      if (DEBUG) printData(buf);

      // simply write the data over Serial to the IoT Experimenter
      Serial.println((char *)buf);
    } else {
      Serial.println("recv failed");
    }
  }
}


