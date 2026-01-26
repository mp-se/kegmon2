.. _configuration:

Configuration
-------------

This guide explains how to configure KegMon through its web interface. Access the device via a web browser using its IP address or mDNS name (e.g., kegmon.local). The interface is divided into sections for device settings, taps, integrations, and tools.

Index
*****

The main dashboard displays general device information, including temperatures and status. Only available data is shown.

.. image:: images/ui_home.png
  :width: 600
  :alt: Index page

Device - Settings
*****************

Configure general device preferences here.

.. image:: images/ui_device_settings.png
  :width: 600
  :alt: Device Settings

* **MDNS**: Set a custom name for the device on the network (requires mDNS support).
* **Temperature format**: Choose Celsius or Fahrenheit for temperature display.
* **Weight unit**: Select kg or lbs for weight measurements.
* **Volume unit**: Choose cl, UK fl. oz, or US fl. oz for volume display.
* **Dark Mode**: Toggle between light and dark UI themes (also available via the menu bar).

Device - Calibration
********************

Calibrate your scales in three easy steps for accurate weight measurements.

.. image:: images/ui_device_calibration.png
  :width: 600
  :alt: Device calibration

* **STEP 1 - Tare scale**: Select the scale from the dropdown and ensure it's empty, then tare to zero.
* **STEP 2 - Calculate factor**: Place an object of known weight on the scale and enter its weight. The software calculates the calibration factor.
* **STEP 3 - Validate**: Place another object of known weight and verify the reading. If accurate, calibration is complete.

Device - Stability
******************

Monitor the stability of your hardware setup to ensure reliable readings.

.. image:: images/ui_device_stats.png
  :width: 600
  :alt: Statistics

This page helps diagnose unstable builds. Keep the browser open to view historical data (raw, Kalman-filtered, and stable values) for analysis. Data is stored in the browser and resets on refresh.

Device - WiFi
*************

Manage WiFi connections and settings.

.. image:: images/ui_device_wifi.png
  :width: 600
  :alt: Device WiFi

* **SSID #1/Password #1**: Primary network credentials.
* **SSID #2/Password #2**: Secondary network for fallback (optional).
* **Portal timeout**: Time (in seconds) the WiFi portal remains active when triggered (e.g., by tapping reset 2-3 times within 3 seconds).
* **Connect timeout**: Maximum time allowed for WiFi connection attempts.

Taps - Settings
***************

Configure individual tap settings for volume calculations and beer details.

.. image:: images/ui_taps_settings.png
  :width: 600
  :alt: Tap settings

* **Empty keg weight**: Weight of the empty keg, used to calculate remaining beer volume.
* **Glass volume**: Standard glass size for estimating remaining pours.
* **Keg volume**: Standard keg size for validating weight and calculating remaining volume.
* **Temperature sensor**: Connect a temperature sensor to the specific scale base. If not used the first sensor value will be used.

Taps - Beers
************

View and manage beers on tap, with options to import from integrations.

.. image:: images/ui_taps_beer.png
  :width: 600
  :alt: Tap beers

Use buttons to import data from BrewSpy or Brewfather. Beer name, FG, EBC, ABV and IBU are imported if available

Taps - History
**************

Review recent pour history for each tap.

.. image:: images/ui_taps_history.png
  :width: 600
  :alt: Tap history

Displays the latest pours for beers on tap.

Integration - Home Assistant
****************************

Set up MQTT for Home Assistant integration.

.. image:: images/ui_post_ha.png
  :width: 600
  :alt: Home Assistant integration

Configure MQTT server details for data export to Home Assistant.

Integration - Brewfather
************************

Connect to Brewfather for brew data import.

.. image:: images/ui_post_brewfather.png
  :width: 600
  :alt: Brewfather integration

Enter API and user keys for Brewfather access.

Integration - Brewspy
*********************

Link to BrewSpy for keg and beer data.

.. image:: images/ui_post_brewspy.png
  :width: 600
  :alt: Brewspy integration

Enter tokens for kegs. Copy the webhook URL code from BrewSpy and paste it here.

Integration - Barhelper
***********************

Integrate with Barhelper for additional monitoring.

.. image:: images/ui_post_barhelper.png
  :width: 600
  :alt: Barhelper integration

Enter API keys and monitor names. Obtain the key from Barhelper settings (save it securely, as it's not retrievable later). Monitors are added automatically on first run.

Integration - Influx
********************

Send data to InfluxDB for analysis.

.. image:: images/ui_post_influxdb.png
  :width: 600
  :alt: Influx integration

Configure InfluxDB v2 for logging scale and internal parameters. If enabled data will be logged every second for all the scales and paramaters. 

Serial Console
**************

Access device logs for troubleshooting.

.. image:: images/ui_other_serial.png
  :width: 600
  :alt: Serial console

View real-time serial output from the device.

Backup & Recovery
*****************

Manage configuration backups.

.. image:: images/ui_other_backup.png
  :width: 600
  :alt: Backup configuration

Export current settings or restore from a saved JSON file.

Firmware Update
***************

Update device firmware over-the-air.

.. image:: images/ui_other_firmware.png
  :width: 600
  :alt: Upload firmware

Upload new firmware files without serial connection.

Support
*******

Access diagnostic information.

.. image:: images/ui_other_support.png
  :width: 600
  :alt: Support information

View logs and hardware details.

Tools
*****

Interact with the device's file system.

.. image:: images/ui_other_tools.png
  :width: 600
  :alt: Tools

Manage files on the device.
