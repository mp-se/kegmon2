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
#ifndef SRC_KEGPUSH_HPP_
#define SRC_KEGPUSH_HPP_

#include <barhelper.hpp>
#include <basepush.hpp>
#include <brewlogger.hpp>
#include <brewspy.hpp>
#include <homeassist.hpp>
#include <kegconfig.hpp>

class KegPushHandler : public BasePush {
 private:
  Brewspy* _brewspy = NULL;
  HomeAssist* _ha = NULL;
  Barhelper* _barhelper = NULL;
  BrewLogger* _brewLogger = NULL;

 public:
  explicit KegPushHandler(KegConfig* config) : BasePush(config) {
    _brewspy = new Brewspy(this);
    _ha = new HomeAssist(this);
    _barhelper = new Barhelper(this);
    _brewLogger = new BrewLogger(this);
  }

  void requestTapInfoFromBrewspy(JsonObject& obj, String token) {
    _brewspy->getTapInformation(obj, token);
  }

  void pushPourInformation(UnitIndex idx, float stableVol, float pourVol,
                           float tempC, bool isLoop = false);
  void pushKegInformation(UnitIndex idx, float stableVol, float pourVol,
                          float glasses, float tempC, bool isLoop = false);

  Brewspy* getBrewspy() { return _brewspy; }
  HomeAssist* getHomeAssist() { return _ha; }
  Barhelper* getBarHelper() { return _barhelper; }
  BrewLogger* getBrewLogger() { return _brewLogger; }
};

extern KegPushHandler myPush;

#endif  // SRC_KEGPUSH_HPP_

// EOF
