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
#ifndef SRC_TEMP_BASE_HPP_
#define SRC_TEMP_BASE_HPP_

#include <Arduino.h>

class TempSensorBase {
 public:
  TempSensorBase() = default;
  virtual ~TempSensorBase() {}

  virtual void setup() = 0;
  virtual float read(int index) = 0;
  virtual bool hasSensor() const = 0;
  virtual int getSensorCount() const = 0;
  virtual String getSensorId(int index) const = 0;
};

#endif  // SRC_TEMP_BASE_HPP_

// EOF
