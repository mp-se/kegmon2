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
#ifdef __has_include
#if __has_include("lvgl.h")
#ifndef LV_LVGL_H_INCLUDE_SIMPLE
#define LV_LVGL_H_INCLUDE_SIMPLE
#endif
#endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include <lvgl.h>
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_FONT_KEGMON_10
#define LV_FONT_KEGMON_10 1
#endif

#if LV_FONT_KEGMON_10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+0030 "0" */
    0x76, 0xe3, 0x18, 0xed, 0xc0,

    /* U+0031 "1" */
    0x23, 0x8, 0x42, 0x13, 0xe0,

    /* U+0032 "2" */
    0x70, 0x42, 0x22, 0x23, 0xe0,

    /* U+0033 "3" */
    0x70, 0x42, 0x60, 0xc5, 0xc0,

    /* U+0034 "4" */
    0x8, 0x62, 0x8a, 0x4b, 0xf0, 0x80,

    /* U+0035 "5" */
    0xfc, 0x3d, 0x10, 0xc5, 0xc0,

    /* U+0036 "6" */
    0x76, 0x2d, 0x98, 0xe5, 0xc0,

    /* U+0037 "7" */
    0xf8, 0x44, 0x22, 0x21, 0x0,

    /* U+0038 "8" */
    0x74, 0x62, 0xe8, 0xc5, 0xc0,

    /* U+0039 "9" */
    0x74, 0x63, 0x17, 0x87, 0xc0,

    /* U+0041 "A" */
    0x10, 0x50, 0xa2, 0x27, 0xc8, 0xe0, 0x80,

    /* U+0042 "B" */
    0xf4, 0x63, 0xe8, 0xc7, 0xc0,

    /* U+0043 "C" */
    0x7b, 0x18, 0x20, 0x83, 0x17, 0x80,

    /* U+0044 "D" */
    0xfa, 0x38, 0x61, 0x86, 0x3f, 0x80,

    /* U+0045 "E" */
    0xfc, 0x21, 0xe8, 0x43, 0xe0,

    /* U+0046 "F" */
    0xfc, 0x21, 0xe8, 0x42, 0x0,

    /* U+0047 "G" */
    0x7b, 0x18, 0x27, 0x87, 0x17, 0xc0,

    /* U+0048 "H" */
    0x86, 0x18, 0x7f, 0x86, 0x18, 0x40,

    /* U+0049 "I" */
    0xfe,

    /* U+004A "J" */
    0x24, 0x92, 0x49, 0xe0,

    /* U+004B "K" */
    0x8a, 0x4a, 0x30, 0xa2, 0x48, 0x80,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x43, 0xe0,

    /* U+004D "M" */
    0x83, 0x8f, 0x1e, 0x5a, 0xb6, 0x64, 0x80,

    /* U+004E "N" */
    0x87, 0x1a, 0x69, 0x96, 0x38, 0x40,

    /* U+004F "O" */
    0x3d, 0x8e, 0xc, 0x18, 0x38, 0x9e, 0x0,

    /* U+0050 "P" */
    0xf4, 0x63, 0xe8, 0x42, 0x0,

    /* U+0051 "Q" */
    0x3d, 0x8e, 0xc, 0x18, 0x38, 0x9e, 0x4, 0x4,

    /* U+0052 "R" */
    0xf4, 0x63, 0xe9, 0x46, 0x20,

    /* U+0053 "S" */
    0x74, 0x60, 0xc1, 0xc5, 0xc0,

    /* U+0054 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x80,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x17, 0x80,

    /* U+0056 "V" */
    0x82, 0x89, 0x12, 0x42, 0x85, 0x4, 0x0,

    /* U+0057 "W" */
    0x84, 0x29, 0x89, 0x29, 0x25, 0x23, 0x28, 0x63, 0x4, 0x40,

    /* U+0058 "X" */
    0x45, 0x23, 0x4, 0x29, 0x28, 0x40,

    /* U+0059 "Y" */
    0x44, 0x88, 0xa0, 0x81, 0x2, 0x4, 0x0,

    /* U+005A "Z" */
    0xf8, 0x44, 0x44, 0x43, 0xe0,

    /* U+0061 "a" */
    0x6c, 0xe3, 0x17, 0x80,

    /* U+0062 "b" */
    0x84, 0x3d, 0x18, 0xc7, 0xc0,

    /* U+0063 "c" */
    0x78, 0x88, 0x70,

    /* U+0064 "d" */
    0x8, 0x5f, 0x18, 0xc5, 0xe0,

    /* U+0065 "e" */
    0x74, 0xbd, 0x7, 0x0,

    /* U+0066 "f" */
    0x6b, 0xa4, 0x90,

    /* U+0067 "g" */
    0x7c, 0x63, 0x16, 0x85, 0xc0,

    /* U+0068 "h" */
    0x88, 0xf9, 0x99, 0x90,

    /* U+0069 "i" */
    0xbe,

    /* U+006A "j" */
    0x45, 0x55, 0xc0,

    /* U+006B "k" */
    0x84, 0x25, 0x4c, 0x52, 0x40,

    /* U+006C "l" */
    0xfe,

    /* U+006D "m" */
    0xef, 0x26, 0x4c, 0x99, 0x20,

    /* U+006E "n" */
    0xf9, 0x99, 0x90,

    /* U+006F "o" */
    0x74, 0x63, 0x17, 0x0,

    /* U+0070 "p" */
    0xf4, 0x63, 0x1f, 0x42, 0x0,

    /* U+0071 "q" */
    0x7c, 0x63, 0x17, 0x84, 0x20,

    /* U+0072 "r" */
    0xf2, 0x48,

    /* U+0073 "s" */
    0xe8, 0x43, 0xf0,

    /* U+0074 "t" */
    0xb, 0xa4, 0x98,

    /* U+0075 "u" */
    0x99, 0x99, 0xf0,

    /* U+0076 "v" */
    0x8a, 0x54, 0xa2, 0x0,

    /* U+0077 "w" */
    0x89, 0x59, 0x55, 0x66, 0x22,

    /* U+0078 "x" */
    0x4a, 0x88, 0xa4, 0x80,

    /* U+0079 "y" */
    0x8a, 0x54, 0xa2, 0x13, 0x0,

    /* U+007A "z" */
    0xf2, 0x48, 0xf0,

    /* U+F06C "" */
    0x0, 0x20, 0xc, 0x3f, 0x8f, 0xf3, 0xe, 0x1f, 0xcf, 0xf3, 0xfc, 0xcf, 0x10,
    0x0,

    /* U+F0FC "" */
    0xfc, 0x7f, 0xbf, 0x3f, 0x9f, 0xcf, 0xe7, 0xfd, 0xf8, 0xfc, 0x0,

    /* U+F53F "" */
    0x3f, 0x1f, 0xef, 0x7a, 0x7b, 0xff, 0xef, 0xff, 0xc1, 0xf0, 0x3c, 0x7, 0x0};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0,
     .adv_w = 0,
     .box_w = 0,
     .box_h = 0,
     .ofs_x = 0,
     .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0,
     .adv_w = 63,
     .box_w = 1,
     .box_h = 1,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 1,
     .adv_w = 68,
     .box_w = 3,
     .box_h = 1,
     .ofs_x = 1,
     .ofs_y = 3},
    {.bitmap_index = 2,
     .adv_w = 37,
     .box_w = 1,
     .box_h = 1,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 3,
     .adv_w = 96,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 8,
     .adv_w = 105,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 13,
     .adv_w = 97,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 18,
     .adv_w = 93,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 23,
     .adv_w = 98,
     .box_w = 6,
     .box_h = 7,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 29,
     .adv_w = 101,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 34,
     .adv_w = 97,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 39,
     .adv_w = 96,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 44,
     .adv_w = 96,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 49,
     .adv_w = 95,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 54,
     .adv_w = 118,
     .box_w = 7,
     .box_h = 7,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 61,
     .adv_w = 113,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 66,
     .adv_w = 112,
     .box_w = 6,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 72,
     .adv_w = 124,
     .box_w = 6,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 78,
     .adv_w = 100,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 83,
     .adv_w = 98,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 88,
     .adv_w = 114,
     .box_w = 6,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 94,
     .adv_w = 123,
     .box_w = 6,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 100,
     .adv_w = 41,
     .box_w = 1,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 101,
     .adv_w = 39,
     .box_w = 3,
     .box_h = 9,
     .ofs_x = 0,
     .ofs_y = -1},
    {.bitmap_index = 105,
     .adv_w = 117,
     .box_w = 6,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 111,
     .adv_w = 98,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 116,
     .adv_w = 136,
     .box_w = 7,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 123,
     .adv_w = 119,
     .box_w = 6,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 129,
     .adv_w = 124,
     .box_w = 7,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 136,
     .adv_w = 111,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 141,
     .adv_w = 124,
     .box_w = 7,
     .box_h = 9,
     .ofs_x = 1,
     .ofs_y = -1},
    {.bitmap_index = 149,
     .adv_w = 118,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 154,
     .adv_w = 100,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 159,
     .adv_w = 97,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 164,
     .adv_w = 117,
     .box_w = 6,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 170,
     .adv_w = 112,
     .box_w = 7,
     .box_h = 7,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 177,
     .adv_w = 178,
     .box_w = 11,
     .box_h = 7,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 187,
     .adv_w = 104,
     .box_w = 6,
     .box_h = 7,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 193,
     .adv_w = 103,
     .box_w = 7,
     .box_h = 7,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 200,
     .adv_w = 97,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 205,
     .adv_w = 91,
     .box_w = 5,
     .box_h = 5,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 209,
     .adv_w = 101,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 214,
     .adv_w = 83,
     .box_w = 4,
     .box_h = 5,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 217,
     .adv_w = 91,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 222,
     .adv_w = 88,
     .box_w = 5,
     .box_h = 5,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 226,
     .adv_w = 53,
     .box_w = 3,
     .box_h = 7,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 229,
     .adv_w = 92,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = -1},
    {.bitmap_index = 234,
     .adv_w = 94,
     .box_w = 4,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 238,
     .adv_w = 38,
     .box_w = 1,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 239,
     .adv_w = 36,
     .box_w = 2,
     .box_h = 9,
     .ofs_x = 0,
     .ofs_y = -1},
    {.bitmap_index = 242,
     .adv_w = 96,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 247,
     .adv_w = 38,
     .box_w = 1,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 248,
     .adv_w = 140,
     .box_w = 7,
     .box_h = 5,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 253,
     .adv_w = 93,
     .box_w = 4,
     .box_h = 5,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 256,
     .adv_w = 92,
     .box_w = 5,
     .box_h = 5,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 260,
     .adv_w = 100,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = -1},
    {.bitmap_index = 265,
     .adv_w = 93,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = -1},
    {.bitmap_index = 270,
     .adv_w = 68,
     .box_w = 3,
     .box_h = 5,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 272,
     .adv_w = 79,
     .box_w = 4,
     .box_h = 5,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 275,
     .adv_w = 54,
     .box_w = 3,
     .box_h = 7,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 278,
     .adv_w = 92,
     .box_w = 4,
     .box_h = 5,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 281,
     .adv_w = 85,
     .box_w = 5,
     .box_h = 5,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 285,
     .adv_w = 139,
     .box_w = 8,
     .box_h = 5,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 290,
     .adv_w = 86,
     .box_w = 5,
     .box_h = 5,
     .ofs_x = 0,
     .ofs_y = 1},
    {.bitmap_index = 294,
     .adv_w = 85,
     .box_w = 5,
     .box_h = 7,
     .ofs_x = 0,
     .ofs_y = -1},
    {.bitmap_index = 299,
     .adv_w = 84,
     .box_w = 4,
     .box_h = 5,
     .ofs_x = 1,
     .ofs_y = 1},
    {.bitmap_index = 302,
     .adv_w = 180,
     .box_w = 11,
     .box_h = 10,
     .ofs_x = 0,
     .ofs_y = -1},
    {.bitmap_index = 316,
     .adv_w = 140,
     .box_w = 9,
     .box_h = 9,
     .ofs_x = 0,
     .ofs_y = -1},
    {.bitmap_index = 327,
     .adv_w = 160,
     .box_w = 10,
     .box_h = 10,
     .ofs_x = 0,
     .ofs_y = -1}};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {0x0, 0xd, 0xe};

static const uint16_t unicode_list_4[] = {0x0, 0x90, 0x4d3};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] = {
    {.range_start = 32,
     .range_length = 15,
     .glyph_id_start = 1,
     .unicode_list = unicode_list_0,
     .glyph_id_ofs_list = NULL,
     .list_length = 3,
     .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY},
    {.range_start = 48,
     .range_length = 10,
     .glyph_id_start = 4,
     .unicode_list = NULL,
     .glyph_id_ofs_list = NULL,
     .list_length = 0,
     .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY},
    {.range_start = 65,
     .range_length = 26,
     .glyph_id_start = 14,
     .unicode_list = NULL,
     .glyph_id_ofs_list = NULL,
     .list_length = 0,
     .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY},
    {.range_start = 97,
     .range_length = 26,
     .glyph_id_start = 40,
     .unicode_list = NULL,
     .glyph_id_ofs_list = NULL,
     .list_length = 0,
     .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY},
    {.range_start = 61548,
     .range_length = 1236,
     .glyph_id_start = 66,
     .unicode_list = unicode_list_4,
     .glyph_id_ofs_list = NULL,
     .list_length = 3,
     .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY}};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 5,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};

/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t lv_font_kegmon_10 = {
#else
lv_font_t lv_font_kegmon_10 = {
#endif
    .get_glyph_dsc =
        lv_font_get_glyph_dsc_fmt_txt, /*Function pointer to get glyph's data*/
    .get_glyph_bitmap =
        lv_font_get_bitmap_fmt_txt, /*Function pointer to get glyph's bitmap*/
    .line_height = 10, /*The maximum line height required by the font*/
    .base_line = 1,    /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -4,
    .underline_thickness = 0,
#endif
    .static_bitmap = 0,
    .dsc = &font_dsc,  // The custom font data. Will be accessed by
                       // get_glyph_bitmap/dsc`
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};

#endif  // LV_FONT_KEGMON_10
