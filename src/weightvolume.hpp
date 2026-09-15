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
#ifndef SRC_WEIGHTVOLUME_HPP_
#define SRC_WEIGHTVOLUME_HPP_

#include <kegconfig.hpp>

// Note! Internally we assume that everything are in Metric formats, weights=Kg
// and all volumes=Liters.
class WeightVolumeConverter {
 private:
  float _fg;
  UnitIndex _idx;

 public:
  explicit WeightVolumeConverter(UnitIndex idx) {
    _idx = idx;
    _fg = myConfig.getBeerFG(idx);
    if (_fg < 1) _fg = 1;
  }

  float weightToVolume(float kg) {
    float liter = isnan(kg) || kg == 0 ? 0 : kg / _fg;
    return liter;
  }

  float weightToGlasses(float kg) {
    float glassVol = myConfig.getGlassVolume(_idx);
    float glassWeight = glassVol * _fg;
    float glass = kg / glassWeight;
    return glass < 0 ? 0 : glass;
  }
};

#endif  // SRC_WEIGHTVOLUME_HPP_

// EOF
