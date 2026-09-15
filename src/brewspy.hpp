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
#ifndef SRC_BREWSPY_HPP_
#define SRC_BREWSPY_HPP_

#include <basepush.hpp>
#include <main.hpp>

class Brewspy {
 protected:
  BasePush *_push;

  bool _hasRun = false;
  uint32_t _lastTimestamp = 0;
  bool _lastStatus = false;
  int _lastHttpError = 0;
  String _lastResponse;

  void updateStatus(String &response);

 public:
  explicit Brewspy(BasePush *push) { _push = push; }

  void sendTapInformation(UnitIndex idx, float stableVol, float pourVol,
                          float tempC);
  void sendPourInformation(UnitIndex idx, float pourVol, float tempC);
  void clearKegInformation(UnitIndex idx);
  void getTapInformation(JsonObject &obj, const String token);

  bool hasRun() { return _hasRun; }
  uint32_t getLastTimeStamp() { return _lastTimestamp; }
  bool getLastStatus() { return _lastStatus; }
  int getLastError() { return _lastHttpError; }
  String getLastResponse() { return _lastResponse; }
};

#endif  // SRC_BREWSPY_HPP_

// EOF
