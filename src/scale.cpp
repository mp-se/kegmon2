/*
MIT License

Copyright (c) 2021-2026 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#include <changedetection.hpp>
#include <kegpush.hpp>
#include <memory>
#include <perf.hpp>
#include <scale.hpp>
#include <scale_filter_pipeline.hpp>

bool Scale::isReady(UnitIndex idx) const {
  if (!_hxScale[idx]) {
    Log.verbose(F("SCAL: HX711 scale not ready [%d]." CR), idx);
    return false;
  }
  return _hxScale[idx]->wait_ready_retry(3, 1000);
}

void Scale::tare(UnitIndex idx) {
  if (!_hxScale[idx]) return;

  Log.notice(
      F("SCAL: HX711 set scale to zero, prepare for calibration %d [%d]." CR),
      myConfig.getScaleReadCountCalibration(), idx);

  _hxScale[idx]->set_scale(1.0);
  _hxScale[idx]->tare(myConfig.getScaleReadCountCalibration());
  int32_t l = _hxScale[idx]->get_offset();
  // Log.verbose(F("SCAL: HX711 New scale offset found %l [%d]." CR), l, idx);
  myConfig.setScaleOffset(idx, l);
  myConfig.setScaleFactor(idx,
                          FACTOR_UNCALIBRATED);  // Reset to uncalibrated, must
                                                 // call findFactor() next
  myConfig.saveFile();
}

void Scale::findFactor(UnitIndex idx, float weight) {
  if (!_hxScale[idx]) return;

  int idx_int = static_cast<int>(idx);
  _isCalibrating[idx_int] = true;  // Suppress events during calibration

  float l = _hxScale[idx]->get_units(myConfig.getScaleReadCountCalibration());
  float f = l / weight;
  Log.notice(F("SCAL: HX711 Detecting factor for weight %F, raw %F, factor %F "
               "[%d]." CR),
             weight, l, f, idx);

  if (isinf(f) || isnan(f)) f = 0.0;

  myConfig.setScaleFactor(idx, f);
  myConfig.saveFile();  // save the factor to file

  setScaleFactor(idx);  // apply the factor after it has been saved

  // Reset filters so they warm up with fresh data from newly calibrated scale
  resetFilters(idx);

  // Warmup by taking 10 raw readings (without filter processing) to stabilize
  // sensor This allows the HX711 ADC to settle with the new calibration factor
  // applied
  Log.notice(F("SCAL: Stabilizing sensor for scale [%d]..." CR), idx_int);
  for (int i = 0; i < 10; i++) {
    // Read raw value directly from HX711 without updating filters
    _hxScale[idx]->get_units(myConfig.getScaleReadCount());
    delay(50);  // Let sensor settle
  }

  Log.notice(F("SCAL: Sensor stabilization complete for scale [%d]." CR),
             idx_int);

  _isCalibrating[idx_int] = false;  // Re-enable event generation
}

float Scale::readRaw(UnitIndex idx) {
  // Log.verbose(F("SCAL: HX711 Reading raw scale for [%d]." CR), idx);
  if (!_hxScale[idx]) return 0;
  float l = _hxScale[idx]->read_average(
      myConfig.getScaleReadCountCalibration());  // get the raw value without
                                                 // applying scaling factor
  _lastRaw[idx] = l;
  // Log.verbose(F("SCAL: HX711 Reading scale raw weight=%d [%d]" CR), l, idx);
  return l;
}

ScaleReadingResult Scale::read(UnitIndex idx) {
  if (!_hxScale[idx]) {
    myChangeDetection.updateSignalQuality(
        idx, false, SignalErrorReason::TIMEOUT, 0.0f, millis());
    return ScaleReadingResult::createInvalidResult();
  }

  if (myConfig.getScaleFactor(idx) == FACTOR_UNCALIBRATED ||
      myConfig.getScaleOffset(idx) == 0) {  // Not initialized
    // Log.verbose(F("SCAL: HX711 has no configuration [%d]." CR), idx);
    myChangeDetection.updateSignalQuality(
        idx, false, SignalErrorReason::CALIBRATION_INVALID, 0.0f, millis());
    return ScaleReadingResult::createInvalidResult();
  }

  // Log.verbose(F("SCAL: HX711 reading scale for [%d]." CR), idx);
  _hxScale[idx]->set_medavg_mode();
  float raw = _hxScale[idx]->get_units(myConfig.getScaleReadCount());
  // Log.verbose(F("SCAL: HX711 Reading weight=%F [%d]" CR), raw, idx);

  // Check for NAN reading (timeout from HX711)
  if (isnan(raw)) {
    myChangeDetection.updateSignalQuality(
        idx, false, SignalErrorReason::NAN_VALUE, 0.0f, millis());
    _stats.recordReading(static_cast<int>(idx), raw, false, millis());
    return ScaleReadingResult::createInvalidResult();
  }

  // Validate raw reading against hardware sensor limits (±50kg)
  constexpr float MAX_VALID_SENSOR_KG = 50.0f;
  constexpr float MIN_VALID_SENSOR_KG = -50.0f;

  bool isValid = true;
  if (raw > MAX_VALID_SENSOR_KG) {
    // Log.error(F("SCAL: HX711 Ignoring value since it's higher than %F kg, %F
    // "
    //             "[%d]." CR),
    //           MAX_VALID_SENSOR_KG, raw, idx);
    isValid = false;
    myChangeDetection.updateSignalQuality(
        idx, false, SignalErrorReason::OUT_OF_RANGE, 0.0f, millis());
    _stats.recordReading(static_cast<int>(idx), raw, isValid, millis());
    return ScaleReadingResult::createInvalidResult();
  }

  if (raw < MIN_VALID_SENSOR_KG) {
    // Log.error(
    //     F("SCAL: HX711 Ignoring value since it's less than %F kg %F [%d]."
    //     CR), MIN_VALID_SENSOR_KG, raw, idx);
    isValid = false;
    myChangeDetection.updateSignalQuality(
        idx, false, SignalErrorReason::OUT_OF_RANGE, 0.0f, millis());
    _stats.recordReading(static_cast<int>(idx), raw, isValid, millis());
    return ScaleReadingResult::createInvalidResult();
  }

  // Apply all filters through the filter pipeline
  uint64_t timestampMs = millis();
  ScaleReadingResult result = _filterPipeline[static_cast<int>(idx)]->update(raw, timestampMs);

  _lastResult[idx] = result;

  // Record statistics and signal quality for valid reading
  _stats.recordReading(static_cast<int>(idx), raw, isValid, timestampMs);

  // Update _lastRaw for API access
  _lastRaw[idx] = raw;

  // Calculate variance across filters as indicator of data jitter
  float values[] = {result.moving_average, result.ema, result.weighted_ma,
                    result.median, result.kalman};
  float mean = 0.0f;
  for (float v : values) mean += v;
  mean /= sizeof(values) / sizeof(values[0]);

  float variance = 0.0f;
  for (float v : values) {
    float diff = v - mean;
    variance += diff * diff;
  }
  variance = sqrt(variance / (sizeof(values) / sizeof(values[0])));

  // Report valid signal
  myChangeDetection.updateSignalQuality(idx, true, SignalErrorReason::TIMEOUT,
                                        variance, millis());
  return result;
}

void Scale::setupScale(UnitIndex idx, bool force, int pinData, int pinClock) {
  if (!_hxScale[idx] || force) {
    // Log.verbose(F("SCAL: HX711 initializing scale, using offset %l [%d]."
    // CR),
    //             myConfig.getScaleOffset(idx), idx);
    _hxScale[idx] = std::make_unique<HX711>();
    _dataPins[static_cast<int>(idx)] = pinData;  // Store data pin for later use
    _hxScale[idx]->begin(pinData, pinClock, true, false);

    _hxScale[idx]->set_offset(myConfig.getScaleOffset(idx));

    Log.notice(F("SCAL: Initializing HX711 on pins Data=%d,Clock=%d [%d]." CR),
               pinData, pinClock, idx);

    setScaleFactor(idx);

    if (_hxScale[idx]->is_ready()) {
      Log.notice(F("SCAL: HX711 scale found [%d]." CR), idx);
    } else {
      Log.error(
          F("SCAL: HX711 scale not responding, disabling interface [%d]." CR),
          idx);
      _hxScale[idx].reset();
    }
  }
}

void Scale::setScaleFactor(UnitIndex idx) {
  if (!_hxScale[idx]) return;

  float fs = myConfig.getScaleFactor(idx);

  // Protect against zero or NAN values from corrupted config
  if (isnan(fs) || fs == 0.0) fs = 1.0;

  _hxScale[idx]->set_scale(fs);
}

void Scale::setup(bool force) {
  if (MAX_SCALES > 0)
    setupScale(UnitIndex::U1, force, PIN_SCALE_SDA1, PIN_SCALE_SCK1);

  if (MAX_SCALES > 1)
    setupScale(UnitIndex::U2, force, PIN_SCALE_SDA2, PIN_SCALE_SCK2);

  if (MAX_SCALES > 2)
    setupScale(UnitIndex::U3, force, PIN_SCALE_SDA3, PIN_SCALE_SCK3);

  if (MAX_SCALES > 3)
    setupScale(UnitIndex::U4, force, PIN_SCALE_SDA4, PIN_SCALE_SCK4);

  // Initialize filter pipelines for each scale
  for (int i = 0; i < MAX_SCALES; ++i) {
    _filterPipeline[i] = std::make_unique<ScaleFilterPipeline>();
  }

  Log.notice(F("SCAL: All filter pipelines initialized for all 4 scales."));
}

void Scale::loop() {
  for (int i = 0; i < MAX_SCALES; i++) loopScale((UnitIndex)i);
}

void Scale::loopScale(UnitIndex idx) {
  if (!isConnected(idx)) return;

  if (_sched[idx].tare) {
    // Log.verbose(F("SCAL: Tare triggered [%d]." CR), idx);
    tare(idx);
    readRaw(idx);
    _sched[idx].tare = false;
  }

  if (_sched[idx].findFactor) {
    // Log.verbose(F("SCAL: Find factor triggered [%d]." CR), idx);
    findFactor(idx, _sched[idx].factorWeight);
    readRaw(idx);
    _sched[idx].findFactor = false;
  }

  // Attempt sampling rate detection with retries (up to 3 attempts)
  int idx_int = static_cast<int>(idx);
  if (_detectionRetries[idx_int] < 3 && isConnected(idx)) {
    uint8_t result = detectSamplingRate(idx);
    _detectionRetries[idx_int]++;

    if (result > 0) {
      // Successfully detected, don't retry anymore
      _detectionRetries[idx_int] = 3;
    }
  }

  // Update all filters with latest reading
  read(idx);
}

uint8_t Scale::detectSamplingRate(UnitIndex idx) {
  int idx_int = static_cast<int>(idx);

  const int DATA_PIN = _dataPins[idx_int];
  if (DATA_PIN < 0 || !_hxScale[idx]) {
    Log.notice(F("SCAL: [%d] not initialized." CR), idx_int);
    return 0;
  }

  uint8_t attempt = _detectionRetries[idx_int] + 1;
  Log.notice(F("SCAL: [%d] detecting sampling rate (attempt %d/3)." CR),
             idx_int, attempt);

  // Count pin transitions to estimate sampling rate
  // At 10 SPS: ~10 transitions per second
  // At 80 SPS: ~80 transitions per second

  const uint32_t SAMPLE_TIME_MS = 500;  // Sample for 500ms
  const uint32_t TRANSITION_THRESHOLD =
      25;  // ~12.5 transitions per 500ms = 25/1000ms threshold

  uint32_t start = millis();
  int transition_count = 0;
  int last_state = digitalRead(DATA_PIN);

  // Count transitions for SAMPLE_TIME_MS
  while (millis() - start < SAMPLE_TIME_MS) {
    int current_state = digitalRead(DATA_PIN);
    if (current_state != last_state) {
      transition_count++;
      last_state = current_state;
      delay(1);  // Small delay to avoid bouncing
    }
  }

  Log.notice(F("SCAL: [%d] transitions=%d in %d ms." CR), idx_int,
             transition_count, SAMPLE_TIME_MS);

  // Determine rate based on transition count
  // 10 SPS = ~10 transitions per second = ~5 in 500ms
  // 80 SPS = ~80 transitions per second = ~40 in 500ms
  uint8_t detected_rate = 0;

  if (transition_count < TRANSITION_THRESHOLD) {
    detected_rate = 10;
    Log.notice(F("SCAL: [%d] low transitions (%d) -> 10 SPS." CR), idx_int,
               transition_count);
  } else {
    detected_rate = 80;
    Log.notice(F("SCAL: [%d] high transitions (%d) -> 80 SPS." CR), idx_int,
               transition_count);
  }

  _detectedSamplingRate[idx_int] = detected_rate;
  if (detected_rate > 0) {
    Log.notice(F("SCAL: [%d] Sampling rate = %d sps." CR), idx_int,
               detected_rate);
  }

  return detected_rate;
}

void Scale::resetFilters(UnitIndex idx) {
  int idx_int = static_cast<int>(idx);

  if (_filterPipeline[idx_int]) {
    _filterPipeline[idx_int]->resetFilters();
    Log.notice(F("SCAL: Filters reset for scale [%d]." CR), idx_int);
  }
}

// EOF
