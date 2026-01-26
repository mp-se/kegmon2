.. _releases:

Releases 
########

v2.0.0
======

* New redesigned hardware and PCB for supporting more than 2 scales. 
* New scale reading and filtering engine to improve accuracy and pour detection. 
* Event based engine so now its possible to send notification to HA when entering pouring state or any other state.
* Upgraded depdendencies to latest versions

Dropped support for

* DHT22 and BME temperature sensors (use DS18B20 instead).
* LiquidCrystal I2C displays.
* OLED displays.
* ESP8266 and ESP32 mini boards

Not yet completed:

* BLE temperature sensor support
* Additional TFT display options
