# lora-weather-station
Remote weather station with LoRa radio communication to base station.

There are 3 parts to this project:

* WeatherSensor: Arduino code that runs on a board with a LoRa module. Transmits sensor readings to base station.
* WeatherReceiver: Arduino code that runs on a board with LoRa module on the base station. Receives sensor data and writes it to serial to the gateway board.
* LoRaGateway: Code for IoT Experimenter board that bridges the LoRa data to the Internet over Wi-Fi.


[See the full project for all the details](https://nootropicdesign.com/projectlab/2017/09/03/solar-powered-lora-weather-station/)
