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
#ifndef SRC_TEMP_DS_HPP_
#define SRC_TEMP_DS_HPP_
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>

#include <temp_base.hpp>

class TempSensorDS : public TempSensorBase {
 private:
  OneWire* _oneWire = 0;
  DallasTemperature* _dallas = 0;
  bool _hasSensor = false;
  String _sensorIds[MAX_SCALES + 1] = {};

 public:
  TempSensorDS() {}
  ~TempSensorDS();

  void setup() override;
  float read(int index) override;
  bool hasSensor() const override { return _hasSensor; }
  int getSensorCount() const override;
  String getSensorId(int index) const override;
};

#endif  // SRC_TEMP_DS_HPP_

// EOF
