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
#ifndef SRC_FILTERS_FILTER_FIR_HPP_
#define SRC_FILTERS_FILTER_FIR_HPP_

#include <filters/filter_base.hpp>

#include <deque>
#include <vector>

static const std::vector<float> fir_coefficients_order5 = {
    0.1f, 0.2f, 0.4f, 0.2f, 0.1f};
static const std::vector<float> fir_coefficients_order7 = {
    0.05f, 0.1f, 0.2f, 0.3f, 0.2f, 0.1f, 0.05f};

class FIRLowPassFilter : public ScaleFilter {
 private:
  std::vector<float> _coefficients;
  std::deque<float> _buffer;
  float _variance = 0.0f;

 public:
  explicit FIRLowPassFilter(size_t order = 5) {
    if (order <= 5) {
      _coefficients = fir_coefficients_order5;
    } else {
      _coefficients = fir_coefficients_order7;
    }
  }

  float update(float raw_value) override {
    _buffer.push_back(raw_value);
    if (_buffer.size() > _coefficients.size()) {
      _buffer.pop_front();
    }

    float filtered = 0.0f;
    for (size_t i = 0; i < _buffer.size(); ++i) {
      filtered += _buffer[i] * _coefficients[i];
    }

    float mean = 0.0f;
    for (float val : _buffer) {
      mean += val;
    }
    mean /= _buffer.size();

    float var_sum = 0.0f;
    for (float val : _buffer) {
      var_sum += (val - mean) * (val - mean);
    }
    _variance = var_sum / _buffer.size();

    return filtered;
  }

  void reset() override {
    _buffer.clear();
    _variance = 0.0f;
  }

  std::string getName() const override { return "FIR"; }
  float getVariance() const override { return _variance; }
};

#endif  // SRC_FILTERS_FILTER_FIR_HPP_
// EOF
