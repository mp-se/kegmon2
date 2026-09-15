.. _releases:

Releases 
########

v2.0.0 - alfa 1 
===============

* New redesigned hardware and PCB for supporting more than 2 scales (PCB v1.1 required)
* New scale reading and filtering engine to improve accuracy and pour detection. 
* Event based engine so now its possible to send notification to HA when entering pouring state or any other state.
* Upgraded depdendencies to latest versions

Current source configuration
----------------------------

The PlatformIO configuration identifies the current v2.0.0 build as ``alfa 1``.
This documentation tracks the source tree rather than a separately tagged
release artifact.

Dropped support for

* DHT22 and BME temperature sensors (use DS18B20 instead).
* LiquidCrystal I2C displays.
* OLED displays.
* ESP8266 and ESP32 mini boards

Not yet completed:

* BLE temperature sensor support
* Additional TFT display options
