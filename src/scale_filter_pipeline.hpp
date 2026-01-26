/*
MIT License

Copyright (c) 2026 Magnus

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
#ifndef SRC_SCALE_FILTER_PIPELINE_HPP_
#define SRC_SCALE_FILTER_PIPELINE_HPP_

#include <filters/filter_alphabeta.hpp>
#include <filters/filter_butterworth.hpp>
#include <filters/filter_chebyshev.hpp>
#include <filters/filter_complementary.hpp>
#include <filters/filter_ema.hpp>
#include <filters/filter_hampel.hpp>
#include <filters/filter_kalman.hpp>
#include <filters/filter_median.hpp>
#include <filters/filter_movingaverage.hpp>
#include <filters/filter_weightedma.hpp>
#include <filters/filter_zscore.hpp>
#include <main.hpp>
#include <memory>

// Slope tracking for a single filter (rate of change metric)
struct SlopeMetrics {
  float currentSlope = 0.0f;       // Most recent slope (kg/sec)
  float accumulatedSlope = 0.0f;   // Sum of all slope readings
  int slopeReadings = 0;           // Number of slope readings accumulated
  float previousValue = 0.0f;      // Previous filter value for calculating delta
  uint64_t lastUpdateMs = 0;       // Timestamp of last update

  // Get average slope across all readings
  float getAverageSlope() const {
    return (slopeReadings > 0) ? (accumulatedSlope / slopeReadings) : 0.0f;
  }

  // Reset slope tracking
  void reset() {
    currentSlope = 0.0f;
    accumulatedSlope = 0.0f;
    slopeReadings = 0;
    previousValue = 0.0f;
    lastUpdateMs = 0;
  }
};

// Result structure containing raw reading and all filter outputs
struct ScaleReadingResult {
  float raw = 0.0f;
  float moving_average = 0.0f;
  float ema = 0.0f;
  float weighted_ma = 0.0f;
  float median = 0.0f;
  float zscore = 0.0f;
  float hampel = 0.0f;
  float complementary = 0.0f;
  float alphabeta = 0.0f;
  float butterworth = 0.0f;
  float chebyshev = 0.0f;
  float kalman = 0.0f;

  // Get filter value by FilterType enum
  inline float getFilterValue(FilterType filter) const {
    switch (filter) {
      case FilterType::FILTER_RAW:
        return raw;
      case FilterType::FILTER_MOVING_AVERAGE:
        return moving_average;
      case FilterType::FILTER_EMA:
        return ema;
      case FilterType::FILTER_WEIGHTED_MA:
        return weighted_ma;
      case FilterType::FILTER_MEDIAN:
        return median;
      case FilterType::FILTER_ZSCORE:
        return zscore;
      case FilterType::FILTER_HAMPEL:
        return hampel;
      case FilterType::FILTER_COMPLEMENTARY:
        return complementary;
      case FilterType::FILTER_ALPHABETA:
        return alphabeta;
      case FilterType::FILTER_BUTTERWORTH:
        return butterworth;
      case FilterType::FILTER_CHEBYSHEV:
        return chebyshev;
      case FilterType::FILTER_KALMAN:
        return kalman;
      default:
        return NAN;
    }
  }

  // Get filter name by FilterType enum
  inline const char* getFilterName(FilterType filter) const {
    switch (filter) {
      case FilterType::FILTER_RAW:
        return "raw";
      case FilterType::FILTER_MOVING_AVERAGE:
        return "moving_average";
      case FilterType::FILTER_EMA:
        return "ema";
      case FilterType::FILTER_WEIGHTED_MA:
        return "weighted_ma";
      case FilterType::FILTER_MEDIAN:
        return "median";
      case FilterType::FILTER_ZSCORE:
        return "zscore";
      case FilterType::FILTER_HAMPEL:
        return "hampel";
      case FilterType::FILTER_COMPLEMENTARY:
        return "complementary";
      case FilterType::FILTER_ALPHABETA:
        return "alphabeta";
      case FilterType::FILTER_BUTTERWORTH:
        return "butterworth";
      case FilterType::FILTER_CHEBYSHEV:
        return "chebyshev";
      case FilterType::FILTER_KALMAN:
        return "kalman";
      default:
        return "unknown";
    }
  }

  // Create an invalid reading result with all fields set to NAN
  static ScaleReadingResult createInvalidResult() {
    ScaleReadingResult result;
    result.raw = NAN;
    result.moving_average = NAN;
    result.ema = NAN;
    result.weighted_ma = NAN;
    result.median = NAN;
    result.zscore = NAN;
    result.hampel = NAN;
    result.complementary = NAN;
    result.alphabeta = NAN;
    result.butterworth = NAN;
    result.chebyshev = NAN;
    result.kalman = NAN;
    return result;
  }
};

/**
 * ScaleFilterPipeline manages all filter instances for a single scale sensor.
 * It applies a raw reading through a bank of filters and returns all filter outputs.
 * This separates filter management from the Scale class, making both simpler.
 */
class ScaleFilterPipeline {
 private:
  std::unique_ptr<ScaleFilter> _filter_ma;
  std::unique_ptr<ScaleFilter> _filter_ema;
  std::unique_ptr<ScaleFilter> _filter_wma;
  std::unique_ptr<ScaleFilter> _filter_median;
  std::unique_ptr<ScaleFilter> _filter_zscore;
  std::unique_ptr<ScaleFilter> _filter_hampel;
  std::unique_ptr<ScaleFilter> _filter_complementary;
  std::unique_ptr<ScaleFilter> _filter_alphabeta;
  std::unique_ptr<ScaleFilter> _filter_butterworth;
  std::unique_ptr<ScaleFilter> _filter_chebyshev;
  std::unique_ptr<ScaleFilter> _filter_kalman;

  // Slope tracking for each filter (for derivative-based analysis)
  SlopeMetrics _slopeMetrics[11];  // One for each filter
  uint64_t _lastUpdateMs = 0;      // Track time between updates
  ScaleReadingResult _lastResult;

 public:
  ScaleFilterPipeline();
  ~ScaleFilterPipeline() = default;

  /**
   * Reset all filters to their initial state.
   * Call after calibration or when sensor parameters change.
   */
  void resetFilters();

  /**
   * Update all filters with a new raw reading and return all filter outputs.
   * Also calculates slope (rate of change) for each filter.
   *
   * @param raw The raw sensor reading to filter
   * @param timestampMs The timestamp of this reading in milliseconds
   * @return ScaleReadingResult containing raw value and all filter outputs
   */
  ScaleReadingResult update(float raw, uint64_t timestampMs);

  /**
   * Calculate slopes for all filters based on current reading.
   * Called automatically by update(), but can be called separately if needed.
   *
   * @param result The current reading results with all filter values
   * @param timestampMs The timestamp of the current reading
   */
  void calculateSlopes(const ScaleReadingResult& result, uint64_t timestampMs);

  /**
   * Get the average slope for a specific filter.
   *
   * @param filterIndex Index of the filter (0-10, matching FilterType order)
   * @return Average slope in kg/sec, or 0.0 if no readings
   */
  float getAverageSlope(int filterIndex) const;

  /**
   * Get the most recent slope for a specific filter.
   *
   * @param filterIndex Index of the filter (0-10)
   * @return Current slope in kg/sec
   */
  float getCurrentSlope(int filterIndex) const;

  /**
   * Get slope metrics for a specific filter (primary Kalman filter by default).
   * Useful for dual-filter slope detection (comparing different filters).
   *
   * @param filterIndex Index of the filter (0-10)
   * @return SlopeMetrics struct with accumulated and current slope data
   */
  SlopeMetrics getSlopeMetrics(int filterIndex) const;

  /**
   * Reset slope tracking for all filters.
   * Call when transitioning between states or starting a new measurement.
   */
  void resetSlopes();

  /**
   * Reset slope tracking for a specific filter.
   *
   * @param filterIndex Index of the filter (0-10)
   */
  void resetSlope(int filterIndex);

  /**
   * Get the last computed filter result without updating.
   *
   * @return The last ScaleReadingResult
   */
  inline ScaleReadingResult getLastResult() const { return _lastResult; }
};

#endif  // SRC_SCALE_FILTER_PIPELINE_HPP_

// EOF
