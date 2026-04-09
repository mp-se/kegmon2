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
#ifndef SRC_MAIN_HPP_
#define SRC_MAIN_HPP_

#include <LittleFS.h>
#include <stdlib.h>

#include <log.hpp>

constexpr auto CFG_MDNSNAME = "Kegmon";
constexpr auto CFG_FILENAME = "/kegmon2.json";
constexpr int MAX_SCALES = 4;

enum RunMode {
  normalMode = 0,
  wifiSetupMode = 2,
};
extern RunMode runMode;

enum UnitIndex { U1 = 0, U2 = 1, U3 = 2, U4 = 3 };

#if defined(LOLIN_S3_PRO)
// Hardware config for Lolin S3 PRO
// --------------------------------
constexpr auto PIN_LED = BUILTIN_LED;
#define PIN_SCALE_SDA1 41
#define PIN_SCALE_SCK1 42
#define PIN_SCALE_SDA2 39
#define PIN_SCALE_SCK2 40
#define PIN_SCALE_SDA3 18
#define PIN_SCALE_SCK3 38
#define PIN_SCALE_SDA4 16
#define PIN_SCALE_SCK4 17
#define PIN_DS 1
#endif

#endif  // SRC_MAIN_HPP_
