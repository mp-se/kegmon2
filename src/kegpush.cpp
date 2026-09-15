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
#include <kegpush.hpp>
#include <log.hpp>
#include <scale.hpp>
#include <utils.hpp>

void KegPushHandler::pushPourInformation(UnitIndex idx, float stableVol,
                                         float pourVol, float tempC,
                                         bool isLoop) {
  if (!isLoop)  // Limit calls to brewspy
    _brewspy->sendPourInformation(idx, pourVol, tempC);

  _ha->sendPourInformation(idx, pourVol, tempC);
  _brewLogger->sendPourInformation(idx, pourVol, stableVol, tempC);
}

void KegPushHandler::pushKegInformation(UnitIndex idx, float stableVol,
                                        float pourVol, float glasses,
                                        float tempC, bool isLoop) {
  if (!isLoop)  // Limit calls to brewspy
    _brewspy->sendTapInformation(idx, stableVol, pourVol, tempC);

  _ha->sendTapInformation(idx, stableVol, glasses, tempC);
  _barhelper->sendKegInformation(idx, stableVol, tempC);
  _brewLogger->sendKegInformation(idx, stableVol, tempC);
}

// EOF
