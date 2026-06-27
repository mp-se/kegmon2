# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

KegMon is an ESP32-S3 firmware project for monitoring keg weights (up to 4 scales) via HX711 load cells. Built with PlatformIO/Arduino framework. Detects pours via slope analysis, tracks state per scale, and integrates with InfluxDB, Home Assistant, Brewfather, and Brewspy.

## Build Commands

```bash
# Build primary target (ILI9341 TFT + SD card)
pio run -e kegmon-esp32s3-pro-tft-sd

# Build and upload
pio run -e kegmon-esp32s3-pro-tft-sd --target upload

# Serial monitor
pio device monitor -e kegmon-esp32s3-pro-tft-sd

# Build hardware test variant
pio run -e kegmon-esp32s3-pro-hardware-test

# Run native unit tests (Google Test, no device required)
cd test && mkdir -p build && cd build && cmake .. && make && ./changedetection_integration_test
```

## UI Development

The HTML/JS frontend (Vue 3 / Vite / Pinia) lives in `ui/`. After building the UI, copy built assets:

```bash
./copy_ui.sh   # copies dist/assets into html/ as .gz files
```

The `.gz` files in `html/` are embedded directly into firmware via `board_build.embed_txtfiles`.

```bash
# Inside ui/
npm run dev          # dev server (hot reload against mock API)
npm run mock         # start mock REST server (nodemon) for local UI dev without device
npm run build        # production build → dist/
npm run test         # Vitest unit tests (run once)
npm run test:watch   # Vitest in watch mode
npm run lint         # ESLint --fix
npm run format       # Prettier
```

The mock server in `ui/mock-server/` simulates the firmware REST API so the UI can be developed without a connected device.

## PlatformIO Environments

| Environment | TFT Driver | SD Card |
|---|---|---|
| `kegmon-esp32s3-pro-tft-sd` | ILI9341 (240×320) | Yes |
| `kegmon-esp32s3-pro-tft-9488-sd` | ILI9488 (480×320) | Yes |
| `kegmon-esp32s3-pro-tft-9341-sd` | ILI9341 (240×320) | No |
| `kegmon-esp32s3-pro-hardware-test` | ILI9341 | No (test build) |

One firmware binary supports all hardware options via compile-time defines.

## Architecture

### Core Data Flow
```
HX711 (10 SPS) → ScaleFilterPipeline → ChangeDetection (state machine) → EventQueue → Web/InfluxDB/Display
```

### Key Source Files

- **`src/main.cpp`** — Entry point, FreeRTOS task setup
- **`src/changedetection.hpp/.cpp`** — Per-scale 9-state machine + pour detection; the core algorithmic component
- **`src/scale.hpp/.cpp`** — HX711 interface, raw ADC reading, calibration
- **`src/scale_filter_pipeline.hpp/.cpp`** — Chains filter algorithms; two pipelines per scale (stability + pour detection)
- **`src/kegconfig.hpp/.cpp`** — All configuration (4 scales × per-keg settings + global settings); persisted to LittleFS
- **`src/kegwebhandler.hpp/.cpp`** — REST API handlers (`/api/scale`, `/api/status`, `/api/statistic`, `/api/config`, etc.)
- **`src/event_logger.hpp/.cpp`** — 64-slot circular event queue; thread-safe
- **`src/display.hpp/.cpp`** + **`src/ui_kegmon.hpp/.cpp`** — LVGL-based TFT UI
- **`src/filters/`** — 12 filter implementations (Kalman, EMA, Median, Butterworth, etc.), all inheriting `filter_base.hpp`

### State Machine (per scale, in `changedetection`)

9 states: `Idle → Stabilizing → Stable ↔ Pouring → Restabilizing → Stable` plus `KegAbsent`, `ReplacingKeg`, `InvalidWeight`, `CalibrationNeeded`.

Critical design: baseline is **not** locked when Pouring exits — it is deferred to Restabilizing state where slope ≈ 0 detection confirms the weight has truly settled (accounts for Kalman filter lag). Extended 5× timeout fallback prevents deadlock.

Pour detection triggers when slope < −0.002 kg/sec AND within realistic bounds [−0.15, −0.008] kg/sec.

### External Integrations

- **InfluxDB** — continuous push of weight, volume, state, events
- **Home Assistant** — MQTT publish
- **Brewfather / Brewspy** — HTTP fetch of brew metadata; Brewspy also receives volume updates
- **OTA** — firmware update via web UI upload

### Libraries (key)

- `espframework` (mp-se) — WiFi/config base
- `lvgl v9.5` — TFT UI
- `TFT_eSPI` — TFT hardware driver
- `ArduinoJson v7` — JSON serialization
- `ESPAsyncWebServer` + `AsyncTCP` — non-blocking web server
- `HX711` (RobTillaart) — load cell ADC
- `OneWireNg` + `Arduino-Temperature-Control-Library` — DS18B20 temp sensors

## Testing

Unit tests live in `test/` and use CMake + GoogleTest. They test core logic extracted from `changedetection.cpp` without Arduino dependencies. No mocking of hardware — logic is extracted directly.

## Specs

Detailed feature specs and state machine requirements live in `spec/`:
- `STATE_MACHINE_REQUIREMENTS.md` — authoritative requirements for `changedetection`
- `KEGMON_FEATURES_OVERVIEW.md` — full feature inventory
- `EVENT_LOGGING_FORMAT.md` — InfluxDB/event schema

## Notes

- SD card mount fails after initial flash — power cycle the device to recover.
- `INJECT_TEST_EVENT=1` build flag (commented out in `platformio.ini`) enables synthetic event injection for testing.
- `script/git_rev.py` injects git revision into `CFG_GITREV` at build time (currently commented out in favor of static `"prototype"`).
