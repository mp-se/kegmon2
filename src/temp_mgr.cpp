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
#include <kegconfig.hpp>
#include <temp_ds.hpp>
#include <temp_mgr.hpp>

TempSensorManager::~TempSensorManager() {}

void TempSensorManager::setup() {
  Log.notice(F("SCAL: Initializing sensors on pins Data=%d." CR), PIN_DS);

  switch (myConfig.getTempSensorType()) {
    case SensorDS18B20:
      Log.info(F("TEMP: Initializing temp sensor DS18B20." CR));
      _sensor.reset(new TempSensorDS);
      break;

    default:
      Log.error(F("TEMP: Unable to find sensor type, exiting." CR));
      return;
  }

  if (_sensor) {
    _sensor->setup();
  } else {
    Log.error(F("TEMP: Unable to allocate sensor." CR));
    return;
  }

  read();
}

void TempSensorManager::read() {
  if (!_sensor) return;

  for (int i = 0; i < getSensorCount() && i < 5; i++)
    _lastTemperature[i] = _sensor->read(i);
}

// EOF
