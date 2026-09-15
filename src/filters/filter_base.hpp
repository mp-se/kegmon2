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
#ifndef SRC_FILTERS_FILTER_BASE_HPP_
#define SRC_FILTERS_FILTER_BASE_HPP_

#include <string>

// Enum for all available filter types
enum class FilterType {
  FILTER_RAW = 0,
  FILTER_MOVING_AVERAGE,
  FILTER_EMA,
  FILTER_WEIGHTED_MA,
  FILTER_MEDIAN,
  FILTER_ZSCORE,
  FILTER_HAMPEL,
  FILTER_COMPLEMENTARY,
  FILTER_ALPHABETA,
  FILTER_BUTTERWORTH,
  FILTER_FIR,
  FILTER_CHEBYSHEV,
  FILTER_KALMAN
};

class ScaleFilter {
 public:
  virtual ~ScaleFilter() = default;

  // Process a new raw value and return filtered result
  virtual float update(float raw_value) = 0;

  // Reset filter to initial state
  virtual void reset() = 0;

  // Get human-readable filter name
  virtual std::string getName() const = 0;

  // Get internal variance (for diagnostics)
  virtual float getVariance() const { return 0.0f; }

  // Get slope or rate of change (for diagnostics)
  virtual float getSlope() const { return 0.0f; }
};

#endif  // SRC_FILTERS_FILTER_BASE_HPP_
// EOF
