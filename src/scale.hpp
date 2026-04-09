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
#ifndef SRC_SCALE_HPP_
#define SRC_SCALE_HPP_

#include <HX711.h>

#include <atomic>
#include <filters/filter_base.hpp>
#include <filters/filter_kalman.hpp>
#include <kegconfig.hpp>
#include <main.hpp>
#include <memory>
#include <perf.hpp>
#include <scale_filter_pipeline.hpp>
#include <scale_statistics.hpp>

// ScaleReadingResult is defined in scale_filter_pipeline.hpp
// Import it here for backward compatibility
using ScaleReadingResult = ScaleReadingResult;

class Scale {
 private:
  class Schedule {
   public:
    std::atomic<bool> tare = false;
    std::atomic<bool> findFactor = false;
    float factorWeight = 0.0;  // Protected by factorWeight atomic read/write
  };

  std::unique_ptr<HX711> _hxScale[MAX_SCALES] = {nullptr, nullptr, nullptr,
                                                 nullptr};

  Schedule _sched[MAX_SCALES];
  float _lastRaw[MAX_SCALES] = {0.0, 0.0, 0.0, 0.0};
  int _dataPins[MAX_SCALES] = {-1, -1, -1,
                               -1};  // Store data pin for each scale
  std::atomic<bool> _isCalibrating[MAX_SCALES] = {
      false, false, false, false};  // Suppress events during calibration
  uint8_t _detectionRetries[MAX_SCALES] = {
      0, 0, 0, 0};  // Retry counter for sampling rate detection (max 3 retries)

  // Filter pipelines - one per scale sensor
  std::unique_ptr<ScaleFilterPipeline> _filterPipeline[MAX_SCALES];

  // Latest reading results for each scale
  ScaleReadingResult _lastResult[MAX_SCALES];

  // Detected sampling rate (10 or 80 SPS) measured from Data Ready signal
  // timing
  uint8_t _detectedSamplingRate[MAX_SCALES] = {0, 0, 0, 0};

  // Statistics manager
  ScaleStatisticsManager _stats;

  Scale(const Scale&) = delete;
  void operator=(const Scale&) = delete;

  void tare(UnitIndex idx);
  void findFactor(UnitIndex idx, float weight);
  void setupScale(UnitIndex idx, bool force, int pinData, int pinClock);
  void loopScale(UnitIndex idx);
  void setScaleFactor(UnitIndex idx);
  float readRaw(UnitIndex idx);
  uint8_t detectSamplingRate(UnitIndex idx);

 public:
  Scale() = default;
  ~Scale() = default;

  void setup(bool force = false);
  void loop();
  bool isReady(UnitIndex idx) const;

  void scheduleTare(UnitIndex idx) { _sched[idx].tare = true; }
  void scheduleFindFactor(UnitIndex idx, float weight) {
    _sched[idx].findFactor = true;
    _sched[idx].factorWeight = weight;
  }
  bool isScheduleRunning() const {
    for (int i = 0; i < MAX_SCALES; i++) {
      if (_sched[i].findFactor || _sched[i].tare) {
        return true;
      }
    }
    return false;
  }
  int32_t readLastRaw(UnitIndex idx) { return _lastRaw[idx]; }
  bool isConnected(UnitIndex idx) const { return _hxScale[idx] ? true : false; }
  int getConnectedScaleCount() const {
    int count = 0;
    for (int i = 0; i < MAX_SCALES; i++) {
      if (isConnected(static_cast<UnitIndex>(i))) {
        count++;
      }
    }
    return count;
  }
  ScaleReadingResult read(UnitIndex idx);
  inline ScaleReadingResult getLastResult(UnitIndex idx) const {
    return _lastResult[static_cast<int>(idx)];
  }
  bool isCalibrating(UnitIndex idx) const {
    return _isCalibrating[static_cast<int>(idx)];
  }
  const ScaleStatistics& getStatistics(UnitIndex idx) const {
    return _stats.getStatistics(static_cast<int>(idx));
  }
  void resetStatistics(UnitIndex idx) {
    _stats.resetStatistics(static_cast<int>(idx));
  }
  void resetAllStatistics() { _stats.resetAllStatistics(); }
  void resetFilters(UnitIndex idx);
  uint8_t getSamplingRate(UnitIndex idx) const {
    return _detectedSamplingRate[static_cast<int>(idx)];
  }
};

extern Scale myScale;

#endif  // SRC_SCALE_HPP_

// EOF
