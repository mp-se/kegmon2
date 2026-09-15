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
#ifndef SRC_FILTERS_FILTER_MEDIAN_HPP_
#define SRC_FILTERS_FILTER_MEDIAN_HPP_

#include <filters/filter_base.hpp>

#include <algorithm>
#include <deque>
#include <vector>

class MedianFilter : public ScaleFilter {
 private:
  size_t _window_size;
  std::deque<float> _buffer;
  float _variance = 0.0f;

 public:
  explicit MedianFilter(size_t window_size = 5) : _window_size(window_size) {}

  float update(float raw_value) override {
    _buffer.push_back(raw_value);
    if (_buffer.size() > _window_size) {
      _buffer.pop_front();
    }

    std::vector<float> sorted(_buffer.begin(), _buffer.end());
    std::sort(sorted.begin(), sorted.end());

    float median;
    if (sorted.size() % 2 == 0) {
      median = (sorted[sorted.size() / 2 - 1] + sorted[sorted.size() / 2]) / 2.0f;
    } else {
      median = sorted[sorted.size() / 2];
    }

    float var_sum = 0.0f;
    for (float val : sorted) {
      var_sum += (val - median) * (val - median);
    }
    _variance = var_sum / sorted.size();

    return median;
  }

  void reset() override {
    _buffer.clear();
    _variance = 0.0f;
  }

  std::string getName() const override { return "Median"; }
  float getVariance() const override { return _variance; }
};

#endif  // SRC_FILTERS_FILTER_MEDIAN_HPP_
// EOF
