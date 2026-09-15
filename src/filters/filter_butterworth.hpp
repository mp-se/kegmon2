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
#ifndef SRC_FILTERS_FILTER_BUTTERWORTH_HPP_
#define SRC_FILTERS_FILTER_BUTTERWORTH_HPP_

#include <filters/filter_base.hpp>

#include <cmath>

class ButterworthLowPassFilter : public ScaleFilter {
 private:
  float _cutoff_frequency;
  float _sampling_rate;
  float _alpha;
  float _last_value = 0.0f;
  bool _initialized = false;
  float _variance = 0.0f;

 public:
  ButterworthLowPassFilter(float cutoff_frequency = 2.0f,
                           float sampling_rate = 10.0f)
      : _cutoff_frequency(cutoff_frequency), _sampling_rate(sampling_rate) {
    float omega = 2.0f * 3.14159265359f * cutoff_frequency / sampling_rate;
    _alpha = omega / (omega + 1.0f);
  }

  float update(float raw_value) override {
    if (!_initialized) {
      _last_value = raw_value;
      _initialized = true;
      _variance = 0.0f;
      return raw_value;
    }

    float filtered = _alpha * raw_value + (1.0f - _alpha) * _last_value;
    _variance = _alpha * (raw_value - filtered) * (raw_value - filtered) +
                (1.0f - _alpha) * _variance;

    _last_value = filtered;
    return filtered;
  }

  void reset() override {
    _initialized = false;
    _last_value = 0.0f;
    _variance = 0.0f;
  }

  std::string getName() const override { return "Butterworth"; }
  float getVariance() const override { return _variance; }
};

#endif  // SRC_FILTERS_FILTER_BUTTERWORTH_HPP_
// EOF
