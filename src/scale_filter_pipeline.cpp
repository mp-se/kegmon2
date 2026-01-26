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
#include <scale_filter_pipeline.hpp>

ScaleFilterPipeline::ScaleFilterPipeline()
    : _filter_ma(std::make_unique<MovingAverageFilter>(5)),
      _filter_ema(std::make_unique<ExponentialMovingAverageFilter>(0.3f)),
      _filter_wma(std::make_unique<WeightedMovingAverageFilter>(5)),
      _filter_median(std::make_unique<MedianFilter>(5)),
      _filter_zscore(std::make_unique<ModifiedZScoreFilter>(5, 3.5f)),
      _filter_hampel(std::make_unique<HampelFilter>(5, 3.0f)),
      _filter_complementary(std::make_unique<ComplementaryFilter>(0.7f)),
      _filter_alphabeta(std::make_unique<AlphaBetaFilter>(0.9f, 0.5f)),
      _filter_butterworth(std::make_unique<ButterworthLowPassFilter>(2.0f, 10.0f)),
      _filter_chebyshev(std::make_unique<ChebyshevLowPassFilter>(2.0f, 10.0f)),
      _filter_kalman(std::make_unique<KalmanLowPassFilter>(0.001f, 0.1f)) {}

void ScaleFilterPipeline::resetFilters() {
  _filter_ma->reset();
  _filter_ema->reset();
  _filter_wma->reset();
  _filter_median->reset();
  _filter_zscore->reset();
  _filter_hampel->reset();
  _filter_complementary->reset();
  _filter_alphabeta->reset();
  _filter_butterworth->reset();
  _filter_chebyshev->reset();
  _filter_kalman->reset();
  
  // Also reset slope tracking
  resetSlopes();
}

ScaleReadingResult ScaleFilterPipeline::update(float raw, uint64_t timestampMs) {
  ScaleReadingResult result;
  result.raw = raw;
  result.moving_average = _filter_ma->update(raw);
  result.ema = _filter_ema->update(raw);
  result.weighted_ma = _filter_wma->update(raw);
  result.median = _filter_median->update(raw);
  result.zscore = _filter_zscore->update(raw);
  result.hampel = _filter_hampel->update(raw);
  result.complementary = _filter_complementary->update(raw);
  result.alphabeta = _filter_alphabeta->update(raw);
  result.butterworth = _filter_butterworth->update(raw);
  result.chebyshev = _filter_chebyshev->update(raw);
  result.kalman = _filter_kalman->update(raw);

  // Calculate slopes for all filters
  calculateSlopes(result, timestampMs);

  _lastResult = result;
  _lastUpdateMs = timestampMs;
  return result;
}

void ScaleFilterPipeline::calculateSlopes(const ScaleReadingResult& result,
                                           uint64_t timestampMs) {
  // Array of filter values in same order as FilterType enum
  float filterValues[11] = {
    result.moving_average,
    result.ema,
    result.weighted_ma,
    result.median,
    result.zscore,
    result.hampel,
    result.complementary,
    result.alphabeta,
    result.butterworth,
    result.chebyshev,
    result.kalman
  };

  // Calculate slope for each filter
  for (int i = 0; i < 11; i++) {
    SlopeMetrics& metrics = _slopeMetrics[i];

    if (metrics.lastUpdateMs == 0) {
      // First reading - just initialize
      metrics.previousValue = filterValues[i];
      metrics.lastUpdateMs = timestampMs;
      metrics.slopeReadings = 1;
      return;
    }

    uint64_t timeDeltaMs = timestampMs - metrics.lastUpdateMs;
    if (timeDeltaMs > 0) {
      float valueDelta = filterValues[i] - metrics.previousValue;
      float slope = valueDelta / (timeDeltaMs / 1000.0f);  // kg/sec

      metrics.currentSlope = slope;
      metrics.accumulatedSlope += slope;
      metrics.slopeReadings++;
      metrics.previousValue = filterValues[i];
      metrics.lastUpdateMs = timestampMs;
    }
  }
}

float ScaleFilterPipeline::getAverageSlope(int filterIndex) const {
  if (filterIndex < 0 || filterIndex >= 11) {
    return 0.0f;
  }
  return _slopeMetrics[filterIndex].getAverageSlope();
}

float ScaleFilterPipeline::getCurrentSlope(int filterIndex) const {
  if (filterIndex < 0 || filterIndex >= 11) {
    return 0.0f;
  }
  return _slopeMetrics[filterIndex].currentSlope;
}

SlopeMetrics ScaleFilterPipeline::getSlopeMetrics(int filterIndex) const {
  if (filterIndex < 0 || filterIndex >= 11) {
    return SlopeMetrics();
  }
  return _slopeMetrics[filterIndex];
}

void ScaleFilterPipeline::resetSlopes() {
  for (int i = 0; i < 11; i++) {
    _slopeMetrics[i].reset();
  }
}

void ScaleFilterPipeline::resetSlope(int filterIndex) {
  if (filterIndex >= 0 && filterIndex < 11) {
    _slopeMetrics[filterIndex].reset();
  }
}

// EOF
