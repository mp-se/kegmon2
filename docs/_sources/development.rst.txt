.. _development:

Development
===========

Contributing to KegMon
-----------------------

Development for KegMon is conducted using VS Code with the PlatformIO extension for Arduino-based ESP32 programming. The documentation is built with Sphinx and hosted on GitHub Pages.

We use pre-commit hooks to enforce coding standards, ensuring consistent formatting and quality C++ code. All pull requests must pass pre-commit validation before merging.

The CI/CD pipeline relies on GitHub Actions for automated firmware builds and documentation publishing.

Testing includes a unit test target that currently requires hardware but is planned to migrate to Wokwi's GitHub Actions for simulation. A Python script validates API outputs to ensure correctness.

Getting Started
---------------

To contribute:

1. Set up the development environment using the tools below.
2. Fork the repository and create a feature branch.
3. Make changes, ensuring pre-commit passes.
4. Submit a pull request with a clear description.

Required Tools
--------------

* **VS Code**: Primary code editor. Download from https://code.visualstudio.com/.
* **PlatformIO**: Extension for VS Code to handle Arduino ESP32 development. Install via VS Code marketplace or https://platformio.org/.
* **Pre-commit**: Enforces code standards. Install via pip (``pip install pre-commit``) and run ``pre-commit install`` in the repo. Requires Linux/WSL on Windows.
* **GitHub CLI**: For source code management. Install from https://cli.github.com/.
* **Git**: Version control. For Windows, use https://gitforwindows.org/; included with macOS/Linux.

For more details, refer to the repository's README or issue tracker.

