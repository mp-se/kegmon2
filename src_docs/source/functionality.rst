.. _functionality:

Functionality
=============

KegMon is a comprehensive keg monitoring system designed for homebrewers, providing real-time tracking of beer levels, pours, and environmental conditions. Key features include:

* **Modern Web-Based Interface**

  A modern HTML5 web interface accessible from any device, enabling easy configuration of settings, real-time monitoring of keg status, and management of beer data without needing direct hardware access.

* **Graphical Interface on TFT Screen**

  An adaptive graphical user interface displayed on the device's TFT screen, which dynamically adjusts based on the number of installed scales to provide clear, intuitive navigation and data visualization.

* **3-Step Easy Scale Calibration**

  A straightforward 3-step calibration process performed entirely through the web interface: tare the scale to zero, calculate the calibration factor using a known weight, and verify accuracy for reliable measurements.

* **3D Printable Bases for Kegs**

  Includes 3D models for custom keg bases, or compatibility with other commercially available load cell bases, allowing flexible hardware setup for various keg sizes and types.

* **Hardware Support for Up to 4 Scales**

  Designed to support up to 4 scales simultaneously, with each scale capable of displaying real-time statistics on an attached display, enabling monitoring of multiple kegs in a single system.

* **Stable Level Detection**

  Advanced software algorithms that detect and confirm stable liquid levels in each keg, filtering out noise from movement or environmental factors for accurate readings.

* **Pour Detection**

  Intelligent detection of pour events, tracking when beer is dispensed to log consumption and update remaining volume calculations automatically.

* **Volume Estimation**

  Calculates remaining beer volume in terms of glasses or pints, factoring in the keg's weight, final gravity, and the pre-configured weight of an empty keg for precise inventory management.

* **Home Assistant Integration**

  Seamlessly integrates with Home Assistant for automated data export, allowing users to view current beer selections, last pour details, remaining volume, and other metrics within their smart home dashboard.

* **BrewSpy Integration**

  Connects to BrewSpy to fetch and import brew data, automatically updating keg levels and beer information when batches are changed or new brews are added.

* **Brewfather Integration**

  Pulls data from Brewfather on recent brews, simplifying the import of batch details, recipes, and gravity readings for streamlined setup and tracking.

* **Easy Software Updates**

  Supports over-the-air software updates directly through the web UI, ensuring the device stays current with the latest features and bug fixes without physical access.

* **Temperature Sensors**

  Supports DS18B20 temperature sensors placed in each keg base, providing real-time temperature monitoring to help maintain optimal beer storage conditions.

* **Backup and Restore of Settings**

  Allows full backup and restore of device configurations as JSON files, useful for recovery from hardware issues or replicating setups across multiple devices by editing the "id" fields to match each unit's unique ID (available on the index page).

* **Unit Preferences**

  Flexible unit selection for temperature (Celsius or Fahrenheit), weight (kg or lbs), and volume (cl, UK fl. oz, or US fl. oz), with all internal calculations using metric units (Celsius, kg, cl) and automatic conversion for display.
