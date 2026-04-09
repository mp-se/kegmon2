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
#ifndef SRC_UI_KEGMON_HPP_
#define SRC_UI_KEGMON_HPP_

#if defined(ENABLE_LVGL)

#include <lvgl.h>
#include <stdbool.h>

#include <changedetection.hpp>
#include <ui_helpers.hpp>

#ifdef __cplusplus
extern "C" {
#endif

/* Layout IDs */
#define KEGMON_LAYOUT_4KEG 0
#define KEGMON_LAYOUT_2KEG 1

void kegmon_init(lv_disp_t* disp, bool darkmode, uint8_t layout_id);
void kegmon_loop(void);
void kegmon_cleanup(void);
void kegmon_set_beer_name(uint8_t index, const char* name);
void kegmon_set_beer_meta(uint8_t index, float abv, int ebc, int ibu);
void kegmon_set_keg_volume(uint8_t index, float keg_volume);
void kegmon_set_connected(uint8_t index, bool connected);
void kegmon_set_stable_weight(uint8_t index, float weight);
void kegmon_set_stable_volume(uint8_t index, float volume);
void kegmon_set_last_pour_volume(uint8_t index, float last_pour);
void kegmon_set_temperature(uint8_t index, float temp);
void kegmon_set_theme(bool darkmode);
void kegmon_set_volume_unit(const char* format);
void kegmon_set_temp_unit(const char* format);
void kegmon_set_event_type(uint8_t index, ChangeDetectionEventType event_type);
void kegmon_set_status_bar(const char* status);
void kegmon_set_layout(uint8_t layout_id);
uint8_t kegmon_get_layout(void);

#ifdef __cplusplus
}
#endif

#endif  // ENABLE_LVGL

#endif  // SRC_UI_KEGMON_HPP_

// EOF
