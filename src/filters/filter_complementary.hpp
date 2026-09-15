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
#ifndef SRC_FILTERS_FILTER_COMPLEMENTARY_HPP_
#define SRC_FILTERS_FILTER_COMPLEMENTARY_HPP_

#include <filters/filter_base.hpp>

class ComplementaryFilter : public ScaleFilter {
 private:
  float _alpha;
  float _last_filtered = 0.0f;
  float _last_raw = 0.0f;
  bool _initialized = false;
  float _variance = 0.0f;
  float _slope = 0.0f;

 public:
  explicit ComplementaryFilter(float alpha = 0.7f) : _alpha(alpha) {}

  float update(float raw_value) override {
    if (!_initialized) {
      _last_filtered = raw_value;
      _last_raw = raw_value;
      _initialized = true;
      _variance = 0.0f;
      _slope = 0.0f;
      return raw_value;
    }

    float filtered = _alpha * raw_value + (1.0f - _alpha) * _last_filtered;
    _slope = filtered - _last_filtered;
    _variance = _alpha * (raw_value - filtered) * (raw_value - filtered) +
                (1.0f - _alpha) * _variance;

    _last_filtered = filtered;
    _last_raw = raw_value;

    return filtered;
  }

  void reset() override {
    _initialized = false;
    _last_filtered = 0.0f;
    _last_raw = 0.0f;
    _variance = 0.0f;
    _slope = 0.0f;
  }

  std::string getName() const override { return "Complementary"; }
  float getVariance() const override { return _variance; }
  float getSlope() const override { return _slope; }
};

#endif  // SRC_FILTERS_FILTER_COMPLEMENTARY_HPP_
// EOF
