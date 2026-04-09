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
#include <main.hpp>
#include <temp_ds.hpp>
#include <utils.hpp>

TempSensorDS::~TempSensorDS() {
  if (_oneWire) delete _oneWire;
  if (_dallas) delete _dallas;
}

void TempSensorDS::setup() {
  _oneWire = new OneWire(PIN_DS);
  _dallas = new DallasTemperature(_oneWire);
  _dallas->setResolution(12);
  _dallas->begin();

  if (_dallas->getDS18Count())
    _hasSensor = true;
  else
    _hasSensor = false;

  // Cache sensor IDs during setup to avoid repeated OneWire communication
  int cnt = _dallas->getDS18Count();
  for (int i = 0; i < cnt && i <= MAX_SCALES; i++) {
    DeviceAddress adr;
    _dallas->getAddress(&adr[0], i);
    _sensorIds[i] = String(adr[0], 16) + String(adr[1], 16) +
                    String(adr[2], 16) + String(adr[3], 16) +
                    String(adr[4], 16) + String(adr[5], 16) +
                    String(adr[6], 16) + String(adr[7], 16);
  }
}

float TempSensorDS::read(int index) {
  float temp = NAN;

  if (!_dallas) return temp;

  int cnt = _dallas->getDS18Count();

  if (cnt > index) {
    _dallas->requestTemperatures();
    temp = _dallas->getTempCByIndex(index);
  } else {
    Log.error(F("TEMP: No DS18B20 sensor found at index %d." CR), index);
  }

  return temp;
}

int TempSensorDS::getSensorCount() const {
  if (_dallas)
    return _dallas->getDS18Count();
  else
    return 0;
}

String TempSensorDS::getSensorId(int index) const {
  if (index < 0 || index > MAX_SCALES || index >= getSensorCount()) return "";
  return _sensorIds[index];
}

// EOF
