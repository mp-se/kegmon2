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
#include <barhelper.hpp>
#include <kegconfig.hpp>
#include <log.hpp>

void Barhelper::sendKegInformation(UnitIndex idx, float kegVol, float tempC) {
  //
  // API: https://europe-west1-barhelper-app.cloudfunctions.net/api/customKegMon
  // Descr: This method reports the keg volume.
  // Header: authorization with APIKEY
  // Payload:
  // {
  //   name: string, <name of monitor>
  //   volume: number <keg volume in liters>
  //   type: string, <unit of volume>
  // }
  if (strlen(myConfig.getBarhelperApiKey()) == 0) return;

  JsonDocument doc;
  String header = "Authorization: " + String(myConfig.getBarhelperApiKey());

  doc["name"] = myConfig.getBarhelperMonitor(idx);
  doc["volume"] = kegVol;
  doc["type"] = "l";
  Log.notice(F("BARH: Sending TAP information to barhelper "
               "keg %Fl [%d]" CR),
             kegVol, idx);

  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();
  // #if LOG_LEVEL == 6
  EspSerial.print(out.c_str());
  EspSerial.print(CR);
  // #endif

  constexpr auto BARHELPER_URL =
      "https://europe-west1-barhelper-app.cloudfunctions.net/api/customKegMon";
  Log.info(F("BARH: Using URL %s." CR), BARHELPER_URL);
  out = _push->sendHttpPost(out, BARHELPER_URL,
                            "Content-Type: application/json", header.c_str());
  updateStatus(out);
  Log.info(F("BARH: Response %s." CR), out.c_str());
}

void Barhelper::updateStatus(String& response) {
  _lastTimestamp = millis();
  _lastStatus = _push->wasLastSuccessful();
  _lastHttpError = _push->getLastResponseCode();
  _lastResponse = response;
  _hasRun = true;
}

// EOF
