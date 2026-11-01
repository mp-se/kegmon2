/*
 * KegMon
 * Copyright (c) 2026 Magnus
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
 */
#ifndef SRC_BREWGRAPH_HPP_
#define SRC_BREWGRAPH_HPP_

#include <basepush.hpp>
#include <main.hpp>

class BrewGraph {
 protected:
  BasePush* _push;
  bool _hasRun = false;
  uint32_t _lastTimestamp = 0;
  bool _lastStatus = false;
  int _lastHttpError = 0;
  String _lastResponse;

  String getApiUrl(const char* path) const;
  void updateStatus(const String& response);

 public:
  explicit BrewGraph(BasePush* push) : _push(push) {}

  void sendPourInformation(UnitIndex idx, float pourVol, float kegVol,
                           uint32_t eventTimeMs, float temperature = NAN);
  void sendKegInformation(UnitIndex idx, float kegVol,
                          float temperature = NAN);
  bool getBeerInformation(JsonObject& obj, UnitIndex idx);

  bool hasRun() const { return _hasRun; }
  uint32_t getLastTimeStamp() const { return _lastTimestamp; }
  bool getLastStatus() const { return _lastStatus; }
  int getLastError() const { return _lastHttpError; }
  String getLastResponse() const { return _lastResponse; }
};

#endif  // SRC_BREWGRAPH_HPP_
