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
#ifndef SRC_KEGWEBHANDLER_HPP_
#define SRC_KEGWEBHANDLER_HPP_
#include <FS.h>
#include <LittleFS.h>
#include <WiFi.h>

#include <atomic>
#include <basewebserver.hpp>
#include <changedetection.hpp>
#include <kegconfig.hpp>

class KegWebHandler : public BaseWebServer {
 protected:
  KegConfig *_config;
  volatile bool _hardwareScanTask = false;

  String _hardwareScanData;

  // Ringbuffer for recent events (last 10)
  static constexpr size_t RECENT_EVENTS_SIZE = 20;
  ChangeDetectionEvent _recentEvents[RECENT_EVENTS_SIZE];
  std::atomic<size_t> _eventHead = 0;   // Write position
  std::atomic<size_t> _eventCount = 0;  // Number of events stored
  mutable portMUX_TYPE _eventLock =
      portMUX_INITIALIZER_UNLOCKED;  // Thread safety

  void setupWebHandlers();

  void webFeature(AsyncWebServerRequest *request);
  void webScale(AsyncWebServerRequest *request);
  void webScaleTare(AsyncWebServerRequest *request, JsonVariant &json);
  void webScaleFactor(AsyncWebServerRequest *request, JsonVariant &json);
  void webHardwareScan(AsyncWebServerRequest *request);
  void webHardwareScanStatus(AsyncWebServerRequest *request);
  void webConfigGet(AsyncWebServerRequest *request);
  void webConfigPost(AsyncWebServerRequest *request, JsonVariant &json);
  void webStatus(AsyncWebServerRequest *request);
  void webStatistic(AsyncWebServerRequest *request);
  void webStatisticClear(AsyncWebServerRequest *request);
  void webHandleBrewspy(AsyncWebServerRequest *request, JsonVariant &json);
  void webHandleFactoryDefaults(AsyncWebServerRequest *request);
  void webHandleSecureDigital(AsyncWebServerRequest *request,
                              JsonVariant &json);

 public:
  explicit KegWebHandler(KegConfig *config);

  void loop();

  // Queue an event for publishing in status endpoint
  void queueEvent(const ChangeDetectionEvent &event);

  // Get recent events (called by status endpoint)
  void getRecentEvents(ChangeDetectionEvent *outEvents, size_t &count);
};

#endif  // SRC_KEGWEBHANDLER_HPP_

// EOF
