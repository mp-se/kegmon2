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
#if defined(ENABLE_LVGL)

#include <changedetection.hpp>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ui_helpers.hpp>
#include <ui_kegmon.hpp>

constexpr uint8_t KEGMON_MAX_KEGS = 4;

LV_FONT_DECLARE(lv_font_kegmon_10);       // Ensure custom font is declared
#define LV_SYMBOL_PALETTE "\xEF\x94\xBF"  // Unicode for palette icon (0xf53f)
#define LV_SYMBOL_LEAF "\xEF\x81\xAC"     // Unicode for leaf icon (0xf06c)
#define LV_SYMBOL_BEER "\xEF\x83\xBC"     // Unicode for beer icon (0xf0fc)

/* Use https://lvgl.io/tools/fontconverter for creating custom font with symbols
 *     https://www.cogsci.ed.ac.uk/~richard/utf-8.cgi?input=f06c&mode=hex
 *     https://fontawesome.com/search?q=FOOD&ic=free-collection
 *
 * Name: lv_font_kegmon_10
 * Size: 10 px
 * Range: 0x20-0x7E,0xF53F,0xF06C,0xF0FC
 */

/**
 * Layout manager
 */
typedef struct {
  uint8_t current_layout;
  uint8_t total_layouts;
} kegmon_layout_mgr_t;

static kegmon_layout_mgr_t layout_mgr = {
    .current_layout = KEGMON_LAYOUT_4KEG,
    .total_layouts = 2,
};

// Data structure for each keg
typedef struct {
  // UI objects for this keg
  lv_obj_t* container;
  lv_obj_t* lbl_name;
  lv_obj_t* lbl_volume;
  lv_obj_t* lbl_state;
  lv_obj_t* lbl_pours;
  lv_obj_t* lbl_abv;
  lv_obj_t* lbl_ibu;
  lv_obj_t* lbl_ebc;
  lv_obj_t* bar_fill;

  bool flg_created;

  // Data for this keg
  bool data_connected;

  char data_beer_name[32];

  int data_beer_ebc;
  int data_beer_ibu;

  float data_beer_abv;
  float data_temp;
  float data_stable_weight;
  float data_stable_volume;
  float data_keg_volume;
  float data_last_pour_volume;
  ChangeDetectionEventType data_event_type;
} kegmon_scale_t;

// Data structure for the UI
typedef struct {
  lv_obj_t* obj_status_bar;
  lv_obj_t* lbl_status_label;
  char data_volume_unit[10];
  char data_temp_unit[10];
  bool data_darkmode;
  char data_status[64];
  kegmon_scale_t scale[KEGMON_MAX_KEGS];
} kegmon_state_t;

static kegmon_state_t g_scales = {0};
static lv_disp_t* obj_disp = NULL;

static void create_keg_ui(uint8_t idx, lv_obj_t* parent, int32_t x, int32_t y,
                          int32_t w, int32_t h) {
  kegmon_scale_t* k = &g_scales.scale[idx];
  k->container = lv_obj_create(parent);
  lv_obj_set_pos(k->container, x, y);
  lv_obj_set_size(k->container, w, h);
  ui_theme_colors_t theme = ui_get_theme_colors(
      g_scales.data_darkmode ? UI_THEME_DARK : UI_THEME_LIGHT);
  lv_obj_set_style_bg_color(k->container, theme.panel_bg, LV_PART_MAIN);
  lv_obj_set_style_radius(k->container, 6, 0);
  lv_obj_set_style_pad_all(k->container, 4, 0);
  lv_obj_set_style_clip_corner(k->container, true, 0);
  lv_obj_set_style_border_color(k->container, theme.border, LV_PART_MAIN);
  lv_obj_set_style_border_width(k->container, 2, LV_PART_MAIN);
  lv_obj_set_style_border_opa(k->container, LV_OPA_80, LV_PART_MAIN);
  lv_obj_clear_flag(k->container, LV_OBJ_FLAG_SCROLLABLE);

  // Beer name (top area)
  k->lbl_name =
      ui_create_label(k->container, "", 2, 4, w - 12, 16, LV_TEXT_ALIGN_CENTER,
                      theme.text, &lv_font_montserrat_14);
  lv_obj_set_style_border_color(k->lbl_name, theme.border, LV_PART_MAIN);
  lv_obj_set_style_border_opa(k->lbl_name, LV_OPA_60, LV_PART_MAIN);

  // Three equal-sized metadata labels under the name (ABV, IBU, EBC)
  int meta_y = 4 + 16 + 2;
  int meta_h = 12;
  int meta_label_w = (w - 12 - 4) / 3;  // 3 labels with 2px gaps between them

  k->lbl_abv =
      ui_create_label(k->container, "", 2, meta_y, meta_label_w, meta_h,
                      LV_TEXT_ALIGN_CENTER, theme.text, &lv_font_kegmon_10);
  lv_obj_set_style_text_color(k->lbl_abv, theme.text, LV_PART_MAIN);
  lv_obj_set_style_text_align(k->lbl_abv, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

  k->lbl_ibu = ui_create_label(k->container, "", 2 + meta_label_w + 2, meta_y,
                               meta_label_w, meta_h, LV_TEXT_ALIGN_CENTER,
                               theme.text, &lv_font_kegmon_10);
  lv_obj_set_style_text_color(k->lbl_ibu, theme.text, LV_PART_MAIN);
  lv_obj_set_style_text_align(k->lbl_ibu, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

  k->lbl_ebc = ui_create_label(
      k->container, "", 2 + (meta_label_w + 2) * 2, meta_y, meta_label_w,
      meta_h, LV_TEXT_ALIGN_CENTER, theme.text, &lv_font_kegmon_10);
  lv_obj_set_style_text_color(k->lbl_ebc, theme.text, LV_PART_MAIN);
  lv_obj_set_style_text_align(k->lbl_ebc, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

  // Volume & progress bar in middle (left 60% of area)
  k->lbl_volume =
      ui_create_label(k->container, "", 2, h / 5 + 20, w - 12, 12,
                      LV_TEXT_ALIGN_CENTER, theme.text, &lv_font_montserrat_12);
  lv_obj_set_style_border_color(k->lbl_volume, theme.border, LV_PART_MAIN);
  lv_obj_set_style_border_opa(k->lbl_volume, LV_OPA_60, LV_PART_MAIN);

  k->bar_fill = lv_bar_create(k->container);
  lv_obj_set_pos(k->bar_fill, 2, h / 5 + 19);
  lv_obj_set_size(k->bar_fill, w - 12, 16);
  lv_bar_set_range(k->bar_fill, 0, 100);
  lv_bar_set_value(k->bar_fill, 0, LV_ANIM_OFF);
  lv_obj_set_style_border_color(k->bar_fill, theme.border, LV_PART_MAIN);
  lv_obj_set_style_border_opa(k->bar_fill, LV_OPA_60, LV_PART_MAIN);
  lv_obj_move_foreground(k->lbl_volume);

  // Combined "Last pour · Temp" label directly under the progress bar
  int bar_y = h / 5 + 26;
  int bar_h = 10;
  int info_y = bar_y + bar_h + 6;
  k->lbl_pours =
      ui_create_label(k->container, "", 2, info_y, w - 12, 12,
                      LV_TEXT_ALIGN_CENTER, theme.text, &lv_font_montserrat_12);
  lv_obj_set_style_border_color(k->lbl_pours, theme.border, LV_PART_MAIN);
  lv_obj_set_style_border_opa(k->lbl_pours, LV_OPA_60, LV_PART_MAIN);

  // State label (will be replaced by icons in next stage) placed below combined
  // info
  k->lbl_state =
      ui_create_label(k->container, "--", 2, info_y + 16, w - 12, 12,
                      LV_TEXT_ALIGN_CENTER, theme.text, &lv_font_montserrat_12);
  lv_obj_set_style_border_color(k->lbl_state, theme.border, LV_PART_MAIN);
  lv_obj_set_style_border_opa(k->lbl_state, LV_OPA_60, LV_PART_MAIN);

  k->flg_created = true;
}

/**
 * Delete any UI objects created by the active layout and clear pointers.
 *
 * This frees LVGL objects referenced in kegmon_ui_t structures and sets
 * those pointers to NULL so a fresh layout can be created.
 */
static void kegmon_cleanup_layout(void) {
  for (uint8_t i = 0; i < KEGMON_MAX_KEGS; i++) {
    kegmon_scale_t* k = &g_scales.scale[i];
    if (k->container) {
      lv_obj_del(k->container);
      k->container = NULL;
    }
    k->bar_fill = NULL;
    k->lbl_name = NULL;
    k->lbl_volume = NULL;
    k->lbl_state = NULL;
    k->lbl_pours = NULL;
    k->lbl_abv = NULL;
    k->lbl_ibu = NULL;
    k->lbl_ebc = NULL;
    k->flg_created = false;
  }
}

/**
 * Common initialization for the Kegmon UI.
 *
 * Performs one-time setup such as storing the display reference and
 * initializing default data buffers and the layout manager.
 *
 * @param disp Display pointer used by LVGL.
 * @param darkmode True to select dark theme defaults, false for light.
 */
static void kegmon_init_common(lv_disp_t* disp, bool darkmode) {
  obj_disp = disp;
  g_scales.data_darkmode = darkmode;

  // Get active screen
  lv_obj_t* scr = lv_scr_act();
  if (!scr) {
    return;
  }

  // Get theme colors
  ui_theme_colors_t theme_colors =
      ui_get_theme_colors(darkmode ? UI_THEME_DARK : UI_THEME_LIGHT);

  // Apply background
  lv_obj_set_style_bg_color(scr, theme_colors.bg, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

  // Initialize layout manager
  layout_mgr.current_layout = KEGMON_LAYOUT_4KEG;
  layout_mgr.total_layouts = 2;
}

/**
 * Setup layout 0 (4-keg): creates a 2x2 grid layout with all 4 kegs visible.
 */
static void kegmon_setup_layout_0(void) {
  lv_obj_t* scr = lv_scr_act();
  if (!scr) return;

  int32_t W = lv_obj_get_width(scr);
  int32_t H = lv_obj_get_height(scr);

  const int margin = 4;
  const int gap = 6;
  const int status_h = 18;

  int32_t total_h_available = H - status_h - margin * 2 - gap;
  int32_t h = total_h_available / 2;

  int32_t total_w_available = W - margin * 2 - gap;
  int32_t w = total_w_available / 2;

  // Create all 4 keg UIs in 2x2 grid
  for (uint8_t i = 0; i < KEGMON_MAX_KEGS; i++) {
    int col = i % 2;
    int row = i / 2;
    int32_t x = margin + col * (w + gap);
    int32_t y = margin + row * (h + gap);
    create_keg_ui(i, scr, x, y, w, h);
    lv_obj_clear_flag(g_scales.scale[i].container, LV_OBJ_FLAG_HIDDEN);
  }
}

/**
 * Setup layout 1 (2-keg): creates a 2-row layout with only 2 kegs visible
 * stacked vertically.
 */
static void kegmon_setup_layout_1(void) {
  lv_obj_t* scr = lv_scr_act();
  if (!scr) return;

  int32_t W = lv_obj_get_width(scr);
  int32_t H = lv_obj_get_height(scr);

  const int margin = 4;
  const int gap = 6;
  const int status_h = 18;

  int32_t total_h_available = H - status_h - margin * 2 - gap;
  int32_t h = total_h_available / 2;  // Each keg gets half the available height

  int32_t total_w_available = W - margin * 2;
  int32_t w = total_w_available;  // Full width for each row

  // Create only kegs 0 and 1 stacked vertically in 2 rows
  for (uint8_t i = 0; i < 2; i++) {
    int row = i;
    int32_t x = margin;
    int32_t y = margin + row * (h + gap);
    create_keg_ui(i, scr, x, y, w, h);
    lv_obj_clear_flag(g_scales.scale[i].container, LV_OBJ_FLAG_HIDDEN);
  }
}

void kegmon_init(lv_disp_t* disp, bool darkmode, uint8_t layout_id) {
  if (!disp) {
    return;
  }

  // Common initialization (one-time setup)
  kegmon_init_common(disp, darkmode);

  // Clamp layout_id to valid range (0..1)
  if (layout_id >= layout_mgr.total_layouts) {
    layout_id = 0;
  }
  layout_mgr.current_layout = layout_id;

  // Setup requested layout
  if (layout_id == KEGMON_LAYOUT_4KEG) {
    kegmon_setup_layout_0();
  } else if (layout_id == KEGMON_LAYOUT_2KEG) {
    kegmon_setup_layout_1();
  }

  // Create bottom status label (matching gravitymon style)
  lv_obj_t* scr = lv_scr_act();
  int32_t H = lv_obj_get_height(scr);
  int32_t W = lv_obj_get_width(scr);
  const int status_h = 18;
  int status_y = H - status_h - 2;

  ui_theme_colors_t theme = ui_get_theme_colors(
      g_scales.data_darkmode ? UI_THEME_DARK : UI_THEME_LIGHT);
  g_scales.lbl_status_label = ui_create_status_label(
      scr, "", 4, status_y, W - 8, status_h, LV_TEXT_ALIGN_CENTER, theme.text,
      &lv_font_montserrat_12);
  lv_obj_set_style_bg_color(g_scales.lbl_status_label, theme.status_bg,
                            LV_PART_MAIN);
  lv_obj_set_style_bg_opa(g_scales.lbl_status_label, LV_OPA_60, LV_PART_MAIN);
  lv_obj_set_style_border_width(g_scales.lbl_status_label, 0, LV_PART_MAIN);
  lv_obj_set_style_border_color(g_scales.lbl_status_label, theme.border,
                                LV_PART_MAIN);
  lv_obj_set_style_radius(g_scales.lbl_status_label, 0, LV_PART_MAIN);
  lv_obj_move_foreground(g_scales.lbl_status_label);
}

void kegmon_cleanup(void) {
  for (uint8_t i = 0; i < KEGMON_MAX_KEGS; i++) {
    if (g_scales.scale[i].flg_created && g_scales.scale[i].container)
      lv_obj_del(g_scales.scale[i].container);
  }

  memset(&g_scales.scale, 0, sizeof(g_scales.scale));
}

// void kegmon_update_scale(uint8_t index, const kegmon_scale_t* s) {
//   if (!g_kegs || index >= KEGMON_MAX_KEGS || !s) return;
//   // copy into last_scale for compare in loop
//   memcpy(&g_kegs[index].scale, s, sizeof(kegmon_scale_t));
// }

void kegmon_set_temperature(uint8_t index, float temp) {
  if (index >= KEGMON_MAX_KEGS) return;
  g_scales.scale[index].data_temp = temp;
}

// Granular setters follow the pattern in other UI modules: update cached state
void kegmon_set_beer_name(uint8_t index, const char* name) {
  if (index >= KEGMON_MAX_KEGS) return;
  if (!name) return;
  snprintf(g_scales.scale[index].data_beer_name,
           sizeof(g_scales.scale[index].data_beer_name), "%s", name);
}

void kegmon_set_connected(uint8_t index, bool connected) {
  if (index >= KEGMON_MAX_KEGS) return;
  g_scales.scale[index].data_connected = connected;
}

void kegmon_set_keg_volume(uint8_t index, float keg_volume) {
  if (index >= KEGMON_MAX_KEGS) return;
  g_scales.scale[index].data_keg_volume = keg_volume;
}

void kegmon_set_stable_weight(uint8_t index, float weight) {
  if (index >= KEGMON_MAX_KEGS) return;
  g_scales.scale[index].data_stable_weight = weight;
}

void kegmon_set_stable_volume(uint8_t index, float stable_volume) {
  if (index >= KEGMON_MAX_KEGS) return;
  g_scales.scale[index].data_stable_volume = stable_volume;
}

void kegmon_set_last_pour_volume(uint8_t index, float last_pour) {
  if (index >= KEGMON_MAX_KEGS) return;
  g_scales.scale[index].data_last_pour_volume = last_pour;
}

void kegmon_set_beer_meta(uint8_t index, float abv, int ebc, int ibu) {
  if (index >= KEGMON_MAX_KEGS) return;

  g_scales.scale[index].data_beer_abv = abv;
  g_scales.scale[index].data_beer_ebc = ebc;
  g_scales.scale[index].data_beer_ibu = ibu;
}

void kegmon_set_theme(bool darkmode) { g_scales.data_darkmode = darkmode; }

void kegmon_set_volume_unit(const char* format) {
  if (format) {
    snprintf(g_scales.data_volume_unit, sizeof(g_scales.data_volume_unit), "%s",
             format);
  }
}

void kegmon_set_temp_unit(const char* format) {
  if (format) {
    snprintf(g_scales.data_temp_unit, sizeof(g_scales.data_temp_unit), "%s",
             format);
  }
}

void kegmon_set_event_type(uint8_t index, ChangeDetectionEventType event_type) {
  if (index >= KEGMON_MAX_KEGS) return;
  g_scales.scale[index].data_event_type = event_type;
}

void kegmon_set_status_bar(const char* status) {
  if (!status) return;
  snprintf(g_scales.data_status, sizeof(g_scales.data_status), "%s", status);
}

/**
 * Get the current layout ID.
 *
 * @return Current layout ID (KEGMON_LAYOUT_4KEG or KEGMON_LAYOUT_2KEG)
 */
uint8_t kegmon_get_layout(void) { return layout_mgr.current_layout; }

/**
 * Switch to the specified layout ID at runtime.
 *
 * Cleans up the current layout, resets state, and constructs the chosen
 * layout. If the requested `layout_id` is invalid it will wrap to 0.
 *
 * @param layout_id Layout index to activate (KEGMON_LAYOUT_4KEG or
 * KEGMON_LAYOUT_2KEG)
 */
void kegmon_set_layout(uint8_t layout_id) {
  if (layout_id >= layout_mgr.total_layouts) {
    layout_id = 0;
  }

  // If the requested layout is already active, do nothing
  if (layout_id == layout_mgr.current_layout) {
    return;
  }

  // Cleanup current layout objects
  kegmon_cleanup_layout();

  // Update layout ID
  layout_mgr.current_layout = layout_id;

  // Setup new layout
  if (layout_id == KEGMON_LAYOUT_4KEG) {
    kegmon_setup_layout_0();
  } else if (layout_id == KEGMON_LAYOUT_2KEG) {
    kegmon_setup_layout_1();
  }
}

void kegmon_loop(void) {
  // Update status bar from buffer
  if (g_scales.lbl_status_label) {
    lv_label_set_text(g_scales.lbl_status_label, g_scales.data_status);
  }

  for (uint8_t i = 0; i < KEGMON_MAX_KEGS; i++) {
    kegmon_scale_t* k = &g_scales.scale[i];
    if (!k->flg_created) continue;

    // Apply scale values (cached in last_scale)
    char buf[64];

    // Update beer name label from cached name (set by setter)
    if (k->lbl_name) {
      lv_label_set_text(k->lbl_name, k->data_beer_name);
    }
   
    // Volume label: show percentage of capacity
    if (k->lbl_volume) {
      if (k->data_keg_volume > 0) {
        int pct = static_cast<int>(
            (k->data_stable_volume / k->data_keg_volume) * 100.0f);
        if (pct < 0) pct = 0;
        if (pct > 100) pct = 100;
        snprintf(buf, sizeof(buf), "%d%%", pct);
      } else {
        snprintf(buf, sizeof(buf), "--");
      }
      lv_label_set_text(k->lbl_volume, buf);
    }

    // Progress bar: percent of capacity
    if (k->bar_fill && k->data_keg_volume > 0) {
      int pct = static_cast<int>((k->data_stable_volume / k->data_keg_volume) *
                                 100.0f);
      if (pct < 0) pct = 0;
      if (pct > 100) pct = 100;
      lv_bar_set_value(k->bar_fill, pct, LV_ANIM_OFF);
    }

    // Update ABV, IBU, EBC labels with values from `kegmon_set_beer_meta()`
    if (k->lbl_abv) {
      if (k->data_beer_abv > 0.0f) {
        snprintf(buf, sizeof(buf), LV_SYMBOL_BEER " %.1f", k->data_beer_abv);
      } else {
        snprintf(buf, sizeof(buf), "");
      }
      lv_label_set_text(k->lbl_abv, buf);
    }
    if (k->lbl_ibu) {
      if (k->data_beer_ibu > 0) {
        snprintf(buf, sizeof(buf), LV_SYMBOL_LEAF " %d", k->data_beer_ibu);
      } else {
        snprintf(buf, sizeof(buf), "");
      }
      lv_label_set_text(k->lbl_ibu, buf);
    }

    if (k->lbl_ebc) {
      if (k->data_beer_ebc > 0) {
        snprintf(buf, sizeof(buf), LV_SYMBOL_PALETTE " %d", k->data_beer_ebc);
      } else {
        snprintf(buf, sizeof(buf), "");
      }
      lv_label_set_text(k->lbl_ebc, buf);
    }

    // State label / Event icon mapping (event_type takes precedence)
    if (k->lbl_state) {
      switch (k->data_event_type) {
        case ChangeDetectionEventType::SYSTEM_STARTUP:
          snprintf(buf, sizeof(buf), "Starting");
          break;
        case ChangeDetectionEventType::SETTLING_STARTED:
          snprintf(buf, sizeof(buf), LV_SYMBOL_LOOP " Settling");
          break;
        case ChangeDetectionEventType::STABLE_LEVEL:
          snprintf(buf, sizeof(buf), LV_SYMBOL_MINUS " Stable");
          break;
        case ChangeDetectionEventType::POURING:
          snprintf(buf, sizeof(buf), LV_SYMBOL_DOWN " Pouring");
          break;
        case ChangeDetectionEventType::WEIGHT_CHANGE_DETECTED:
          snprintf(buf, sizeof(buf), LV_SYMBOL_UP " Weight changed");
          break;
        case ChangeDetectionEventType::POUR_COMPLETED:
          snprintf(buf, sizeof(buf), LV_SYMBOL_OK " Poured");
          break;
        case ChangeDetectionEventType::KEG_REMOVED:
          snprintf(buf, sizeof(buf), LV_SYMBOL_CLOSE " Keg gone");
          break;
        case ChangeDetectionEventType::KEG_REPLACED:
          snprintf(buf, sizeof(buf), LV_SYMBOL_REFRESH " Replaced");
          break;
        case ChangeDetectionEventType::KEG_ABSENT_TIMEOUT:
          snprintf(buf, sizeof(buf), LV_SYMBOL_CLOSE " No Keg");
          break;
        case ChangeDetectionEventType::INVALID_WEIGHT:
          snprintf(buf, sizeof(buf), LV_SYMBOL_WARNING " Error");
          break;
        case ChangeDetectionEventType::LOAD_CELL_ERROR:
          snprintf(buf, sizeof(buf), LV_SYMBOL_WARNING " Sensor Err");
          break;
        case ChangeDetectionEventType::LOAD_CELL_RECOVERED:
          snprintf(buf, sizeof(buf), LV_SYMBOL_OK " Sensor OK");
          break;
        case ChangeDetectionEventType::SENSOR_RECOVERED:
          snprintf(buf, sizeof(buf), LV_SYMBOL_OK " Weight OK");
          break;
        case ChangeDetectionEventType::CALIBRATION_NEEDED:
          snprintf(buf, sizeof(buf), "Calibration Needed");
          break;
        case ChangeDetectionEventType::CALIBRATION_COMPLETE:
          snprintf(buf, sizeof(buf), LV_SYMBOL_OK " Calibrated");
          break;
        case ChangeDetectionEventType::HARDWARE_DISABLED:
          snprintf(buf, sizeof(buf), "Inactive");
          break;
        default:
          snprintf(buf, sizeof(buf), "--");
          break;
      }
      lv_label_set_text(k->lbl_state, buf);
    }

    // Combined "Last pour · Temp" label under the progress bar
    if (k->lbl_pours) {
      char info[64];
      if (!isnan(k->data_temp)) {
        snprintf(info, sizeof(info), "Last %.0f  %s - %.1f°%s",
                 k->data_last_pour_volume, g_scales.data_volume_unit, k->data_temp,
                 g_scales.data_temp_unit);
      } else {
        snprintf(info, sizeof(info), "Pour %.0f %s", k->data_last_pour_volume,
                 g_scales.data_volume_unit);
      }

      lv_label_set_text(k->lbl_pours, info);
    }
  }
}

#endif  // ENABLE_LVGL

// EOF
