/*
 * KegMon
 * Copyright (c) 2022-2026 Magnus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this software may be used under the terms of a
 * commercial license. See LICENSE_COMMERCIAL for details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include <Arduino.h>

#include <cstdio>
#include <cstring>
#include <event_logger.hpp>
#include <sdcard_mmc.hpp>
#include <sdcard_sd.hpp>

EventLogger myEventLogger;

// External SD card storage (declared in main.cpp)
#if defined(ENABLE_MMC)
extern SdCardMMC mySdStorage;
#elif defined(ENABLE_SD)
extern SdCardSD mySdStorage;
#endif

void EventLogger::init() {
  _systemStartupMs = 0;
  _startupLogged = false;
}

bool EventLogger::hasCard() const {
#if defined(ENABLE_MMC) || defined(ENABLE_SD)
  return mySdStorage.hasCard();
#else
  return false;
#endif
}

const char* EventLogger::getCurrentLogPath() const {
  return LOG_BASE_FILENAME LOG_FILE_EXTENSION;
}

void EventLogger::getLogPathByIndex(int index, char* buffer,
                                    size_t bufferSize) const {
  if (index == 0) {
    snprintf(buffer, bufferSize, "%s%s", LOG_BASE_FILENAME, LOG_FILE_EXTENSION);
  } else {
    snprintf(buffer, bufferSize, "%s%d%s", LOG_BASE_FILENAME, index,
             LOG_FILE_EXTENSION);
  }
}

void EventLogger::formatTimestamp(const ChangeDetectionEvent& event,
                                  char* buffer, size_t bufferSize) {
  // If this is SYSTEM_STARTUP, use current time
  if (event.type == ChangeDetectionEventType::SYSTEM_STARTUP) {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", timeinfo);
  } else {
    // Calculate elapsed time since SYSTEM_STARTUP
    uint64_t elapsedMs = event.timestampMs - _systemStartupMs;

    // Get current RTC time and system uptime
    time_t now = time(nullptr);
    uint32_t currentUptimeMs = millis();

    // Calculate how much uptime has passed since SYSTEM_STARTUP
    uint64_t uptimeSinceStartupMs = currentUptimeMs - _systemStartupMs;

    // Event time = now - (uptime since SYSTEM_STARTUP) + (elapsed since event)
    time_t eventTime = now - (uptimeSinceStartupMs / 1000) + (elapsedMs / 1000);
    struct tm* timeinfo = localtime(&eventTime);
    strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", timeinfo);
  }
}

const char* EventLogger::getEventTypeString(
    ChangeDetectionEventType type) const {
  // Use the global display name and convert it to ENUM_FORMAT
  // (spaces -> underscores, uppercase)
  const char* displayName = getEventTypeDisplayName(type);
  static char buffer[32];
  
  int i = 0;
  while (displayName[i] != '\0' && i < (int)sizeof(buffer) - 1) {
    if (displayName[i] == ' ') {
      buffer[i] = '_';
    } else if (displayName[i] >= 'a' && displayName[i] <= 'z') {
      buffer[i] = displayName[i] - 32;  // Convert to uppercase
    } else if (displayName[i] >= 'A' && displayName[i] <= 'Z') {
      buffer[i] = displayName[i];
    } else {
      buffer[i] = displayName[i];
    }
    i++;
  }
  buffer[i] = '\0';
  
  return buffer;
}

void EventLogger::formatEventRow(const ChangeDetectionEvent& event,
                                 char* buffer, size_t bufferSize) {
  char timestamp[32];
  formatTimestamp(event, timestamp, sizeof(timestamp));

  const char* eventType = getEventTypeString(event.type);
  int scale = static_cast<int>(event.unitIndex) + 1;  // U1-U4

  // Start with version and basic fields
  snprintf(buffer, bufferSize, "%d,%s,%d,%s", LOG_VERSION, timestamp, scale,
           eventType);

  // Append event-specific fields (all rows have same column count for easy
  // parsing) Columns:
  // Version,Timestamp,Scale,EventType,StableWeight,StableVolume,
  //          PrePourWeight,PostPourWeight,PourWeight,PourVolume,DurationMs,AvgSlope,
  //          PrevWeight,CurrWeight,SignalErrorReason,SignalQuality,Variance,ConsecutiveErrors

  // Gather all possible data fields
  const char* emptyField = "";
  float stableWeight = event.stable.stableWeightKg;
  WeightVolumeConverter volumeConverter(event.unitIndex);
  float stableVolume = volumeConverter.weightToVolume(stableWeight);
  float prePourWeight = event.pour.prePourWeightKg;
  float postPourWeight = event.pour.postPourWeightKg;
  float pourWeight = event.pour.pourWeightKg;
  float pourVolume = event.pour.pourVolumeL;
  uint64_t durationMs = 0;
  float avgSlope = event.pour.averageSlopeKgSec;
  float prevWeight = event.weight.previousWeightKg;
  float currWeight = event.weight.currentWeightKg;
  const char* errorReason = emptyField;
  uint8_t signalQuality = event.loadCell.signalQualityPercent;
  float variance = event.loadCell.varianceKg;
  uint16_t consecutiveErrors = event.loadCell.consecutiveErrors;

  // Determine which duration to use
  if (event.type == ChangeDetectionEventType::STABLE_LEVEL) {
    durationMs = event.stable.durationMs;
  } else if (event.type == ChangeDetectionEventType::POURING ||
             event.type == ChangeDetectionEventType::POUR_COMPLETED) {
    durationMs = event.pour.durationMs;
  }

  // Determine signal error reason string
  if (event.type == ChangeDetectionEventType::LOAD_CELL_ERROR ||
      event.type == ChangeDetectionEventType::LOAD_CELL_RECOVERED) {
    switch (event.loadCell.reason) {
      case SignalErrorReason::TIMEOUT:
        errorReason = "TIMEOUT";
        break;
      case SignalErrorReason::NAN_VALUE:
        errorReason = "NAN_VALUE";
        break;
      case SignalErrorReason::OUT_OF_RANGE:
        errorReason = "OUT_OF_RANGE";
        break;
      case SignalErrorReason::STUCK_VALUE:
        errorReason = "STUCK_VALUE";
        break;
      case SignalErrorReason::EXCESSIVE_NOISE:
        errorReason = "EXCESSIVE_NOISE";
        break;
      case SignalErrorReason::CALIBRATION_INVALID:
        errorReason = "CALIBRATION_INVALID";
        break;
      default:
        errorReason = "UNKNOWN";
        break;
    }
  }

  // Append all data columns (using empty string or 0 for missing data)
  char eventDataRow[512];
  snprintf(eventDataRow, sizeof(eventDataRow),
           ",%g,%g,%g,%g,%g,%g,%llu,%g,%g,%g,%s,%d,%g,%d",
           stableWeight == 0.0f ? 0.0f : stableWeight,
           stableVolume == 0.0f ? 0.0f : stableVolume,
           prePourWeight == 0.0f ? 0.0f : prePourWeight,
           postPourWeight == 0.0f ? 0.0f : postPourWeight,
           pourWeight == 0.0f ? 0.0f : pourWeight,
           pourVolume == 0.0f ? 0.0f : pourVolume, durationMs,
           avgSlope == 0.0f ? 0.0f : avgSlope,
           prevWeight == 0.0f ? 0.0f : prevWeight,
           currWeight == 0.0f ? 0.0f : currWeight, errorReason, signalQuality,
           variance == 0.0f ? 0.0f : variance, consecutiveErrors);

  strlcat(buffer, eventDataRow, bufferSize);
}

void EventLogger::writeToFile(const char* row) {
  if (!hasCard()) {
    return;
  }

#if defined(ENABLE_MMC) || defined(ENABLE_SD)
  const char* filePath = getCurrentLogPath();
  File file = mySdStorage.open(filePath, FILE_APPEND);

  if (!file) {
    // Error opening file, silently fail (non-blocking)
    return;
  }

  file.println(row);
  file.close();
#endif
}

bool EventLogger::shouldRotate() const {
  if (!hasCard()) {
    return false;
  }

#if defined(ENABLE_MMC) || defined(ENABLE_SD)
  const char* filePath = getCurrentLogPath();
  File file = mySdStorage.open(filePath, FILE_READ);

  if (!file) {
    // File doesn't exist yet, no rotation needed
    return false;
  }

  size_t fileSize = file.size();
  file.close();

  return fileSize > MAX_LOG_FILE_SIZE;
#endif

  return false;
}

void EventLogger::rotateFiles() {
  if (!hasCard()) {
    return;
  }

#if defined(ENABLE_MMC) || defined(ENABLE_SD)
  // Rotate files: data3.csv deleted, data2->data3, data1->data2, data->data1
  for (int i = MAX_LOG_FILES - 1; i >= 1; --i) {
    char oldPath[64];
    char newPath[64];

    getLogPathByIndex(i - 1, oldPath, sizeof(oldPath));
    getLogPathByIndex(i, newPath, sizeof(newPath));

    // Remove old destination if exists
    mySdStorage.remove(newPath);

    // Rename old file to new location
    if (mySdStorage.exists(oldPath)) {
      mySdStorage.rename(oldPath, newPath);
    }
  }
#endif
}

void EventLogger::logEvent(const ChangeDetectionEvent& event) {
  // Capture system startup timestamp as baseline for subsequent events
  if (event.type == ChangeDetectionEventType::SYSTEM_STARTUP) {
    _systemStartupMs = event.timestampMs;
    _startupLogged = true;
  }

  if (!hasCard()) {
    return;
  }

  // Check if rotation is needed before writing
  if (shouldRotate()) {
    rotateFiles();
  }

  // Format and write the event row
  char row[768];
  formatEventRow(event, row, sizeof(row));
  writeToFile(row);
}
