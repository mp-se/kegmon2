/*
MIT License

Copyright (c) 2026 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
