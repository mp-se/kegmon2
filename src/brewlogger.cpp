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
#include <brewlogger.hpp>
#include <kegconfig.hpp>
#include <log.hpp>

constexpr auto BREWLOGGER_API = "/api/pour/public";

void BrewLogger::sendPourInformation(UnitIndex idx, float pourVol,
                                     float kegVol, float tempC) {
  if (strlen(myConfig.getBrewLoggerUrl()) == 0) return;

  JsonDocument doc;

  doc["id"] = myConfig.getBeerId(idx);
  doc["pour"] = pourVol;
  doc["volume"] = kegVol;
  doc["maxVolume"] = myConfig.getKegVolume(idx);

  Log.notice(F("BLOG: Sending pour information to brewlogger "
               "pour %Fl [%d]" CR),
             pourVol, idx);

  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();
  // #if LOG_LEVEL == 6
  EspSerial.print(out.c_str());
  EspSerial.print(CR);
  // #endif

  String url = myConfig.getBrewLoggerUrl() + String(BREWLOGGER_API);

  out = _push->sendHttpPost(out, url.c_str(), "Content-Type: application/json",
                            "");
  updateStatus(out);
  Log.info(F("BLOG: Response %s." CR), out.c_str());
}

void BrewLogger::sendKegInformation(UnitIndex idx, float kegVol, float tempC) {
  if (strlen(myConfig.getBrewLoggerUrl()) == 0) return;

  JsonDocument doc;

  doc["id"] = myConfig.getBeerId(idx);
  doc["volume"] = kegVol;
  doc["maxVolume"] = myConfig.getKegVolume(idx);

  Log.notice(F("BLOG: Sending level information to brewlogger "
               "kegVol %Fl [%d]" CR),
             kegVol, idx);

  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();
  // #if LOG_LEVEL == 6
  EspSerial.print(out.c_str());
  EspSerial.print(CR);
  // #endif

  String url = myConfig.getBrewLoggerUrl() + String(BREWLOGGER_API);

  out = _push->sendHttpPost(out, url.c_str(), "Content-Type: application/json",
                            "");
  updateStatus(out);
  Log.info(F("BLOG: Response %s." CR), out.c_str());
}

void BrewLogger::updateStatus(String& response) {
  _lastTimestamp = millis();
  _lastStatus = _push->wasLastSuccessful();
  _lastHttpError = _push->getLastResponseCode();
  _lastResponse = response;
  _hasRun = true;
}

// EOF
