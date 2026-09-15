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
#include <algorithm>
#include <memory>
#include <temp_base.hpp>
#include <utils.hpp>

class TempSensorManager {
 private:
  static constexpr int MAX_TEMPERATURE_SENSORS = MAX_SCALES + 1;
  std::unique_ptr<TempSensorBase> _sensor;
  float _lastTemperature[MAX_TEMPERATURE_SENSORS] = {
      NAN, NAN, NAN, NAN, NAN};  // Scale bases plus one extra

 public:
  TempSensorManager() {}
  ~TempSensorManager();
  TempSensorManager(const TempSensorManager&);
  TempSensorManager& operator=(const TempSensorManager&);

  void setup();
  void read();

  bool hasTemp(int index) const {
    if (index < 0 || index >= MAX_TEMPERATURE_SENSORS) return false;
    return !isnan(_lastTemperature[index]);
  }
  bool hasSensor() const { return _sensor && _sensor->hasSensor(); }
  int getSensorCount() const {
    if (!_sensor) return 0;
    return std::min(_sensor->getSensorCount(), MAX_TEMPERATURE_SENSORS);
  }

  String getSensorId(int index) const {
    if (index < 0 || index >= getSensorCount()) return "";
    return _sensor.get()->getSensorId(index);
  }

  float getLastTempC(int index) const {
    if (index < 0 || index >= getSensorCount() ||
        index >= MAX_TEMPERATURE_SENSORS)
      return NAN;
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
