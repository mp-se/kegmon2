
![download](https://img.shields.io/github/downloads/mp-se/kegmon2/total) 
![release](https://img.shields.io/github/v/release/mp-se/kegmon2?label=latest%20release)
![issues](https://img.shields.io/github/issues/mp-se/kegmon2)
![pr](https://img.shields.io/github/issues-pr/mp-se/kegmon2)
![dev_build](https://img.shields.io/github/actions/workflow/status/mp-se/kegmon2/pio-build.yaml?branch=dev)
![doc_build](https://img.shields.io/github/actions/workflow/status/mp-se/kegmon2/doc-build.yaml?branch=master)
![License](https://img.shields.io/github/license/mp-se/kegmon2)
![GitHub Stars](https://img.shields.io/github/stars/mp-se/kegmon2)
![Last Commit](https://img.shields.io/github/last-commit/mp-se/kegmon2)

# KegMon - Monitoring the volume in your kegs

![KegMon Logo](src_docs/source/images/kegmon_logo_s.png)

This is a project that I have done for my own Keezer, if you like it please feel free to suggest improvements. 

For docs see: https://mp-se.github.io/kegmon2/index.html

# Hardware

* Supports Lolin ESP32 S3 PRO with Lolin TFT 320x200
* Loadcells and HX711 ADC converters
* DS18B20

# Features

* Can measure weight from 1 up to 4 scales
* **Advanced pour detection:** Dual-stage detection with slope validation and realistic flow rate bounds (-0.15 to -0.008 kg/sec)
* **Smart state machine:** 9-state system with intelligent weight stabilization and keg lifecycle tracking
* Real-time event system (pour detection, keg replacement, stabilization)
* Comprehensive statistics tracking (pour volume, frequency, state durations)
* Stable scale presentation (13 different filter algorithms including Kalman, EMA, Median)
* Support temperature/humidity sensor in one scale base
* Integration with Brewfather to retrive data on brew
* Integration with Brewspy to retrive data on brew and also update the remaning volume and pours
* Integration with Home Assistant
* InfluxDB export with event logging
* Weights and Volumes in Metric, Imperial (both US and UK)
* Pour detection for pours over 100 ml with configurable slope thresholds
* Estimation on how many glasses remain in the kegs
* Firmware update via UI
* One build for all hardware options
* Modern HTML5 UI
* Easy scale calibration
* 3D models and PCB

# Known Issues

When flashing the ESP32 s3 PRO the SD card mounting will likley fail, just turn power off/on and it will work again.
