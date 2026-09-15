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
#ifndef SRC_TEMP_MGR_HPP_
#define SRC_TEMP_MGR_HPP_
#include <memory>
#include <temp_base.hpp>
#include <utils.hpp>

class TempSensorManager {
 private:
  std::unique_ptr<TempSensorBase> _sensor;
  float _lastTemperature[5] = {NAN, NAN, NAN, NAN,
                               NAN};  // Support for scale bases + one extra

 public:
  TempSensorManager() {}
  ~TempSensorManager();
  TempSensorManager(const TempSensorManager&);
  TempSensorManager& operator=(const TempSensorManager&);

  void setup();
  void read();

  bool hasTemp(int index) const {
    if (index < 0 || index >= 5) return false;
    return !isnan(_lastTemperature[index]);
  }
  bool hasSensor() const { return _sensor.get()->hasSensor(); }
  int getSensorCount() const { return _sensor.get()->getSensorCount(); }

  String getSensorId(int index) const {
    if (index < 0 || index >= getSensorCount()) return "";
    return _sensor.get()->getSensorId(index);
  }

  float getLastTempC(int index) const {
    if (index < 0 || index > getSensorCount()) return NAN;
    return _lastTemperature[index];
  }

  float getLastTempByIdC(const char* sensorId) const {
    if (!sensorId) return NAN;
    for (int i = 0; i < getSensorCount(); i++) {
      String id = getSensorId(i);
      if (id == sensorId) {
        return _lastTemperature[i];
      }
    }
    return _lastTemperature[0];  // Default to first sensor if not found
  }
};

extern TempSensorManager myTemp;

#endif  // SRC_TEMP_MGR_HPP_

// EOF
