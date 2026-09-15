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
#ifndef SRC_FILTERS_FILTER_MOVINGAVERAGE_HPP_
#define SRC_FILTERS_FILTER_MOVINGAVERAGE_HPP_

#include <filters/filter_base.hpp>

#include <deque>

class MovingAverageFilter : public ScaleFilter {
 private:
  size_t _window_size;
  std::deque<float> _buffer;
  float _variance = 0.0f;

 public:
  explicit MovingAverageFilter(size_t window_size = 5) : _window_size(window_size) {}

  float update(float raw_value) override {
    _buffer.push_back(raw_value);
    if (_buffer.size() > _window_size) {
      _buffer.pop_front();
    }

    float sum = 0.0f;
    for (float val : _buffer) {
      sum += val;
    }
    float average = sum / _buffer.size();

    float var_sum = 0.0f;
    for (float val : _buffer) {
      var_sum += (val - average) * (val - average);
    }
    _variance = var_sum / _buffer.size();

    return average;
  }

  void reset() override {
    _buffer.clear();
    _variance = 0.0f;
  }

  std::string getName() const override { return "MovingAverage"; }
  float getVariance() const override { return _variance; }
};

#endif  // SRC_FILTERS_FILTER_MOVINGAVERAGE_HPP_
// EOF
