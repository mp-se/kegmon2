/*
 * KegMon
 * Copyright (c) 2026 Magnus
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 */
#include <brewgraph.hpp>

#include <ctime>

#include <kegconfig.hpp>
#include <log.hpp>

constexpr auto BREWGRAPH_API = "/ingest/kegmon";
constexpr auto BREWGRAPH_BEER_API = "/ingest/kegmon/beer";

namespace {
constexpr time_t MIN_VALID_EVENT_TIME = 1700000000;

uint32_t fallbackBootNonce() {
  static const uint32_t nonce = static_cast<uint32_t>(random(1, 0x7fffffffL));
  return nonce;
}
}  // namespace

String BrewGraph::getApiUrl(const char* path) const {
  String url = myConfig.getBrewGraphUrl();
  while (url.endsWith("/")) url.remove(url.length() - 1);
  return url + path;
}

void BrewGraph::sendPourInformation(UnitIndex idx, float pourVol,
                                    float kegVol, uint32_t eventTimeMs,
                                    float temperature) {
  const uint8_t kegIdx = static_cast<uint8_t>(idx);
  if (strlen(myConfig.getBrewGraphUrl()) == 0 ||
      strlen(myConfig.getBrewGraphToken(idx)) == 0)
    return;

  JsonDocument doc;
  doc["token"] = myConfig.getBrewGraphToken(idx);
  if (!isnan(pourVol)) doc["pour"] = pourVol;
  if (!isnan(kegVol)) doc["volume"] = kegVol;
  const float maxVolume = myConfig.getKegVolume(idx);
  if (!isnan(maxVolume)) doc["maxVolume"] = maxVolume;
  if (!isnan(temperature)) doc["temperature"] = temperature;

  char eventId[48];
  const time_t now = time(nullptr);
  if (now >= MIN_VALID_EVENT_TIME) {
    snprintf(eventId, sizeof(eventId), "keg-%u-%llu-%lu", kegIdx + 1,
             static_cast<unsigned long long>(now),
             static_cast<unsigned long>(eventTimeMs));
  } else {
    snprintf(eventId, sizeof(eventId), "keg-%u-%lu-%lu", kegIdx + 1,
             static_cast<unsigned long>(fallbackBootNonce()),
             static_cast<unsigned long>(eventTimeMs));
  }
  doc["eventId"] = eventId;

  String payload;
  payload.reserve(120);
  serializeJson(doc, payload);

  Log.notice(F("BGRF: Sending pour information for keg %d." CR), idx + 1);
  const String response = _push->sendHttpPost(
      payload, getApiUrl(BREWGRAPH_API).c_str(),
      "Content-Type: application/json", "");
  updateStatus(response);
}

void BrewGraph::sendKegInformation(UnitIndex idx, float kegVol,
                                   float temperature) {
  const uint8_t kegIdx = static_cast<uint8_t>(idx);
  if (strlen(myConfig.getBrewGraphUrl()) == 0 ||
      strlen(myConfig.getBrewGraphToken(idx)) == 0)
    return;

  JsonDocument doc;
  doc["token"] = myConfig.getBrewGraphToken(idx);
  if (!isnan(kegVol)) doc["volume"] = kegVol;
  const float maxVolume = myConfig.getKegVolume(idx);
  if (!isnan(maxVolume)) doc["maxVolume"] = maxVolume;
  if (!isnan(temperature)) doc["temperature"] = temperature;

  String payload;
  payload.reserve(100);
  serializeJson(doc, payload);

  Log.notice(F("BGRF: Sending level information for keg %d." CR), idx + 1);
  const String response = _push->sendHttpPost(
      payload, getApiUrl(BREWGRAPH_API).c_str(),
      "Content-Type: application/json", "");
  updateStatus(response);
}

bool BrewGraph::getBeerInformation(JsonObject& obj, UnitIndex idx) {
  const uint8_t kegIdx = static_cast<uint8_t>(idx);
  if (strlen(myConfig.getBrewGraphUrl()) == 0 ||
      strlen(myConfig.getBrewGraphToken(idx)) == 0)
    return false;

  JsonDocument request;
  request["token"] = myConfig.getBrewGraphToken(idx);

  String payload;
  serializeJson(request, payload);
  const String response = _push->sendHttpPost(
      payload, getApiUrl(BREWGRAPH_BEER_API).c_str(),
      "Content-Type: application/json", "");
  if (!_push->wasLastSuccessful()) return false;

  JsonDocument beer;
  if (deserializeJson(beer, response) || beer["name"].isNull()) return false;

  obj["name"] = beer["name"];
  obj["abv"] = beer["abv"];
  obj["ibu"] = beer["ibu"];
  obj["ebc"] = beer["ebc"];
  return true;
}

void BrewGraph::updateStatus(const String& response) {
  _lastTimestamp = millis();
  _lastStatus = _push->wasLastSuccessful();
  _lastHttpError = _push->getLastResponseCode();
  _lastResponse = response.substring(0, 160);
  _hasRun = true;
}
