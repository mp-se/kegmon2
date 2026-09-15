.. _development:

Development
===========

Contributing to KegMon2
-----------------------

KegMon is an ESP32-S3 Arduino/PlatformIO project. The web UI is a Vue 3/Vite
application in ``ui/``; its production assets are embedded in the firmware.
The documentation is built with Sphinx.

The native change-detection tests use GoogleTest and do not need a device.
PlatformIO environments provide the firmware and hardware-test builds.

Getting Started
---------------

To contribute:

1. Set up the development environment using the tools below.
2. Fork the repository and create a feature branch.
3. Run the relevant UI, native-test, documentation, or PlatformIO validation.
4. Submit a pull request with a clear description.

Required Tools
--------------

* **VS Code**: Primary code editor. Download from https://code.visualstudio.com/.
* **PlatformIO**: Extension for VS Code to handle Arduino ESP32 development. Install via VS Code marketplace or https://platformio.org/.
* **Git**: Version control. For Windows, use https://gitforwindows.org/; included with macOS/Linux.

Useful commands
~~~~~~~~~~~~~~~

Build the primary firmware target::

  pio run -e kegmon-esp32s3-pro-tft-sd

Run native change-detection tests::

  cd test && mkdir -p build && cd build && cmake .. && make && ./changedetection_integration_test

Build the web UI and copy its embedded assets::

  cd ui && npm run build
  cd .. && ./copy_ui.sh

Build the documentation::

  cd src_docs && make html

For more details, refer to the repository's README or issue tracker.
