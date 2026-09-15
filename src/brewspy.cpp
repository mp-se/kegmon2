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
#include <brewspy.hpp>
#include <kegconfig.hpp>
#include <log.hpp>
#include <scale.hpp>
#include <utils.hpp>

void Brewspy::sendTapInformation(UnitIndex idx, float stableVol,
                                 float pourVol, float tempC) {
  //
  // API: https://brew-spy.com/api/tap/keg/set
  // Descr: This method sets all the keg related fields.
  // Payload:
  // {
  //   token: string, *
  //   unit: string, **
  //   kegVolume: number, ***
  //   beerLeft: number,
  //   pour: number
  // }
  // * To get the token for a tap, click on it and look at the Webhook URL. The
  // token is the alphanumeric string at the end of the url (right after
  // /taplist/)
  // ** can be: l, ml, or pint. Let me know if support for imperial units is
  // needed.
  // *** Initial volume of the keg - used to calculate percentage left
  if (strlen(myConfig.getBrewspyToken(idx)) == 0) return;

  JsonDocument doc;

  doc["token"] = myConfig.getBrewspyToken(idx);
  doc["unit"] = "l";
  doc["kegVolume"] = myConfig.getKegVolume(idx);
  doc["beerLeft"] = stableVol;
  doc["pour"] = pourVol;
  Log.notice(F("BSPY: Sending TAP information to brewspy, last %Fl, "
               "pour %Fl  [%d]" CR),
             stableVol, pourVol, idx);
  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();
  // #if LOG_LEVEL == 6
  EspSerial.print(out.c_str());
  EspSerial.print(CR);
  // #endif
  out =
      _push->sendHttpPost(out, "https://brew-spy.com/api/tap/keg/set", "", "");
  updateStatus(out);

  Log.info(F("BSPY: Response %s." CR), out.c_str());
}

void Brewspy::sendPourInformation(UnitIndex idx, float pourVol, float tempC) {
  // API: https://brew-spy.com/api/tap/keg/pour
  // Descr: Sets the last pour field and subtracts the volume from the beer left
  // field Payload:
  // {
  //   token: string,
  //   unit: string,
  //   pour: number
  // }
  if (strlen(myConfig.getBrewspyToken(idx)) == 0) return;

  JsonDocument doc;

  doc["token"] = myConfig.getBrewspyToken(idx);
  doc["unit"] = "l";
  doc["pour"] = pourVol;
  Log.notice(F("BSPY: Sending POUR information to brewspy, pour %Fl [%d]." CR),
             pourVol, idx);
  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();
  // #if LOG_LEVEL == 6
  EspSerial.print(out.c_str());
  EspSerial.print(CR);
  // #endif
  out =
      _push->sendHttpPost(out, "https://brew-spy.com/api/tap/keg/pour", "", "");
  updateStatus(out);

  Log.info(F("BSPY: Response %s." CR), out.c_str());
}

void Brewspy::clearKegInformation(UnitIndex idx) {
  // API: https://brew-spy.com/api/tap/keg/clear
  // Descr: Clears all fields for the tap
  // Payload:
  // {
  //   token: string
  // }
  if (strlen(myConfig.getBrewspyToken(idx)) == 0) return;

  JsonDocument doc;

  doc["token"] = myConfig.getBrewspyToken(idx);
  Log.notice(F("BSPY: Sending CLEAR to brewspy [%d]." CR), idx);

  String out;
  out.reserve(30);
  serializeJson(doc, out);
  doc.clear();
  // #if LOG_LEVEL == 6
  EspSerial.print(out.c_str());
  EspSerial.print(CR);
  // #endif
  out = _push->sendHttpPost(out, "https://brew-spy.com/api/tap/keg/clear", "",
                            "");
  updateStatus(out);

  Log.info(F("BSPY: Response %s." CR), out.c_str());
}

void Brewspy::getTapInformation(JsonObject& obj, const String token) {
  // API: https://brew-spy.com/api/json/taplist/<TOKEN>
  // Descr: To view the current data for a tap you can use the
  // Payload:
  // {
  //   "tap" : <tap number>,
  //   "recipe" : <beer name>,
  //   "abv" : <abv>,
  //   "style" : <beer style>,
  //   "date" : <date added on tap>,
  //   "kegLevel" : {
  //     "pcnt" : <perecentage beer left>,
  //     "pour" : <last pour volume>,
  //     "pourUnit" : <last pour volume unit>,
  //     "vol" : <volume of beer left>,
  //     "volUnit" : <volume of beer left unit>,
  //   }
  // }
  if (token.length() == 0) return;

  Log.notice(F("BSPY: Requesting TAP information from brewspy." CR));

  String url = "https://brew-spy.com/api/json/taplist/" + token;
  String payload = "";
  String resp =
      _push->sendHttpGet(payload, url.c_str(), "Accept: application/json", "");
  EspSerial.print(resp.c_str());
  EspSerial.print(CR);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, resp);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  obj["recipe"] = doc["recipe"].as<String>();
  obj["abv"] = doc["abv"].as<float>();
}

void Brewspy::updateStatus(String& response) {
  _lastTimestamp = millis();
  _lastStatus = _push->wasLastSuccessful();
  _lastHttpError = _push->getLastResponseCode();
  _lastResponse = response;
  _hasRun = true;
}

// EOF
