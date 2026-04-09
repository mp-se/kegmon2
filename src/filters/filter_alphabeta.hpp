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
#ifndef SRC_FILTERS_FILTER_ALPHABETA_HPP_
#define SRC_FILTERS_FILTER_ALPHABETA_HPP_

#include <filters/filter_base.hpp>

class AlphaBetaFilter : public ScaleFilter {
 private:
  float _alpha;
  float _beta;
  float _x = 0.0f;
  float _v = 0.0f;
  bool _initialized = false;
  float _variance = 0.0f;
  float _slope = 0.0f;

 public:
  explicit AlphaBetaFilter(float alpha = 0.9f, float beta = 0.5f)
      : _alpha(alpha), _beta(beta) {}

  float update(float raw_value) override {
    if (!_initialized) {
      _x = raw_value;
      _v = 0.0f;
      _initialized = true;
      _variance = 0.0f;
      _slope = 0.0f;
      return raw_value;
    }

    float x_pred = _x + _v;
    float v_pred = _v;
    float residual = raw_value - x_pred;

    _x = x_pred + _alpha * residual;
    _v = v_pred + (_beta / 1.0f) * residual;
    _slope = _v;
    _variance = _alpha * residual * residual + (1.0f - _alpha) * _variance;

    return _x;
  }

  void reset() override {
    _initialized = false;
    _x = 0.0f;
    _v = 0.0f;
    _variance = 0.0f;
    _slope = 0.0f;
  }

  std::string getName() const override { return "AlphaBeta"; }
  float getVariance() const override { return _variance; }
  float getSlope() const override { return _slope; }
};

#endif  // SRC_FILTERS_FILTER_ALPHABETA_HPP_
// EOF
