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
#ifndef SRC_CHANGEDETECTION_STATISTICS_HPP_
#define SRC_CHANGEDETECTION_STATISTICS_HPP_

#include <Arduino.h>

#include <cstdint>
#include <main.hpp>

enum class ChangeDetectionState;

// Per-scale change detection statistics
struct ChangeDetectionStatistics {
  // Pour statistics
  uint32_t totalPours = 0;
  float totalPourVolume = 0.0f;   // liters
  float avgPourVolume = 0.0f;     // liters
  float maxPourVolume = 0.0f;     // liters
  float minPourVolume = 1000.0f;  // liters
  uint32_t totalPourDurationMs = 0;
  float avgPourDurationMs = 0.0f;

  // Keg statistics
  uint32_t kegReplacements = 0;
  uint64_t currentKegStartTimeMs = 0;
  uint64_t lastKegReplacementMs = 0;
  float lastKegWeight = 0.0f;

  // State tracking
  uint32_t stateTransitions = 0;
  uint32_t timeInStateMs[8] = {0};    // One entry per ChangeDetectionState
  uint32_t stateEnterCount[8] = {0};  // Times entered each state

  // Stability metrics
  uint32_t stabilizationCount = 0;  // Times reached Stable state
  float avgStabilizationTimeMs = 0.0f;

  // Last event timestamps
  uint64_t lastPourTimeMs = 0;
  uint64_t lastStableTimeMs = 0;
  uint64_t lastKegRemovalTimeMs = 0;

  float avgPourVolumeL() const {
    if (totalPours == 0) return 0.0f;
    return totalPourVolume / totalPours;
  }

  float avgPourDurationSec() const {
    if (totalPours == 0) return 0.0f;
    return avgPourDurationMs / 1000.0f;
  }

  uint64_t getCurrentKegAgeMs() const {
    if (currentKegStartTimeMs == 0) return 0;
    return millis() - currentKegStartTimeMs;
  }

  uint32_t getCurrentKegAgeHours() const {
    return getCurrentKegAgeMs() / (1000UL * 60UL * 60UL);
  }

  void recordPour(float volumeL, uint64_t durationMs) {
    totalPours++;
    totalPourVolume += volumeL;
    totalPourDurationMs += durationMs;

    if (volumeL > maxPourVolume) maxPourVolume = volumeL;
    if (volumeL < minPourVolume) minPourVolume = volumeL;

    avgPourVolume = totalPourVolume / totalPours;
    avgPourDurationMs = totalPourDurationMs / totalPours;

    lastPourTimeMs = millis();
  }

  void recordKegReplacement(float newKegWeight) {
    kegReplacements++;
    lastKegReplacementMs = millis();
    currentKegStartTimeMs = lastKegReplacementMs;
    lastKegWeight = newKegWeight;
  }

  void recordStateChange(int previousStateIndex, int newStateIndex,
                         uint64_t durationInPreviousStateMs) {
    stateTransitions++;

    if (previousStateIndex >= 0 && previousStateIndex < 8) {
      timeInStateMs[previousStateIndex] += durationInPreviousStateMs;
      stateEnterCount[previousStateIndex]++;
    }

    if (newStateIndex >= 0 && newStateIndex < 8) {
      // Track stabilization
      if (newStateIndex == 3) {  // Stable state = index 3
        stabilizationCount++;
        if (stabilizationCount == 1) {
          avgStabilizationTimeMs = durationInPreviousStateMs;
        } else {
          // Running average
          avgStabilizationTimeMs =
              (avgStabilizationTimeMs * (stabilizationCount - 1) +
               durationInPreviousStateMs) /
              stabilizationCount;
        }
        lastStableTimeMs = millis();
      }
    }
  }

  float getAvgTimeInStateMs(int stateIndex) const {
    if (stateIndex < 0 || stateIndex >= 8) return 0.0f;
    if (stateEnterCount[stateIndex] == 0) return 0.0f;
    return static_cast<float>(timeInStateMs[stateIndex]) /
           stateEnterCount[stateIndex];
  }

  float getTotalTimeInStateSec(int stateIndex) const {
    if (stateIndex < 0 || stateIndex >= 8) return 0.0f;
    return static_cast<float>(timeInStateMs[stateIndex]) / 1000.0f;
  }

  void reset() {
    totalPours = 0;
    totalPourVolume = 0.0f;
    avgPourVolume = 0.0f;
    maxPourVolume = 0.0f;
    minPourVolume = 1000.0f;
    totalPourDurationMs = 0;
    avgPourDurationMs = 0.0f;

    kegReplacements = 0;
    currentKegStartTimeMs = 0;
    lastKegReplacementMs = 0;
    lastKegWeight = 0.0f;

    stateTransitions = 0;
    for (int i = 0; i < 8; i++) {
      timeInStateMs[i] = 0;
      stateEnterCount[i] = 0;
    }

    stabilizationCount = 0;
    avgStabilizationTimeMs = 0.0f;

    lastPourTimeMs = 0;
    lastStableTimeMs = 0;
    lastKegRemovalTimeMs = 0;
  }
};

// Global statistics manager for all 4 scales
class ChangeDetectionStatisticsManager {
 private:
  ChangeDetectionStatistics _scales[MAX_SCALES];

 public:
  ChangeDetectionStatisticsManager() = default;

  void recordPour(int scaleIndex, float volumeL, uint64_t durationMs) {
    if (scaleIndex < MAX_SCALES && scaleIndex >= 0) {
      _scales[scaleIndex].recordPour(volumeL, durationMs);
    }
  }

  void recordKegReplacement(int scaleIndex, float newKegWeight) {
    if (scaleIndex < MAX_SCALES && scaleIndex >= 0) {
      _scales[scaleIndex].recordKegReplacement(newKegWeight);
    }
  }

  void recordStateChange(int scaleIndex, int previousStateIndex,
                         int newStateIndex,
                         uint64_t durationInPreviousStateMs) {
    if (scaleIndex < MAX_SCALES && scaleIndex >= 0) {
      _scales[scaleIndex].recordStateChange(previousStateIndex, newStateIndex,
                                            durationInPreviousStateMs);
    }
  }

  const ChangeDetectionStatistics& getStatistics(int scaleIndex) const {
    return _scales[scaleIndex];
  }

  void resetStatistics(int scaleIndex) {
    if (scaleIndex < MAX_SCALES) {
      _scales[scaleIndex].reset();
    }
  }

  void resetAllStatistics() {
    for (int i = 0; i < 4; i++) {
      _scales[i].reset();
    }
  }
};

#endif  // SRC_CHANGEDETECTION_STATISTICS_HPP_

// EOF
