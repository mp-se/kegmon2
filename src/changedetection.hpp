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
#ifndef SRC_CHANGEDETECTION_HPP_
#define SRC_CHANGEDETECTION_HPP_

#include <changedetection_statistics.hpp>
#include <cmath>
#include <filters/filter_base.hpp>
#include <kegconfig.hpp>
#include <scale.hpp>
#include <weightvolume.hpp>

enum class ChangeDetectionState {
  Idle,              // Initializing, no stable level established
  Settling,          // Settling to a stable level (initial OR re-stabilization)
  Stable,            // Stable level confirmed
  Pouring,           // Pour detected, waiting for stabilization
  KegAbsent,         // Weight dropped below threshold (keg removed)
  ReplacingKeg,      // Keg reappeared, settling to new level
  InvalidWeight,     // Sensor reading out of valid range
  LoadCellError,     // ADC/Load cell signal lost or unreliable (sensor dead)
  CalibrationNeeded, // Scale needs calibration (scaleFactor is invalid)
  Disabled           // Scale hardware (ADC) not found at startup
};

// Event types fired by change detection state machine
enum class ChangeDetectionEventType {
  SYSTEM_STARTUP,           // System initialized/session started
  SETTLING_STARTED,         // Entered Settling state (initial or re-stabilization)
  STABLE_LEVEL,             // Entered Stable state (baseline locked)
  POURING,                  // Entered Pouring state
  WEIGHT_CHANGE_DETECTED,   // Large weight increase (>0.4kg) in STABLE state
  POUR_COMPLETED,           // Exiting Pouring state (pour finished)
  KEG_REMOVED,              // Entered KegAbsent state
  KEG_REPLACED,             // Entered ReplacingKeg state
  KEG_ABSENT_TIMEOUT,       // Periodic reminder in KEG_ABSENT (every 30 sec)
  INVALID_WEIGHT,           // Sensor reading out of valid range
  LOAD_CELL_ERROR,          // Signal lost/became unreliable
  LOAD_CELL_RECOVERED,      // Signal restored to normal
  SENSOR_RECOVERED,         // Weight returned to valid range (exit INVALID_WEIGHT)
  CALIBRATION_NEEDED,       // Scale needs calibration (scaleFactor == 0.0f)
  CALIBRATION_COMPLETE,     // Calibration successfully completed
  HARDWARE_DISABLED         // Scale unit disabled due to missing hardware
};

// Reasons for load cell signal errors
enum class SignalErrorReason : uint8_t {
  TIMEOUT = 0,             // Read timeout from HX711
  NAN_VALUE = 1,           // Got NAN reading
  OUT_OF_RANGE = 2,        // Reading exceeds ±50kg sensor limits
  STUCK_VALUE = 3,         // Same value repeated (variance = 0)
  PHANTOM_SPIKE = 4,       // Unexplained large jump between reads (>0.5kg)
  EXCESSIVE_NOISE = 5,     // Jitter too high
  CALIBRATION_INVALID = 6  // Factor or offset not set
};

// Self-contained event with all data needed for processing
struct ChangeDetectionEvent {
  ChangeDetectionEventType type;
  UnitIndex unitIndex = UnitIndex(0);  // U1
  uint64_t timestampMs = 0;

  // Data for STABLE_LEVEL
  struct {
    float stableWeightKg = 0.0f;
    uint64_t durationMs = 0;  // Time to stabilize
  } stable;

  // Data for POURING, POUR_COMPLETED
  struct {
    float prePourWeightKg = 0.0f;
    float postPourWeightKg = 0.0f;
    float pourWeightKg = 0.0f;
    float pourVolumeL = 0.0f;
    uint64_t durationMs = 0;
    float averageSlopeKgSec = 0.0f;
  } pour;

  // Data for KEG_REMOVED, KEG_REPLACED
  struct {
    float previousWeightKg = 0.0f;
    float currentWeightKg = 0.0f;
  } weight;

  // Data for INVALID_WEIGHT
  struct {
    // No data - indicates sensor fault or reading out of valid range
  } invalid;

  // Data for LOAD_CELL_ERROR, LOAD_CELL_RECOVERED
  struct {
    SignalErrorReason reason = SignalErrorReason::TIMEOUT;
    uint8_t signalQualityPercent = 0;  // 0-100
    float varianceKg = 0.0f;           // Reading variance
    uint16_t consecutiveErrors = 0;    // Error count
  } loadCell;
};

// Thread-safe circular queue for change detection events
class ChangeDetectionEventQueue {
 private:
  static constexpr size_t MAX_EVENTS = 64;
  ChangeDetectionEvent _events[MAX_EVENTS];
  volatile size_t _head = 0;
  volatile size_t _tail = 0;
  mutable portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;  // ESP32 spinlock

  size_t nextIndex(size_t current) const { return (current + 1) % MAX_EVENTS; }

 public:
  ChangeDetectionEventQueue() = default;

  // Push event to queue (called from detection thread)
  bool push(const ChangeDetectionEvent& event);

  // Pop event from queue (called from main loop)
  bool pop(ChangeDetectionEvent& event);

  bool isEmpty() const;

  size_t count() const;
};

// Per-scale state tracking
struct ChangeDetectionInstance {
  ChangeDetectionState state = ChangeDetectionState::Idle;
  ChangeDetectionState previousState = ChangeDetectionState::Idle;

  // Timing and state tracking
  uint64_t stateEntryTimeMs = 0;
  float stableWeight = 0.0f;
  float previousFilterValue = 0.0f;
  float accumulatedSlope = 0.0f;
  int slopeReadings = 0;
  float pourVolume = 0.0f;
  uint64_t lastSlopeUpdateMs = 0;
  float lastSlopeValue = 0.0f;

  // Dual-filter slope tracking for pour detection
  // Primary slope (Kalman filter - stable baseline)
  float primaryPreviousValue = 0.0f;
  float primaryAccumulatedSlope = 0.0f;
  int primarySlopeReadings = 0;
  
  // Secondary slope (Fast-response filter like MMA - catches quick changes)
  float secondaryPreviousValue = 0.0f;
  float secondaryAccumulatedSlope = 0.0f;
  int secondarySlopeReadings = 0;

  // Pour tracking
  float prePourWeight = 0.0f;   // Weight before pour started
  float previousWeight = 0.0f;  // For weight change detection

  // Settlement tracking (sustained slope stability)
  uint64_t lastSlopeThresholdExceededMs = 0;  // When slope last went outside acceptable range
  // Used to track: slope must stay within threshold for 6 consecutive seconds

  // Unrealistic drop rate tracking (weight removal detection)
  int steepSlopeReadings = 0;  // Consecutive readings with slope < minPourSlope
  // Requires 3+ consecutive steep slope readings to confirm weight removal (not transient spike)

  // Large weight increase tracking (weight addition detection)
  int largeIncreaseReadings = 0;  // Consecutive readings showing weight >= stableWeight + 0.4kg
  // Requires 3+ consecutive readings to confirm weight addition (not sensor spike)

  // Load cell signal quality tracking
  bool signalWasValid = false;         // Was signal valid in previous read?
  uint8_t signalQualityPercent = 100;  // Current signal quality (0-100)
  uint16_t consecutiveErrors = 0;      // Count of bad readings in a row
  SignalErrorReason lastErrorReason = SignalErrorReason::TIMEOUT;
};

// Multi-scale change detection with consensus voting
// Manages pour detection and keg absence detection for all 4 scales
class ChangeDetection {
 private:
  ChangeDetectionInstance _scales[MAX_SCALES];
  ChangeDetectionEventQueue _eventQueue;

  // Configuration (shared across all scales)
  FilterType _stabilityFilter;
  FilterType _pourDetectionFilter;
  uint64_t _levelStabilizationDurationMs;
  uint64_t _pourDurationMs;
  uint64_t _kegAbsenceDurationMs;
  uint64_t _kegReplacementDurationMs;
  float _weightAbsentThreshold;
  float _levelIncreaseThreshold;
  float _levelDecreaseThreshold;
  float _pourSlopeThreshold;
  float _minPourSlope;  // Maximum downward slope (fastest realistic pour)
  float _maxPourSlope;  // Minimum downward slope (slowest realistic pour)

  // Statistics manager
  ChangeDetectionStatisticsManager _stats;

  // Helper methods
  bool isWeightValid(float weight, UnitIndex idx) const;
  bool isWeightAbsent(float weight) const;
  bool isWeightPresent(float weight, UnitIndex idx) const;
  bool isWithinStabilityWindow(float weight, float stableWeight) const;

  void transitionState(UnitIndex idx, ChangeDetectionState newState,
                       uint64_t timestampMs);

  void fireEvent(UnitIndex idx, ChangeDetectionEventType eventType,
                 uint64_t timestampMs);

  float calculateSlope(UnitIndex idx, float currentValue, uint64_t timestampMs);

  void resetSlope(UnitIndex idx);

  // Dual-filter slope detection for pour triggering
  // Calculate slope from primary filter (Kalman - stable) and secondary filter (fast-response)
  float calculateDualFilterSlope(UnitIndex idx, const ScaleReadingResult& result,
                                 uint64_t timestampMs);
  
  // Check if fast-response filter detects pour (slope exceeds threshold)
  bool isPourDetected(UnitIndex idx) const;
  
  // Reset both primary and secondary slope tracking
  void resetDualFilterSlope(UnitIndex idx);

  // Helper to split oversized pours into multiple events based on glass volume
  void fireMultiplePourEvents(UnitIndex idx,
                              const ChangeDetectionEvent& originalEvent,
                              uint64_t timestampMs);

  // Fire POUR_COMPLETED event with automatic splitting if needed
  void firePourCompletedWithSplitting(UnitIndex idx, uint64_t timestampMs);

 public:
  ChangeDetection();

  void loadConfiguration();

  // Fire a startup event to mark the beginning of a monitoring session
  void fireStartupEvent(uint64_t timestampMs);

  // Permanently disable a scale unit (usually because hardware was not found at startup)
  void disableUnit(UnitIndex idx);

  // Fire an invalid weight event when sensor reading is out of valid range
  void fireInvalidWeightEvent(UnitIndex idx, float currentWeight,
                              uint64_t timestampMs);

  // Update the state machine for a single scale
  void update(UnitIndex idx, const ScaleReadingResult& result,
              uint64_t timestampMs);

  ChangeDetectionState getState(UnitIndex idx) const;
  float getStableWeight(UnitIndex idx) const;
  float getPouringVolume(UnitIndex idx) const;
  float getStableVolume(UnitIndex idx) const;
  float getLastPourVolume(UnitIndex idx) const;
  const char* getStateString(UnitIndex idx) const;
  float getConfidence(UnitIndex idx) const;
  float getAverageSlope(UnitIndex idx) const;

  // Signal quality tracking for load cells
  uint8_t getSignalQuality(UnitIndex idx) const;
  void updateSignalQuality(UnitIndex idx, bool isValid,
                           SignalErrorReason reason, float variance,
                           uint64_t timestampMs);

  // Event queue access (called from main loop)
  bool getNextEvent(ChangeDetectionEvent& event);
  bool hasQueuedEvents() const;
  size_t getPendingEventCount() const;

  // Test event injection for integration testing
#if defined(INJECT_TEST_EVENT)
  void injectTestEvents();
#endif // INJECT_TEST_EVENT

  // Statistics access
  const ChangeDetectionStatistics& getStatistics(UnitIndex idx) const {
    return _stats.getStatistics(static_cast<int>(idx));
  }

  void resetStatistics(UnitIndex idx) {
    _stats.resetStatistics(static_cast<int>(idx));
  }

  void resetAllStatistics() { _stats.resetAllStatistics(); }
};

extern ChangeDetection myChangeDetection;

// Helper function to convert ChangeDetectionEventType to human-readable display name (for UI/logging)
const char* getEventTypeDisplayName(ChangeDetectionEventType eventType);

#endif  // SRC_CHANGEDETECTION_HPP_

// EOF
