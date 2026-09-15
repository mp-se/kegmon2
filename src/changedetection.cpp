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
#include <algorithm>
#include <changedetection.hpp>

// Helper function to convert ChangeDetectionEventType to human-readable display name (for UI/logging)
const char* getEventTypeDisplayName(ChangeDetectionEventType eventType) {
  switch (eventType) {
    case ChangeDetectionEventType::SYSTEM_STARTUP:
      return "System startup";
    case ChangeDetectionEventType::SETTLING_STARTED:
      return "Settling started";
    case ChangeDetectionEventType::STABLE_LEVEL:
      return "Stable level";
    case ChangeDetectionEventType::POURING:
      return "Pouring";
    case ChangeDetectionEventType::WEIGHT_CHANGE_DETECTED:
      return "Weight change detected";
    case ChangeDetectionEventType::POUR_COMPLETED:
      return "Pour completed";
    case ChangeDetectionEventType::KEG_REMOVED:
      return "Keg removed";
    case ChangeDetectionEventType::KEG_REPLACED:
      return "Keg replaced";
    case ChangeDetectionEventType::KEG_ABSENT_TIMEOUT:
      return "Keg absent timeout";
    case ChangeDetectionEventType::INVALID_WEIGHT:
      return "Invalid weight";
    case ChangeDetectionEventType::LOAD_CELL_ERROR:
      return "Load cell error";
    case ChangeDetectionEventType::LOAD_CELL_RECOVERED:
      return "Load cell recovered";
    case ChangeDetectionEventType::SENSOR_RECOVERED:
      return "Sensor recovered";
    case ChangeDetectionEventType::CALIBRATION_NEEDED:
      return "Calibration needed";
    case ChangeDetectionEventType::CALIBRATION_COMPLETE:
      return "Calibration complete";
    default:
      return "Unknown";
  }
}

bool ChangeDetectionEventQueue::push(const ChangeDetectionEvent& event) {
  portENTER_CRITICAL(&_lock);
  size_t nextTail = nextIndex(_tail);

  if (nextTail == _head) {
    // Queue full - drop oldest event (overflow)
    _head = nextIndex(_head);
  }

  _events[_tail] = event;
  _tail = nextTail;
  portEXIT_CRITICAL(&_lock);
  return true;
}

bool ChangeDetectionEventQueue::pop(ChangeDetectionEvent& event) {
  portENTER_CRITICAL(&_lock);

  if (_head == _tail) {
    // Queue empty
    portEXIT_CRITICAL(&_lock);
    return false;
  }

  event = _events[_head];
  _head = nextIndex(_head);
  portEXIT_CRITICAL(&_lock);
  return true;
}

bool ChangeDetectionEventQueue::isEmpty() const { return _head == _tail; }

size_t ChangeDetectionEventQueue::count() const {
  portENTER_CRITICAL(reinterpret_cast<portMUX_TYPE*>(&_lock));
  size_t result =
      (_tail >= _head) ? (_tail - _head) : (MAX_EVENTS - _head + _tail);
  portEXIT_CRITICAL(reinterpret_cast<portMUX_TYPE*>(&_lock));
  return result;
}

bool ChangeDetection::isWeightValid(float weight, UnitIndex idx) const {
  float minValidWeight = -0.1f;  // Allow -100g tolerance for Kalman filter noise
  float maxValidWeight = myConfig.getBeerVolumeToWeight(idx) * 1.1f;  // Allow +10% tolerance for sensor overshoot
  return weight >= minValidWeight && weight <= maxValidWeight;
}

bool ChangeDetection::isWeightAbsent(float weight) const {
  return weight < _weightAbsentThreshold;
}

bool ChangeDetection::isWeightPresent(float weight, UnitIndex idx) const {
  return weight >= myConfig.getKegWeight(idx);
}

bool ChangeDetection::isWithinStabilityWindow(float weight,
                                              float stableWeight) const {
  // Asymmetric thresholds: allow small decreases (pours), but reject increases
  // (drift)
  if (weight < stableWeight) {
    // Decrease: allow decreases up to _levelDecreaseThreshold
    return (stableWeight - weight) <= _levelDecreaseThreshold;
  } else {
    // Increase: only allow increases up to _levelIncreaseThreshold
    return (weight - stableWeight) <= _levelIncreaseThreshold;
  }
}

void ChangeDetection::transitionState(UnitIndex idx,
                                      ChangeDetectionState newState,
                                      uint64_t timestampMs) {
  ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  if (scale.state != newState) {
    // Capture old state before changing
    ChangeDetectionState oldState = scale.state;
    
    // Record state change statistics
    uint64_t durationInCurrentState = timestampMs - scale.stateEntryTimeMs;
    _stats.recordStateChange(
        static_cast<int>(idx), static_cast<int>(scale.state),
        static_cast<int>(newState), durationInCurrentState);

    // Get state name strings
    const char* oldStateName = "Unknown";
    const char* newStateName = "Unknown";
    
    switch (oldState) {
      case ChangeDetectionState::Idle: oldStateName = "Idle"; break;
      case ChangeDetectionState::Settling: oldStateName = "Settling"; break;
      case ChangeDetectionState::Stable: oldStateName = "Stable"; break;
      case ChangeDetectionState::Pouring: oldStateName = "Pouring"; break;
      case ChangeDetectionState::KegAbsent: oldStateName = "KegAbsent"; break;
      case ChangeDetectionState::ReplacingKeg: oldStateName = "ReplacingKeg"; break;
      case ChangeDetectionState::InvalidWeight: oldStateName = "InvalidWeight"; break;
      case ChangeDetectionState::CalibrationNeeded: oldStateName = "CalibrationNeeded"; break;
      case ChangeDetectionState::Disabled: oldStateName = "Disabled"; break;
    }
    
    switch (newState) {
      case ChangeDetectionState::Idle: newStateName = "Idle"; break;
      case ChangeDetectionState::Settling: newStateName = "Settling"; break;
      case ChangeDetectionState::Stable: newStateName = "Stable"; break;
      case ChangeDetectionState::Pouring: newStateName = "Pouring"; break;
      case ChangeDetectionState::KegAbsent: newStateName = "KegAbsent"; break;
      case ChangeDetectionState::ReplacingKeg: newStateName = "ReplacingKeg"; break;
      case ChangeDetectionState::InvalidWeight: newStateName = "InvalidWeight"; break;
      case ChangeDetectionState::CalibrationNeeded: newStateName = "CalibrationNeeded"; break;
      case ChangeDetectionState::Disabled: newStateName = "Disabled"; break;
    }

    Log.notice(F("CHGD: Scale state transition: %s -> %s (duration in state: %d ms) [%d]." CR),
               oldStateName, newStateName, durationInCurrentState, static_cast<int>(idx));

    scale.previousState = scale.state;
    scale.state = newState;
    scale.stateEntryTimeMs = timestampMs;
    
    // Initialize sustained stability timer when entering SETTLING or POURING state
    if (newState == ChangeDetectionState::Settling || newState == ChangeDetectionState::Pouring) {
      scale.lastSlopeThresholdExceededMs = timestampMs;
    }
  }
}

void ChangeDetection::fireEvent(UnitIndex idx,
                                ChangeDetectionEventType eventType,
                                uint64_t timestampMs) {
  const ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  ChangeDetectionEvent event;
  event.type = eventType;
  event.unitIndex = idx;
  event.timestampMs = timestampMs;

  WeightVolumeConverter converter(idx);

  switch (eventType) {
    case ChangeDetectionEventType::STABLE_LEVEL: {
      event.stable.stableWeightKg = scale.stableWeight;
      event.stable.durationMs = timestampMs - scale.stateEntryTimeMs;
      float stableVolume = converter.weightToVolume(scale.stableWeight);
      Log.notice(F("CHGD: Scale STABLE_LEVEL - weight: %F kg, volume: %F L, duration: %d ms [%d]." CR),
                 event.stable.stableWeightKg, stableVolume, event.stable.durationMs, static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::POURING: {
      event.pour.prePourWeightKg = scale.stableWeight;
      event.pour.averageSlopeKgSec = getAverageSlope(idx);
      Log.notice(F("CHGD: Scale POURING - pre-pour weight: %F kg, slope: %F kg/sec [%d]." CR),
                 event.pour.prePourWeightKg, event.pour.averageSlopeKgSec, static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::POUR_COMPLETED: {
      event.pour.prePourWeightKg = scale.prePourWeight;
      event.pour.postPourWeightKg = scale.stableWeight;
      event.pour.pourWeightKg = scale.prePourWeight - scale.stableWeight;
      event.pour.pourVolumeL =
          converter.weightToVolume(event.pour.pourWeightKg);
      event.pour.durationMs = timestampMs - scale.stateEntryTimeMs;
      event.pour.averageSlopeKgSec = getAverageSlope(idx);
      Log.notice(F("CHGD: Scale POUR_COMPLETED - pre: %F kg, post: %F kg, volume: %F L, duration: %d ms [%d]." CR),
                 event.pour.prePourWeightKg, event.pour.postPourWeightKg,
                 event.pour.pourVolumeL, event.pour.durationMs, static_cast<int>(idx));

      // Record pour statistics
      _stats.recordPour(static_cast<int>(idx), event.pour.pourVolumeL,
                        event.pour.durationMs);
      break;
    }

    case ChangeDetectionEventType::KEG_REMOVED: {
      event.weight.previousWeightKg = scale.previousWeight;
      event.weight.currentWeightKg = scale.stableWeight;
      Log.notice(F("CHGD: Scale KEG_REMOVED - previous: %F kg, current: %F kg [%d]." CR),
                 event.weight.previousWeightKg, event.weight.currentWeightKg, static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::KEG_REPLACED: {
      event.weight.previousWeightKg = scale.previousWeight;
      event.weight.currentWeightKg = scale.stableWeight;
      Log.notice(F("CHGD: Scale KEG_REPLACED - previous: %F kg, current: %F kg [%d]." CR),
                 event.weight.previousWeightKg, event.weight.currentWeightKg, static_cast<int>(idx));

      // Record keg replacement statistics
      _stats.recordKegReplacement(static_cast<int>(idx),
                                  event.weight.currentWeightKg);
      break;
    }

    case ChangeDetectionEventType::INVALID_WEIGHT: {
      // Note: invalid weight data is populated by fireInvalidWeightEvent()
      Log.warning(F("CHGD: Scale INVALID_WEIGHT event fired [%d]." CR), static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::SYSTEM_STARTUP: {
      Log.notice(F("CHGD: Scale SYSTEM_STARTUP - system initialized [%d]." CR), static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::SETTLING_STARTED: {
      uint64_t durationMs = timestampMs - scale.stateEntryTimeMs;
      Log.notice(F("CHGD: Scale SETTLING_STARTED - waiting for stabilization [%d]." CR), static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::WEIGHT_CHANGE_DETECTED: {
      Log.notice(F("CHGD: Scale WEIGHT_CHANGE_DETECTED - large increase detected, re-settling [%d]." CR), static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::KEG_ABSENT_TIMEOUT: {
      uint64_t timeInState = timestampMs - scale.stateEntryTimeMs;
      Log.notice(F("CHGD: Scale KEG_ABSENT_TIMEOUT - keg absent for %d ms [%d]." CR), static_cast<int>(timeInState), static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::LOAD_CELL_ERROR: {
      // Note: load cell error data is populated by fireLoadCellErrorEvent()
      Log.error(F("CHGD: Scale LOAD_CELL_ERROR - sensor signal compromised [%d]." CR), static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::LOAD_CELL_RECOVERED: {
      Log.notice(F("CHGD: Scale LOAD_CELL_RECOVERED - sensor signal restored [%d]." CR), static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::SENSOR_RECOVERED: {
      Log.notice(F("CHGD: Scale SENSOR_RECOVERED - weight returned to valid range [%d]." CR), static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::CALIBRATION_NEEDED: {
      Log.warning(F("CHGD: Scale CALIBRATION_NEEDED - scale factor invalid, calibration required [%d]." CR), static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::CALIBRATION_COMPLETE: {
      Log.notice(F("CHGD: Scale CALIBRATION_COMPLETE - calibration successful [%d]." CR), static_cast<int>(idx));
      break;
    }

    case ChangeDetectionEventType::HARDWARE_DISABLED: {
      Log.notice(F("CHGD: Scale DISABLED - unit inactive due to missing hardware [%d]." CR), static_cast<int>(idx));
      break;
    }

    default:
      break;
  }

  _eventQueue.push(event);
}

float ChangeDetection::calculateSlope(UnitIndex idx, float currentValue,
                                      uint64_t timestampMs) {
  ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];

  if (scale.slopeReadings == 0) {
    // First reading in this window
    scale.previousFilterValue = currentValue;
    scale.lastSlopeUpdateMs = timestampMs;
    scale.slopeReadings = 1;
    return 0.0f;
  }

  uint64_t timeDeltaMs = timestampMs - scale.lastSlopeUpdateMs;
  if (timeDeltaMs > 0) {
    float weightDelta = currentValue - scale.previousFilterValue;
    float slope = weightDelta / (timeDeltaMs / 1000.0f);  // kg/sec

    scale.accumulatedSlope += slope;
    scale.slopeReadings++;
    scale.previousFilterValue = currentValue;
    scale.lastSlopeUpdateMs = timestampMs;

    return slope;
  }

  return 0.0f;
}

float ChangeDetection::getAverageSlope(UnitIndex idx) const {
  const ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  if (scale.secondarySlopeReadings > 0) {
    return scale.secondaryAccumulatedSlope / scale.secondarySlopeReadings;
  }
  // Fallback to legacy tracking if dual filter hasn't started yet
  if (scale.slopeReadings > 0) {
    return scale.accumulatedSlope / scale.slopeReadings;
  }
  return 0.0f;
}

void ChangeDetection::resetSlope(UnitIndex idx) {
  ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  scale.accumulatedSlope = 0.0f;
  scale.slopeReadings = 0;
}

float ChangeDetection::calculateDualFilterSlope(UnitIndex idx,
                                                const ScaleReadingResult& result,
                                                uint64_t timestampMs) {
  ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];

  // Get values from primary filter (Kalman - stable)
  float primaryValue = result.getFilterValue(_stabilityFilter);
  
  // Get values from secondary filter (fast-response - we use pourDetectionFilter)
  float secondaryValue = result.getFilterValue(_pourDetectionFilter);

  uint64_t timeDeltaMs = timestampMs - scale.lastSlopeUpdateMs;

  if (scale.slopeReadings == 0) {
    // First reading in this window
    scale.primaryPreviousValue = primaryValue;
    scale.secondaryPreviousValue = secondaryValue;
    scale.lastSlopeUpdateMs = timestampMs;
    scale.slopeReadings = 1;
    scale.primarySlopeReadings = 1;
    scale.secondarySlopeReadings = 1;
    return 0.0f;
  }

  if (timeDeltaMs > 0) {
    // Calculate primary slope (Kalman filter)
    float primaryWeightDelta = primaryValue - scale.primaryPreviousValue;
    float primarySlope = primaryWeightDelta / (timeDeltaMs / 1000.0f);  // kg/sec
    
    // Calculate secondary slope (fast-response filter)
    float secondaryWeightDelta = secondaryValue - scale.secondaryPreviousValue;
    float secondarySlope = secondaryWeightDelta / (timeDeltaMs / 1000.0f);  // kg/sec

    // Accumulate both
    scale.primaryAccumulatedSlope += primarySlope;
    scale.primarySlopeReadings++;
    
    scale.secondaryAccumulatedSlope += secondarySlope;
    scale.secondarySlopeReadings++;
    
    // For backward compatibility, keep the old slope tracking (use primary)
    scale.accumulatedSlope += primarySlope;
    scale.slopeReadings++;
    
    scale.primaryPreviousValue = primaryValue;
    scale.secondaryPreviousValue = secondaryValue;
    scale.lastSlopeUpdateMs = timestampMs;

    return primarySlope;
  }

  return 0.0f;
}

bool ChangeDetection::isPourDetected(UnitIndex idx) const {
  const ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];

  // Require at least 3 readings in the fast filter to avoid false detection on reset
  if (scale.secondarySlopeReadings >= 3) {
    float secondaryAvgSlope = scale.secondaryAccumulatedSlope / scale.secondarySlopeReadings;
    
    // Trigger pour if fast filter detects slope exceeding threshold
    // Must also be within realistic pour flow rate bounds
    bool secondaryInRange = (secondaryAvgSlope >= _minPourSlope && secondaryAvgSlope <= _maxPourSlope);
    bool secondaryPourDetected = (secondaryAvgSlope < _pourSlopeThreshold && secondaryInRange);
    
    return secondaryPourDetected;
  }

  return false;
}

void ChangeDetection::resetDualFilterSlope(UnitIndex idx) {
  ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  
  // Reset primary slope
  scale.primaryAccumulatedSlope = 0.0f;
  scale.primarySlopeReadings = 0;
  scale.primaryPreviousValue = 0.0f;
  
  // Reset secondary slope
  scale.secondaryAccumulatedSlope = 0.0f;
  scale.secondarySlopeReadings = 0;
  scale.secondaryPreviousValue = 0.0f;
  
  // Reset unrealistic drop rate counter
  scale.steepSlopeReadings = 0;
  
  // Reset large increase counter
  scale.largeIncreaseReadings = 0;
  
  // Also reset the legacy slope tracking for backward compatibility
  scale.accumulatedSlope = 0.0f;
  scale.slopeReadings = 0;
  scale.previousFilterValue = 0.0f;
}

ChangeDetection::ChangeDetection()
    : _stabilityFilter(FilterType::FILTER_MEDIAN),
      _pourDetectionFilter(FilterType::FILTER_EMA),
      _levelStabilizationDurationMs(0),
      _pourDurationMs(0),
      _kegAbsenceDurationMs(0),
      _kegReplacementDurationMs(0),
      _weightAbsentThreshold(0.0f),
      _levelIncreaseThreshold(0.0f),
      _levelDecreaseThreshold(0.0f),
      _pourSlopeThreshold(0.0f),
      _minPourSlope(0.0f),
      _maxPourSlope(0.0f) {
  loadConfiguration();
}

void ChangeDetection::loadConfiguration() {
  _stabilityFilter =
      static_cast<FilterType>(myConfig.getStabilityDetectionFilterIndex());
  _pourDetectionFilter =
      static_cast<FilterType>(myConfig.getPourDetectionFilterIndex());
  _levelStabilizationDurationMs =
      myConfig.getLevelStabilizationDurationSeconds() * 1000U;
  _pourDurationMs = myConfig.getPourDurationSeconds() * 1000U;
  _kegAbsenceDurationMs = myConfig.getKegAbsenceDurationSeconds() * 1000U;
  _kegReplacementDurationMs =
      myConfig.getKegReplacementDurationSeconds() * 1000U;
  _weightAbsentThreshold = myConfig.getWeightAbsentThreshold();
  _levelIncreaseThreshold = myConfig.getLevelIncreaseThreshold();
  _levelDecreaseThreshold = myConfig.getLevelDecreaseThreshold();
  _pourSlopeThreshold = myConfig.getPourSlopeThreshold();
  _minPourSlope = myConfig.getMinPourSlope();
  _maxPourSlope = myConfig.getMaxPourSlope();
}

void ChangeDetection::fireStartupEvent(uint64_t timestampMs) {
  ChangeDetectionEvent event;
  event.type = ChangeDetectionEventType::SYSTEM_STARTUP;
  event.unitIndex = UnitIndex(0);
  event.timestampMs = timestampMs;
  _eventQueue.push(event);
}

void ChangeDetection::disableUnit(UnitIndex idx) {
  uint64_t currentTimeMs = millis();
  transitionState(idx, ChangeDetectionState::Disabled, currentTimeMs);
  fireEvent(idx, ChangeDetectionEventType::HARDWARE_DISABLED, currentTimeMs);
}

void ChangeDetection::fireInvalidWeightEvent(UnitIndex idx, float currentWeight,
                                             uint64_t timestampMs) {
  ChangeDetectionEvent event;
  event.type = ChangeDetectionEventType::INVALID_WEIGHT;
  event.unitIndex = idx;
  event.timestampMs = timestampMs;

  _eventQueue.push(event);
}

// Splits oversized pours into multiple events based on configured glass volume
void ChangeDetection::fireMultiplePourEvents(
    UnitIndex idx, const ChangeDetectionEvent& originalEvent,
    uint64_t timestampMs) {
  const float glassVolumeL = myConfig.getGlassVolume(idx);
  const float pourVolumeL = originalEvent.pour.pourVolumeL;

  // If pour is within glass size, just fire the event as-is
  if (pourVolumeL <= glassVolumeL) {
    _eventQueue.push(originalEvent);
    _stats.recordPour(static_cast<int>(idx), originalEvent.pour.pourVolumeL,
                      originalEvent.pour.durationMs);
    return;
  }

  // Calculate number of pours needed to account for total volume
  const int numPours = static_cast<int>(ceil(pourVolumeL / glassVolumeL));
  const float volumePerPour = pourVolumeL / numPours;
  const float weightPerPour = originalEvent.pour.pourWeightKg / numPours;
  const uint64_t timePerPour = originalEvent.pour.durationMs / numPours;

  // Fire individual pour events for each "glass" worth of volume
  for (int i = 0; i < numPours; i++) {
    ChangeDetectionEvent event = originalEvent;
    event.pour.pourVolumeL = volumePerPour;
    event.pour.pourWeightKg = weightPerPour;
    event.pour.postPourWeightKg =
        originalEvent.pour.prePourWeightKg - (weightPerPour * (i + 1));
    event.pour.durationMs = timePerPour;
    // Timestamp is offset for each pour
    event.timestampMs = timestampMs + (i * timePerPour);

    _eventQueue.push(event);
    // Record statistics for each individual pour
    _stats.recordPour(static_cast<int>(idx), volumePerPour, timePerPour);
  }
}

// Constructs POUR_COMPLETED event and handles multi-pour splitting
void ChangeDetection::firePourCompletedWithSplitting(UnitIndex idx,
                                                     uint64_t timestampMs) {
  const ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  ChangeDetectionEvent event;
  event.type = ChangeDetectionEventType::POUR_COMPLETED;
  event.unitIndex = idx;
  event.timestampMs = timestampMs;

  WeightVolumeConverter converter(idx);

  event.pour.prePourWeightKg = scale.prePourWeight;
  event.pour.postPourWeightKg = scale.stableWeight;
  event.pour.pourWeightKg = scale.prePourWeight - scale.stableWeight;
  event.pour.pourVolumeL = converter.weightToVolume(event.pour.pourWeightKg);
  event.pour.durationMs = timestampMs - scale.stateEntryTimeMs;
  event.pour.averageSlopeKgSec = getAverageSlope(idx);

  // Use the multi-pour splitting logic
  fireMultiplePourEvents(idx, event, timestampMs);
}

void ChangeDetection::update(UnitIndex idx, const ScaleReadingResult& result,
                             uint64_t timestampMs) {
  ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];

  // If unit is permanently disabled (missing hardware), skip all processing
  if (scale.state == ChangeDetectionState::Disabled) {
    return;
  }

  float currentWeight = result.getFilterValue(_stabilityFilter);
  float pourDetectionWeight = result.getFilterValue(_pourDetectionFilter);

  // CALIBRATION CHECK: Use scaleFactor == 0.0f (FACTOR_UNCALIBRATED) to
  // identify uncalibrated scales
  // PRIORITY: HIGHEST - Cannot process readings without calibration
  if (myConfig.getScaleFactor(idx) == 0.0f) {
    // Scale has not been calibrated yet (scaleFactor is sentinel value 0.0f)
    // Transition to CalibrationNeeded and skip state machine processing
    if (scale.state != ChangeDetectionState::CalibrationNeeded) {
      transitionState(idx, ChangeDetectionState::CalibrationNeeded,
                      timestampMs);
      fireEvent(idx, ChangeDetectionEventType::CALIBRATION_NEEDED, timestampMs);
    }
    return;  // Skip normal state machine until calibration is complete
  }

  // SIGNAL QUALITY CHECK: Monitor sensor health
  // PRIORITY: HIGH - Check signal quality if calibrated
  if (scale.signalQualityPercent < 20 || scale.consecutiveErrors >= 10) {
    if (scale.state != ChangeDetectionState::LoadCellError) {
      transitionState(idx, ChangeDetectionState::LoadCellError, timestampMs);
      // Event is already fired by updateSignalQuality() on first error
    }
  }

  // Update state machine
  uint64_t timeInState = timestampMs - scale.stateEntryTimeMs;

  switch (scale.state) {
    case ChangeDetectionState::Idle:
      if (!isWeightValid(currentWeight, idx)) {
        transitionState(idx, ChangeDetectionState::InvalidWeight, timestampMs);
        fireInvalidWeightEvent(idx, currentWeight, timestampMs);
      } else if (isWeightAbsent(currentWeight)) {
        scale.previousWeight = 0.0f;
        scale.stableWeight = currentWeight;
        transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::KEG_REMOVED, timestampMs);
      } else {
        transitionState(idx, ChangeDetectionState::Settling, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::SETTLING_STARTED, timestampMs);
      }
      break;

    case ChangeDetectionState::Settling:
      if (!isWeightValid(currentWeight, idx)) {
        transitionState(idx, ChangeDetectionState::InvalidWeight, timestampMs);
        fireInvalidWeightEvent(idx, currentWeight, timestampMs);
      } else if (isWeightAbsent(currentWeight)) {
        scale.previousWeight = scale.stableWeight;
        scale.stableWeight = currentWeight;
        transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::KEG_REMOVED, timestampMs);
      } else {
        // In Settling, simply wait for weight to stabilize
        // The stableWeight will be set to currentWeight when transitioning to Stable
        // IMPORTANT: Calculate slope on the STABLE filter (Kalman), not the fast filter
        // The stable filter has the most meaningful slope for determining true convergence
        calculateSlope(idx, currentWeight, timestampMs);
        float recentSlope = getAverageSlope(idx);
        // STRICTER threshold: avgSlope > -minPourSlope/100 (~0.0015 kg/s)
        // This is 10x stricter than before to ensure filters have truly reached steady state
        // Raw sensor fluctuation is ±0.001-0.002 kg, Kalman needs time to settle
        bool isNearZeroSlope = (recentSlope > _minPourSlope / 100.0f);  // Extremely small slope (> -0.0015)
        
        // CRITICAL: Track sustained stability, not just one-time check
        // If slope exceeds threshold, reset the stability timer
        if (!isNearZeroSlope) {
          // Slope went out of acceptable range - reset timer
          scale.lastSlopeThresholdExceededMs = timestampMs;
        }
        
        // Check if weight has been stable (slope within threshold) for sustained 6 seconds
        uint64_t timeSinceExceeded = timestampMs - scale.lastSlopeThresholdExceededMs;
        bool isSustainedlyStable = isNearZeroSlope && (timeSinceExceeded >= _levelStabilizationDurationMs);
        
        // Check extended timeout as fallback (still within stability window)
        const uint64_t extendedStabilizationMs = _levelStabilizationDurationMs * 5;
        bool timeoutReached = timeInState >= extendedStabilizationMs;
        bool timeoutStabilized = timeoutReached && isWithinStabilityWindow(currentWeight, currentWeight);
        
        if (isSustainedlyStable || timeoutStabilized) {
          // Weight has settled - transition to Stable and set the baseline
          Log.notice(
              F("CD: Weight stabilized after change - settling at %F kg, "
                "slope: %F kg/s, method: %s" CR),
              currentWeight, recentSlope,
              isSustainedlyStable ? "sustained_slope" : (timeoutReached ? "timeout" : "stability window"));
          
          scale.stableWeight = currentWeight;  // Set baseline when entering Stable
          transitionState(idx, ChangeDetectionState::Stable, timestampMs);
          fireEvent(idx, ChangeDetectionEventType::STABLE_LEVEL, timestampMs);
          
          resetDualFilterSlope(idx);
        }
        // Otherwise: still settling - just keep waiting
      }
      break;

    case ChangeDetectionState::Stable: {
      if (!isWeightValid(currentWeight, idx)) {
        transitionState(idx, ChangeDetectionState::InvalidWeight, timestampMs);
        fireInvalidWeightEvent(idx, currentWeight, timestampMs);
      } else if (isWeightAbsent(currentWeight)) {
        scale.previousWeight = scale.stableWeight;
        scale.stableWeight = currentWeight;
        transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::KEG_REMOVED, timestampMs);
      } else if (currentWeight >=
                 scale.stableWeight + _levelIncreaseThreshold) {
        // Large increase (> 0.4kg): weight added to scale, need to restabilize
        // Require 3 consecutive readings to confirm weight addition (prevents sensor spikes)
        scale.largeIncreaseReadings++;
        Log.verbose(
            F("CD: STABLE - weight increase check: current=%F, stable=%F, delta=%F, readings=%d/3" CR),
            currentWeight, scale.stableWeight, currentWeight - scale.stableWeight, scale.largeIncreaseReadings);
        if (scale.largeIncreaseReadings >= 3) {
          // Confirmed: 3+ consecutive readings show weight increase
          Log.notice(
              F("CD: STABLE - large weight increase CONFIRMED (+%F kg, %d readings), entering SETTLING" CR),
              currentWeight - scale.stableWeight, scale.largeIncreaseReadings);
          // Reset slope tracking before transition
          resetDualFilterSlope(idx);
          scale.prePourWeight = 0.0f;  // Clear any previous pour weight (this is a weight increase, not a pour)
          Log.notice(F("CD: STABLE->SETTLING (weight increase). stableWeight=%F, prePourWeight cleared" CR), 
            scale.stableWeight);
          transitionState(idx, ChangeDetectionState::Settling, timestampMs);
          fireEvent(idx, ChangeDetectionEventType::WEIGHT_CHANGE_DETECTED, timestampMs);
        }
      } else {
        // Weight back to normal range - reset counter
        if (scale.largeIncreaseReadings > 0) {
          Log.verbose(
              F("CD: STABLE - weight back to normal (current=%F, stable=%F), resetting increase counter from %d" CR),
              currentWeight, scale.stableWeight, scale.largeIncreaseReadings);
          scale.largeIncreaseReadings = 0;
        }
        
        // Detect weight drops that may be pours
        calculateDualFilterSlope(idx, result, timestampMs);
        float rawWeightDrop = scale.stableWeight - currentWeight;
        
        // If weight is dropping with significant slope, enter POURING state
        // The exit criteria in POURING state will determine if it's a real pour
        if (isPourDetected(idx)) {
          // Entering POURING state - slope indicates weight is dropping
          Log.notice(
              F("CD: STABLE->POURING - weight drop detected: %F kg" CR),
              rawWeightDrop);
          scale.prePourWeight = scale.stableWeight;
          // resetDualFilterSlope(idx); -- REMOVED: Continue slope tracking to catch unrealistic drops
          resetSlope(idx);  // Also reset regular slope to clear historical accumulation
          scale.pourVolume = 0.0f;
          transitionState(idx, ChangeDetectionState::Pouring, timestampMs);
          fireEvent(idx, ChangeDetectionEventType::POURING, timestampMs);
        } else if (currentWeight <= scale.stableWeight - _levelDecreaseThreshold) {
          // Weight decrease detected but too slow for slope trigger
          // Require 3 consecutive readings to confirm (prevents sensor spikes)
          scale.steepSlopeReadings++;
          if (scale.steepSlopeReadings >= 3) {
            Log.notice(
                F("CD: STABLE - slow weight decrease CONFIRMED (-%F kg, %d readings), entering SETTLING" CR),
                scale.stableWeight - currentWeight, scale.steepSlopeReadings);
            // resetDualFilterSlope(idx); -- REMOVED: Continue slope tracking
            scale.prePourWeight = 0.0f;
            transitionState(idx, ChangeDetectionState::Settling, timestampMs);
            fireEvent(idx, ChangeDetectionEventType::WEIGHT_CHANGE_DETECTED, timestampMs);
            scale.steepSlopeReadings = 0;
          }
        } else if (isWithinStabilityWindow(currentWeight, scale.stableWeight)) {
          // Small drift within tolerance: continue monitoring but don't update
          // stableWeight until we transition to a new stable state
          scale.steepSlopeReadings = 0;
        } else {
          // Reset counter if no significant decrease
          scale.steepSlopeReadings = 0;
        }
      }
      break;  // End STABLE state
    }

    case ChangeDetectionState::Pouring: {
      if (!isWeightValid(currentWeight, idx)) {
        transitionState(idx, ChangeDetectionState::InvalidWeight, timestampMs);
        fireInvalidWeightEvent(idx, currentWeight, timestampMs);
      } else if (isWeightAbsent(currentWeight)) {
        scale.previousWeight = scale.stableWeight;
        scale.stableWeight = currentWeight;
        transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::KEG_REMOVED, timestampMs);
      } else {
        // Calculate slope for pour detection (use dual filter)
        calculateDualFilterSlope(idx, result, timestampMs);

        // Update current pour volume (for real-time display)
        const float currentWeightDrop = scale.prePourWeight - currentWeight;
        WeightVolumeConverter converter(idx);
        scale.pourVolume = converter.weightToVolume(currentWeightDrop);

        float currentSlope = getAverageSlope(idx);
        
        // Check for unrealistically fast drop (slope steeper than max realistic pour rate)
        // This indicates item removal, not an actual pour - exit to SETTLING
        if (currentSlope < _minPourSlope) {
          // Steep slope detected - but require 3 consecutive readings to confirm weight removal
          // (prevents false detection from single transient slope spike)
          scale.steepSlopeReadings++;
          if (scale.steepSlopeReadings >= 3) {
            // Confirmed: 3+ consecutive steep slope readings indicate weight removal
            Log.notice(
                F("CD: POURING - unrealistic drop rate confirmed (%F kg/s < %F, %d readings), exiting to SETTLING (item removal)" CR),
                currentSlope, _minPourSlope, scale.steepSlopeReadings);
            scale.prePourWeight = 0.0f;  // Clear pour tracking
            transitionState(idx, ChangeDetectionState::Settling, timestampMs);
            fireEvent(idx, ChangeDetectionEventType::SETTLING_STARTED, timestampMs);
          } else {
            Log.verbose(
                F("CD: POURING - steep slope detected (%F kg/s), need %d more readings to confirm" CR),
                currentSlope, 3 - scale.steepSlopeReadings);
          }
        } else {
          // Slope back within normal range - reset counter
          if (scale.steepSlopeReadings > 0) {
            Log.verbose(
                F("CD: POURING - slope normalized, resetting steep slope counter" CR));
            scale.steepSlopeReadings = 0;
          }
          
          if (currentSlope > _pourSlopeThreshold) {
          // Pour complete - slope has returned to positive/near-zero
          // Apply SAME stabilization criteria as SETTLING state for exact weight locking:
          // - Slope must be near zero (sustained stability)
          // - Must sustain this for full stabilization duration (6 seconds)
          // - Or hit extended timeout as fallback
          // This ensures new level is stable and exact, and naturally captures multi-glass pours
          
          // Track sustained stability (like SETTLING does)
          bool isNearZeroSlope = (currentSlope > _minPourSlope / 100.0f);  // Extremely small slope (> -0.0015)
          
          // If slope exceeds threshold, reset the stability timer
          if (!isNearZeroSlope) {
            scale.lastSlopeThresholdExceededMs = timestampMs;
          }
          
          // Check if weight has been stable for sustained period (same as SETTLING)
          uint64_t timeSinceExceeded = timestampMs - scale.lastSlopeThresholdExceededMs;
          bool isSustainedlyStable = isNearZeroSlope && (timeSinceExceeded >= _levelStabilizationDurationMs);
          
          // Check extended timeout as fallback (like SETTLING does)
          const uint64_t extendedStabilizationMs = _levelStabilizationDurationMs * 5;
          bool timeoutReached = timeInState >= extendedStabilizationMs;
          bool timeoutStabilized = timeoutReached && isWithinStabilityWindow(currentWeight, currentWeight);
          
          if (isSustainedlyStable || timeoutStabilized) {
            // Weight has stabilized using exact same criteria as SETTLING
            // Now complete the pour and lock the baseline
            if (currentWeightDrop >= _levelDecreaseThreshold) {
              // Valid pour - weight drop significant enough
              if (!isWeightAbsent(currentWeight)) {
                scale.stableWeight = currentWeight;  // Lock new baseline (exact weight after full stabilization)
                transitionState(idx, ChangeDetectionState::Stable, timestampMs);
                firePourCompletedWithSplitting(idx, timestampMs);
                Log.verbose(F("CD: POURING->STABLE transition. stableWeight now=%F (after full stabilization), prePourWeight cleared" CR), scale.stableWeight);
                scale.prePourWeight = 0.0f;  // Clear pour tracking
              } else {
                // Keg removed during pour
                scale.previousWeight = scale.stableWeight;
                scale.stableWeight = currentWeight;
                transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
                fireEvent(idx, ChangeDetectionEventType::KEG_REMOVED, timestampMs);
                scale.prePourWeight = 0.0f;  // Clear pour tracking
              }
            } else {
              // Weight drop too small - not a real pour, return to STABLE
              Log.notice(
                  F("CD: POURING->STABLE - drop too small (%F kg < %F threshold), not a pour" CR),
                  currentWeightDrop, _levelDecreaseThreshold);
              scale.stableWeight = currentWeight;  // Accept new baseline
              transitionState(idx, ChangeDetectionState::Stable, timestampMs);
              fireEvent(idx, ChangeDetectionEventType::STABLE_LEVEL, timestampMs);
              scale.prePourWeight = 0.0f;  // Clear pour tracking
            }
          }
          // else: slope positive but not stabilized long enough - keep waiting for stability
          } else if (timeInState >= _pourDurationMs) {
          // Pour timeout - still dropping but time exceeded
          // Lock baseline and go to STABLE
          if (!isWeightAbsent(currentWeight)) {
            scale.stableWeight = currentWeight;  // Lock baseline at timeout
            transitionState(idx, ChangeDetectionState::Stable, timestampMs);
            firePourCompletedWithSplitting(idx, timestampMs);
            scale.prePourWeight = 0.0f;  // Clear pour tracking
          } else {
            // Keg removed during pour
            scale.previousWeight = scale.stableWeight;
            scale.stableWeight = currentWeight;
            transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
            fireEvent(idx, ChangeDetectionEventType::KEG_REMOVED, timestampMs);
            scale.prePourWeight = 0.0f;  // Clear pour tracking
          }
        }
        }
      }
      break;
    }

    case ChangeDetectionState::KegAbsent: {
      if (isWeightPresent(currentWeight, idx)) {
        // Keg reappeared
        scale.previousWeight = scale.stableWeight;
        scale.stableWeight = currentWeight;
        transitionState(idx, ChangeDetectionState::ReplacingKeg, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::KEG_REPLACED, timestampMs);
      } else {
        // Fire KEG_ABSENT_TIMEOUT every 30-second boundary while keg is absent
        uint64_t timeInState = timestampMs - scale.stateEntryTimeMs;
        uint64_t seconds = timeInState / 1000;
        
        // Fire event if we just crossed a 30-second boundary
        // (seconds is now at 30, 60, 90, etc. and was not before)
        static uint64_t lastAbsentTimeoutSeconds[MAX_SCALES] = {0};
        if (seconds > 0 && seconds % 30 == 0 && lastAbsentTimeoutSeconds[static_cast<int>(idx)] != seconds) {
          lastAbsentTimeoutSeconds[static_cast<int>(idx)] = seconds;
          fireEvent(idx, ChangeDetectionEventType::KEG_ABSENT_TIMEOUT, timestampMs);
        }
      }
      break;
    }

    case ChangeDetectionState::ReplacingKeg: {
      if (isWeightAbsent(currentWeight)) {
        scale.previousWeight = scale.stableWeight;
        scale.stableWeight = currentWeight;
        transitionState(idx, ChangeDetectionState::KegAbsent, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::KEG_REMOVED, timestampMs);
      } else if (isWithinStabilityWindow(currentWeight, scale.stableWeight)) {
        if (timeInState >= _kegReplacementDurationMs) {
          transitionState(idx, ChangeDetectionState::Stable, timestampMs);
          fireEvent(idx, ChangeDetectionEventType::STABLE_LEVEL,
                    timestampMs);
          resetDualFilterSlope(idx);
        }
      } else {
        scale.stableWeight = currentWeight;
        scale.stateEntryTimeMs = timestampMs;  // Reset timer
      }
      break;
    }

    case ChangeDetectionState::InvalidWeight: {
      if (isWeightValid(currentWeight, idx)) {
        transitionState(idx, ChangeDetectionState::Settling, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::SENSOR_RECOVERED, timestampMs);
      }
      break;
    }

    case ChangeDetectionState::CalibrationNeeded: {
      // Calibration State: Waiting for calibration to complete
      // When scaleFactor changes from 0.0f to a valid value, FORCE transition
      // to Idle This triggers full re-initialization: Idle → Settling →
      // Stable
      if (myConfig.getScaleFactor(idx) != 0.0f) {
        transitionState(idx, ChangeDetectionState::Idle, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::CALIBRATION_COMPLETE, timestampMs);
      }
      break;
    }

    case ChangeDetectionState::LoadCellError: {
      // Load Cell Error State: Hardware signal is lost or unreliable
      // Recovery happens when signal quality stabilizes
      if (scale.signalQualityPercent > 80 && scale.consecutiveErrors == 0) {
        // Signal recovered!
        // Transition to Settling to re-establish baseline
        transitionState(idx, ChangeDetectionState::Settling, timestampMs);
        fireEvent(idx, ChangeDetectionEventType::LOAD_CELL_RECOVERED, timestampMs);
      }
      break;
    }

    default:
      break;
  }
}

ChangeDetectionState ChangeDetection::getState(UnitIndex idx) const {
  return _scales[static_cast<int>(idx)].state;
}

float ChangeDetection::getStableWeight(UnitIndex idx) const {
  return _scales[static_cast<int>(idx)].stableWeight;
}

float ChangeDetection::getPouringVolume(UnitIndex idx) const {
  return _scales[static_cast<int>(idx)].pourVolume;
}

float ChangeDetection::getStableVolume(UnitIndex idx) const {
  // Convert stable weight to beer volume using FG
  // First subtract the empty keg weight to get only the beer weight
  float stableWeightKg = getStableWeight(idx);
  if (isnan(stableWeightKg) || stableWeightKg <= 0) {
    return 0;
  }

  float kegWeightKg = myConfig.getKegWeight(idx);
  float beerWeightKg = stableWeightKg - kegWeightKg;

  if (beerWeightKg < 0) {
    return 0;  // Scale reading is less than empty keg weight
  }

  WeightVolumeConverter converter(idx);
  return converter.weightToVolume(beerWeightKg);
}

float ChangeDetection::getLastPourVolume(UnitIndex idx) const {
  // Return the last/max pour volume from statistics
  return getStatistics(idx).maxPourVolume;
}

const char* ChangeDetection::getStateString(UnitIndex idx) const {
  switch (getState(idx)) {
    case ChangeDetectionState::Idle:
      return "Idle";
    case ChangeDetectionState::Settling:
      return "Settling";
    case ChangeDetectionState::Stable:
      return "Stable";
    case ChangeDetectionState::Pouring:
      return "Pouring";
    case ChangeDetectionState::KegAbsent:
      return "KegAbsent";
    case ChangeDetectionState::ReplacingKeg:
      return "ReplacingKeg";
    case ChangeDetectionState::InvalidWeight:
      return "InvalidWeight";
    case ChangeDetectionState::LoadCellError:
      return "LoadCellError";
    case ChangeDetectionState::CalibrationNeeded:
      return "CalibrationNeeded";
    default:
      return "Unknown";
  }
}

float ChangeDetection::getConfidence(UnitIndex idx) const {
  const ChangeDetectionInstance& scale = _scales[static_cast<int>(idx)];
  uint64_t timeInState = (millis() - scale.stateEntryTimeMs);
  float confidence = 0.0f;

  switch (scale.state) {
    case ChangeDetectionState::Settling:
      confidence = (static_cast<float>(timeInState) /
                    static_cast<float>(_levelStabilizationDurationMs)) *
                   100.0f;
      break;
    case ChangeDetectionState::Pouring:
      confidence = (static_cast<float>(timeInState) /
                    static_cast<float>(_pourDurationMs)) *
                   100.0f;
      break;
    case ChangeDetectionState::KegAbsent:
      confidence = (static_cast<float>(timeInState) /
                    static_cast<float>(_kegAbsenceDurationMs)) *
                   100.0f;
      break;
    case ChangeDetectionState::ReplacingKeg:
      confidence = (static_cast<float>(timeInState) /
                    static_cast<float>(_kegReplacementDurationMs)) *
                   100.0f;
      break;
    case ChangeDetectionState::Stable:
      confidence = 100.0f;
      break;
    default:
      confidence = 0.0f;
      break;
  }

  return std::min(confidence, 100.0f);
}

bool ChangeDetection::getNextEvent(ChangeDetectionEvent& event) {
  return _eventQueue.pop(event);
}

bool ChangeDetection::hasQueuedEvents() const { return !_eventQueue.isEmpty(); }

size_t ChangeDetection::getPendingEventCount() const {
  return _eventQueue.count();
}

uint8_t ChangeDetection::getSignalQuality(UnitIndex idx) const {
  if (static_cast<int>(idx) < 0 || static_cast<int>(idx) >= MAX_SCALES) {
    return 0;
  }
  return _scales[static_cast<int>(idx)].signalQualityPercent;
}

void ChangeDetection::updateSignalQuality(UnitIndex idx, bool isValid,
                                          SignalErrorReason reason,
                                          float variance,
                                          uint64_t timestampMs) {
  int idx_int = static_cast<int>(idx);
  if (idx_int < 0 || idx_int >= MAX_SCALES) return;

  // Suppress signal quality events during calibration to avoid noise
  if (myScale.isCalibrating(idx)) {
    return;
  }

  // Calibration errors are handled by the state machine's CALIBRATION_NEEDED state
  // and do not contribute to hardware signal quality score.
  if (reason == SignalErrorReason::CALIBRATION_INVALID) {
    return;
  }

  ChangeDetectionInstance& scale = _scales[idx_int];

  if (isValid) {
    // Signal is good
    // Only fire RECOVERED if we were previously in an error state (had
    // consecutive errors)
    bool wasInErrorState = scale.consecutiveErrors > 0;

    scale.consecutiveErrors = 0;
    scale.signalQualityPercent = 100;

    // Transition from error → recovered?
    if (wasInErrorState && !scale.signalWasValid) {
      ChangeDetectionEvent event;
      event.type = ChangeDetectionEventType::LOAD_CELL_RECOVERED;
      event.unitIndex = idx;
      event.timestampMs = timestampMs;
      event.loadCell.reason = reason;
      event.loadCell.signalQualityPercent = 100;
      event.loadCell.varianceKg = variance;
      event.loadCell.consecutiveErrors = 0;

      _eventQueue.push(event);
      Log.notice(F("CHGD: Load cell signal recovered [%d]." CR), idx_int);
    }
    scale.signalWasValid = true;
  } else {
    // Signal is bad - increment error counter
    bool isFirstError = (scale.consecutiveErrors == 0);

    scale.consecutiveErrors++;
    scale.lastErrorReason = reason;

    // Quality decreases with more consecutive errors (100% → 0%)
    scale.signalQualityPercent =
        static_cast<uint8_t>(100 - (scale.consecutiveErrors * 5));
    if (scale.signalQualityPercent < 0) scale.signalQualityPercent = 0;

    // Fire error event on first error (regardless of previous state)
    if (isFirstError) {
      ChangeDetectionEvent event;
      event.type = ChangeDetectionEventType::LOAD_CELL_ERROR;
      event.unitIndex = idx;
      event.timestampMs = timestampMs;
      event.loadCell.reason = reason;
      event.loadCell.signalQualityPercent = scale.signalQualityPercent;
      event.loadCell.varianceKg = variance;
      event.loadCell.consecutiveErrors = scale.consecutiveErrors;

      _eventQueue.push(event);
      Log.notice(
          F("CHGD: Load cell error - reason=%d quality=%d%% [%d]." CR),
          static_cast<uint8_t>(reason), scale.signalQualityPercent,
          idx_int);
    }
    scale.signalWasValid = false;
  }
}

#if defined(INJECT_TEST_EVENT)
void ChangeDetection::injectTestEvents() {
  uint64_t now = millis();
  ChangeDetectionEvent event = {};

  // Inject test events for first scale to test all integration paths
  UnitIndex idx = UnitIndex::U1;

  // STABLE_LEVEL event - triggers pushKegInformation + sendEventState
  event.type = ChangeDetectionEventType::STABLE_LEVEL;
  event.unitIndex = idx;
  event.timestampMs = now;
  // stableVolumeL is now calculated from stableWeightKg
  event.stable.stableWeightKg = 20.0f;
  _eventQueue.push(event);
  Log.notice(F("TEST: Injected STABLE_LEVEL for scale [%d]" CR), static_cast<int>(idx));

  // POURING event - triggers sendEventState
  event.type = ChangeDetectionEventType::POURING;
  event.unitIndex = idx;
  event.timestampMs = now + 500;
  _eventQueue.push(event);
  Log.notice(F("TEST: Injected POURING for scale [%d]" CR), static_cast<int>(idx));

  // POUR_COMPLETED event - triggers pushPourInformation + sendEventState
  event.type = ChangeDetectionEventType::POUR_COMPLETED;
  event.unitIndex = idx;
  event.timestampMs = now + 2000;
  event.pour.prePourWeightKg = 20.0f;
  event.pour.postPourWeightKg = 18.5f;
  event.pour.pourVolumeL = 0.375f;  // ~12.6 oz
  _eventQueue.push(event);
  Log.notice(F("TEST: Injected POUR_COMPLETED for scale [%d]" CR), static_cast<int>(idx));

  // KEG_REMOVED event - triggers Brewspy.clearKegInformation + sendEventState
  event.type = ChangeDetectionEventType::KEG_REMOVED;
  event.unitIndex = idx;
  event.timestampMs = now + 3000;
  _eventQueue.push(event);
  Log.notice(F("TEST: Injected KEG_REMOVED for scale [%d]" CR), static_cast<int>(idx));

  // KEG_REPLACED event - triggers sendEventState
  event.type = ChangeDetectionEventType::KEG_REPLACED;
  event.unitIndex = idx;
  event.timestampMs = now + 3500;
  _eventQueue.push(event);
  Log.notice(F("TEST: Injected KEG_REPLACED for scale [%d]" CR), static_cast<int>(idx));
}
#endif  // INJECT_TEST_EVENT

// EOF
