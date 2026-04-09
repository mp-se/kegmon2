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
#ifndef SRC_FILTERS_FILTER_EMA_HPP_
#define SRC_FILTERS_FILTER_EMA_HPP_

#include <filters/filter_base.hpp>

class ExponentialMovingAverageFilter : public ScaleFilter {
 private:
  float _alpha;
  float _last_value = 0.0f;
  bool _initialized = false;
  float _variance = 0.0f;

 public:
  explicit ExponentialMovingAverageFilter(float alpha = 0.3f) : _alpha(alpha) {}

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

  std::string getName() const override { return "EMA"; }
  float getVariance() const override { return _variance; }
};

#endif  // SRC_FILTERS_FILTER_EMA_HPP_
// EOF
