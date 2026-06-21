# Event Logging Format

## Overview

Kegmon logs all system events to CSV files on the SD card for persistent event history, analysis, and debugging. Each event is logged as a single CSV row with a version number for schema compatibility and extensibility.

## File Organization

### Log Files

- **Current log**: `/data.csv`
- **Rotation files**: `/data1.csv`, `/data2.csv`, `/data3.csv`
- **File size limit**: 16 KB (configurable via `MAX_LOG_FILE_SIZE`)
- **Max rotated files**: 4 files total (configurable via `MAX_LOG_FILES`)

### File Rotation Behavior

When `/data.csv` exceeds 16 KB:
1. `/data3.csv` is deleted (oldest)
2. `/data2.csv` → `/data3.csv`
3. `/data1.csv` → `/data2.csv`
4. `/data.csv` → `/data1.csv`
5. New empty `/data.csv` is created

This ensures you always have the 4 most recent log files with recent events accessible in `/data.csv`.

## CSV Format

### Structure: Version Per Row

Each row contains a version number as the first column for schema evolution:

```
Version,Timestamp,Scale,EventType,StableWeight,StableVolume,PrePourWeight,PostPourWeight,PourWeight,PourVolume,DurationMs,AvgSlope,PrevWeight,CurrWeight,SignalErrorReason,SignalQuality,Variance,ConsecutiveErrors
```

### Column Definitions

| # | Column | Type | Description | Units |
|---|--------|------|-------------|-------|
| 1 | **Version** | int | Schema version for parser evolution | - |
| 2 | **Timestamp** | string | ISO 8601 datetime (RTC-based) | YYYY-MM-DD HH:MM:SS |
| 3 | **Scale** | int | Scale/unit number (1-4 for U1-U4) | - |
| 4 | **EventType** | string | Event type name | - |
| 5 | **StableWeight** | float | Baseline weight when stable detected | kg |
| 6 | **StableVolume** | float | Calculated volume at stable baseline | L |
| 7 | **PrePourWeight** | float | Weight before pour started | kg |
| 8 | **PostPourWeight** | float | Weight after pour completed | kg |
| 9 | **PourWeight** | float | Weight change during pour | kg |
| 10 | **PourVolume** | float | Volume change during pour | L |
| 11 | **DurationMs** | uint64 | Event duration (stabilization or pour) | milliseconds |
| 12 | **AvgSlope** | float | Average pour rate | kg/sec |
| 13 | **PrevWeight** | float | Previous weight (for KEG_REMOVED) | kg |
| 14 | **CurrWeight** | float | Current weight (for KEG_REPLACED) | kg |
| 15 | **SignalErrorReason** | string | Error type (for LOAD_CELL_ERROR/RECOVERED) | - |
| 16 | **SignalQuality** | uint8 | Signal quality percentage | 0-100 |
| 17 | **Variance** | float | Reading variance | kg² |
| 18 | **ConsecutiveErrors** | uint16 | Error count | - |

### Empty Fields

Columns that don't apply to an event are left empty (consecutive commas), maintaining uniform row structure for easy CSV parsing.

## Event Types

All 16 event types log with identical column structure. Relevance varies by type:

### 1. SYSTEM_STARTUP

Marks system initialization/restart. Use this to identify session boundaries.

**Relevant Columns**: Timestamp, Scale, EventType only

**Example**:
```
1,2026-01-18 10:00:00,1,SYSTEM_STARTUP,0,0,0,0,0,0,0,0,0,0,,0,0,0
```

### 2. SETTLING_STARTED

Weight settling begins (initial stabilization from IDLE or re-stabilization after pour/change).

**Relevant Columns**: Timestamp, Scale, EventType, CurrWeight, DurationMs (0)

**Example**:
```
1,2026-01-18 10:00:05,1,SETTLING_STARTED,0,0,0,0,0,0,0,0,0,23.5,,0,0,0
```

### 3. STABLE_LEVEL

Keg baseline locked, ready for pouring. Fired after a weight change once the sensor signal has remained stable (slope < ±0.0015 kg/s) for at least 6 consecutive seconds.

**Relevant Columns**: Timestamp, Scale, EventType, StableWeight, StableVolume, DurationMs (will be >= 6000ms)

**Example**:
```
1,2026-01-18 10:00:10,1,STABLE_LEVEL,24.5,73.5,0,0,0,0,6000,0,0,0,,0,0,0
```

### 4. POURING

Pour detected, slope calculation and real-time volume tracking begin.

**Relevant Columns**: Timestamp, Scale, EventType, PrePourWeight, AvgSlope (instantaneous)

**Example**:
```
1,2026-01-18 10:03:15,1,POURING,24.5,0,24.5,0,0,0,0,-0.245,0,0,,0,0,0
```

### 5. WEIGHT_CHANGE_DETECTED

Large weight increase detected in STABLE state (> 0.4kg), awaiting settlement confirmation.

**Relevant Columns**: Timestamp, Scale, EventType, PrevWeight, CurrWeight, Variance

**Example**:
```
1,2026-01-18 10:15:20,1,WEIGHT_CHANGE_DETECTED,0,0,0,0,0,0,0,0,23.2,23.7,,0,0.45,0
```

### 6. POUR_COMPLETED

Pour finished with complete metrics.

**Relevant Columns**: Timestamp, Scale, EventType, PrePourWeight, PostPourWeight, PourWeight, PourVolume, DurationMs, AvgSlope

**Example**:
```
1,2026-01-18 10:03:25,1,POUR_COMPLETED,24.5,23.8,0.7,0.21,10000,0.0070,0,0,,0,0,0
```

### 7. KEG_REMOVED

Weight dropped below minimum threshold.

**Relevant Columns**: Timestamp, Scale, EventType, PrevWeight, CurrWeight

**Example**:
```
1,2026-01-18 10:15:30,1,KEG_REMOVED,0,0,0,0,0,0,0,0,23.2,0.1,,0,0,0
```

### 8. KEG_REPLACED

New keg detected, weight increased above minimum.

**Relevant Columns**: Timestamp, Scale, EventType, PrevWeight, CurrWeight

**Example**:
```
1,2026-01-18 10:15:35,1,KEG_REPLACED,0,0,0,0,0,0,0,0,0.1,19.8,,0,0,0
```

### 9. KEG_ABSENT_TIMEOUT

Periodic reminder that keg has been absent for 30+ seconds (fires in KEG_ABSENT state every 30-second cycle).

**Relevant Columns**: Timestamp, Scale, EventType, DurationMs, CurrWeight

**Example**:
```
1,2026-01-18 10:16:00,1,KEG_ABSENT_TIMEOUT,0,0,0,0,0,0,30000,0,0,0.3,,0,0,0
```

### 10. INVALID_WEIGHT

Sensor reading outside valid range (±50kg for HX711).

**Relevant Columns**: Timestamp, Scale, EventType only

**Example**:
```
1,2026-01-18 10:25:48,2,INVALID_WEIGHT,0,0,0,0,0,0,0,0,0,0,,0,0,0
```

### 11. LOAD_CELL_ERROR

Signal lost or became unreliable (timeout, NAN, stuck value, noise, etc.).

**Relevant Columns**: Timestamp, Scale, EventType, SignalErrorReason, SignalQuality, Variance, ConsecutiveErrors

**Example**:
```
1,2026-01-18 10:22:15,2,LOAD_CELL_ERROR,0,0,0,0,0,0,0,0,0,0,TIMEOUT,25,0.85,3
```

### 12. LOAD_CELL_RECOVERED

Signal restored to normal operation (signal quality > 80% and clean reads).

**Relevant Columns**: Timestamp, Scale, EventType, SignalQuality, Variance

**Example**:
```
1,2026-01-18 10:22:20,2,LOAD_CELL_RECOVERED,0,0,0,0,0,0,0,0,0,0,,95,0.02,0
```

### 13. SENSOR_RECOVERED

Weight returned to valid range after being in INVALID_WEIGHT state.

**Relevant Columns**: Timestamp, Scale, EventType, CurrWeight

**Example**:
```
1,2026-01-18 10:26:05,2,SENSOR_RECOVERED,0,0,0,0,0,0,0,0,0,21.5,,0,0,0
```

### 14. CALIBRATION_NEEDED

Scale calibration missing (scale factor == 0.0f).

**Relevant Columns**: Timestamp, Scale, EventType only

**Example**:
```
1,2026-01-18 10:26:00,3,CALIBRATION_NEEDED,0,0,0,0,0,0,0,0,0,0,,0,0,0
```

### 15. CALIBRATION_COMPLETE

Calibration successfully completed. System ready for weight monitoring.

**Relevant Columns**: Timestamp, Scale, EventType only

**Example**:
```
1,2026-01-18 10:26:10,3,CALIBRATION_COMPLETE,0,0,0,0,0,0,0,0,0,0,,0,0,0
```

### 16. DISABLED

Scale unit disabled at startup because ADC hardware (HX711) was not found. This is a permanent state for the session.

**Relevant Columns**: Timestamp, Scale, EventType only

**Example**:
```
1,2026-01-18 10:00:01,4,DISABLED,0,0,0,0,0,0,0,0,0,0,,0,0,0
```

## Signal Error Reasons

Used in LOAD_CELL_ERROR events:

| Reason | Description |
|--------|-------------|
| TIMEOUT | HX711 read timeout, no data from sensor |
| NAN_VALUE | Sensor returned NaN (not a number) |
| OUT_OF_RANGE | Reading exceeds ±50kg sensor limits |
| STUCK_VALUE | Same value repeated, zero variance |
| PHANTOM_SPIKE | Unexplained large jump between reads (> 0.5kg), EMI artifact |
| EXCESSIVE_NOISE | Jitter beyond acceptable threshold |
| CALIBRATION_INVALID | Scale factor or offset not properly set |

## Timestamps

- **Format**: ISO 8601 datetime (`YYYY-MM-DD HH:MM:SS`)
- **Source**: RTC (Real-Time Clock via NTP WiFi sync)
- **Baseline**: System startup time (captured in first event)
- **Accuracy**: ±1 second (resolution is per-second in CSV, millisecond tracking in device)

## Version Strategy

The schema version number enables backward-compatible changes:

- **v1**: Current format (18 columns)
- **v2+**: Future enhancements can add columns without breaking parsers reading v1 data

## Example Analysis

### Session Example

```
1,2026-01-18 10:00:00,1,SYSTEM_STARTUP,0,0,0,0,0,0,0,0,0,0,,0,0,0
1,2026-01-18 10:00:06,1,STABLE_LEVEL,24.5,73.5,0,0,0,0,6000,0,0,0,,0,0,0
1,2026-01-18 10:03:15,1,POURING,24.5,0,24.5,0,0,0,0,0.0245,0,0,,0,0,0
1,2026-01-18 10:03:25,1,POUR_COMPLETED,24.5,23.8,0.7,0.21,10000,0.0070,0,0,,0,0,0
1,2026-01-18 10:03:32,1,STABLE_LEVEL,23.8,71.4,0,0,0,0,6000,0,0,0,,0,0,0
```

Reading this:
- System started at 10:00:00
- Scale 1 baseline locked at 24.5 kg (73.5 L) after 6 seconds of sustained stability
- Pour detected at 10:03:15
- Pour complete at 10:03:25: 0.7 kg poured (0.21 L) over 10 seconds at 0.007 kg/sec
- New baseline at 23.8 kg (71.4 L) after 6 seconds of sustained stabilization

## Configuration

Edit constants in [event_logger.hpp](src/event_logger.hpp):

```cpp
#define LOG_VERSION 1              // Current schema version
#define MAX_LOG_FILE_SIZE (16 * 1024)  // 16 KB per file
#define MAX_LOG_FILES 4            // 4 files total (data.csv + 3 rotations)
#define LOG_BASE_FILENAME "/data"  // CSV file base name
#define LOG_FILE_EXTENSION ".csv"  // File extension
```

## Usage

The logger is initialized and called automatically:
- **Initialization**: `setup()` → `myEventLogger.init()`
- **Logging**: Main loop event processing → `myEventLogger.logEvent(event)`
- **File rotation**: Automatic when size threshold exceeded

No manual configuration needed unless adjusting constants.
