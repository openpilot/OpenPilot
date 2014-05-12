/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup OSDgenModule osdgen Module
 * @brief Process OSD information
 * @{
 *
 * @file       osdgen.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      OSD gen module, handles OSD draw. Parts from CL-OSD and SUPEROSD projects
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef OSDGEN_H_
#define OSDGEN_H_

#include "openpilot.h"
#include "pios.h"

int32_t osdgenInitialize(void);

// Needed till someone fixes the black/white hardware problem
//#define ONLY_WHITE_PIXEL
#ifdef ONLY_WHITE_PIXEL
#define GCS_LEVEL_ONLY_WHITE_PIXEL      4095
#define CHECK_ONLY_WHITE_PIXEL          if (only_white_pixel) mmode = (lmode & 1);
#define CHECK_ONLY_WHITE_PIXEL_CHAR     if (only_white_pixel) mask = level;
#else
#define CHECK_ONLY_WHITE_PIXEL
#define CHECK_ONLY_WHITE_PIXEL_CHAR
#endif

// Size of an array (num items.)
#define SIZEOF_ARRAY(x) (sizeof(x) / sizeof((x)[0]))

#define HUD_VSCALE_FLAG_CLEAR       1
#define HUD_VSCALE_FLAG_NO_NEGATIVE 2

// Macros for computing addresses and bit positions.
#define CALC_BUFF_ADDR(x, y) (((x) / 8) + ((y) * BUFFER_WIDTH))
#define CALC_BIT_IN_BYTE(x)  ((x) & 7)

// Macro for writing a byte with a mode (NAND = clear, OR = set, XOR = toggle) at a given position
#define WRITE_BYTE_MODE(buff, addr, mask, mode) \
    switch (mode) { \
    case 0: buff[addr] &= ~(mask); break; \
    case 1: buff[addr] |= (mask); break; \
    case 2: buff[addr] ^= (mask); break; }

// Macros for writing a byte nand, or, xor at a given position
#define WRITE_BYTE_NAND(buff, addr, mask) { buff[addr] &= ~(mask); }
#define WRITE_BYTE_OR(buff, addr, mask)   { buff[addr] |= (mask); }
#define WRITE_BYTE_XOR(buff, addr, mask)  { buff[addr] ^= (mask); }

// Horizontal line calculations.
// Edge cases.
#define COMPUTE_HLINE_EDGE_L_MASK(b)      ((1 << (8 - (b))) - 1)
#define COMPUTE_HLINE_EDGE_R_MASK(b)      (~((1 << (7 - (b))) - 1))
// This computes an island mask.
#define COMPUTE_HLINE_ISLAND_MASK(b0, b1) (COMPUTE_HLINE_EDGE_L_MASK(b0) ^ COMPUTE_HLINE_EDGE_L_MASK(b1));

// Macro for initializing stroke/fill modes. Add new modes here
// if necessary.
#define SETUP_STROKE_FILL(stroke, fill, mode) \
    stroke = 0; fill = 0; \
    if (mode == 0) { stroke = 0; fill = 1; } \
    if (mode == 1) { stroke = 1; fill = 0; } \

// Line endcaps (for horizontal and vertical lines.)
#define ENDCAP_NONE  0
#define ENDCAP_ROUND 1
#define ENDCAP_FLAT  2

#define DRAW_ENDCAP_HLINE(e, x, y, s, f, l) \
    if ((e) == ENDCAP_ROUND) /* single pixel endcap */ \
    { write_pixel_lm(x, y, f, l); } \
    else if ((e) == ENDCAP_FLAT) /* flat endcap: FIXME, quicker to draw a vertical line(?) */ \
    { write_pixel_lm(x, y - 1, s, l); write_pixel_lm(x, y, s, l); write_pixel_lm(x, y + 1, s, l); }

#define DRAW_ENDCAP_VLINE(e, x, y, s, f, l) \
    if ((e) == ENDCAP_ROUND) /* single pixel endcap */ \
    { write_pixel_lm(x, y, f, l); } \
    else if ((e) == ENDCAP_FLAT) /* flat endcap: FIXME, quicker to draw a horizontal line(?) */ \
    { write_pixel_lm(x - 1, y, s, l); write_pixel_lm(x, y, s, l); write_pixel_lm(x + 1, y, s, l); }

// Macros for writing pixels in a midpoint circle algorithm.
#define CIRCLE_PLOT_8(buff, cx, cy, x, y, mode) \
    CIRCLE_PLOT_4(buff, cx, cy, x, y, mode); \
    if ((x) != (y)) { CIRCLE_PLOT_4(buff, cx, cy, y, x, mode); }

#define CIRCLE_PLOT_4(buff, cx, cy, x, y, mode) \
    write_pixel(buff, (cx) + (x), (cy) + (y), mode); \
    write_pixel(buff, (cx) - (x), (cy) + (y), mode); \
    write_pixel(buff, (cx) + (x), (cy) - (y), mode); \
    write_pixel(buff, (cx) - (x), (cy) - (y), mode);

// Font flags.
#define FONT_BOLD      1               // bold text (no outline)
#define FONT_INVERT    2               // invert: border white, inside black
// Text alignments.
#define TEXT_VA_TOP    0
#define TEXT_VA_MIDDLE 1
#define TEXT_VA_BOTTOM 2
#define TEXT_HA_LEFT   0
#define TEXT_HA_CENTER 1
#define TEXT_HA_RIGHT  2

// Text dimension structures.
struct FontDimensions {
    int width, height;
};

// to convert metric -> imperial
// for speeds see http://en.wikipedia.org/wiki/Miles_per_hour
typedef struct {                    // from		metric			imperial
    float   m_to_m_feet;            // m		m		1.0		feet	3.280840
    float   ms_to_ms_fts;           // m/s		m/s		1.0		ft/s	3.280840
    float   ms_to_kmh_mph;          // m/s		km/h	3.6		mph		2.236936
    uint8_t char_m_feet;            // char		'm'				'f'
    uint8_t char_ms_fts;            // char		'm/s'			'ft/s'
} Unit;

// Home position for calculations
typedef struct {
    int32_t  Latitude;
    int32_t  Longitude;
    float    Altitude;
    uint8_t  GotHome;
    uint32_t Distance;
    uint16_t Direction;
} HomePosition;

// ADC values filtered
typedef struct {
    double rssi;
    double flight;
    double video;
    double volt;
    double curr;
} ADCfiltered;

// Max/Min macros.
#define MAX(a, b)                    ((a) > (b) ? (a) : (b))
#define MIN(a, b)                    ((a) < (b) ? (a) : (b))
#define MAX3(a, b, c)                MAX(a, MAX(b, c))
#define MIN3(a, b, c)                MIN(a, MIN(b, c))

// Check if coordinates are valid. If not, return. Assumes signed coordinates for working correct also with values lesser than 0.
#define CHECK_COORDS(x, y)           if (x < GRAPHICS_LEFT || x > GRAPHICS_RIGHT || y < GRAPHICS_TOP || y > GRAPHICS_BOTTOM) { return; }
#define CHECK_COORD_X(x)             if (x < GRAPHICS_LEFT || x > GRAPHICS_RIGHT) { return; }
#define CHECK_COORD_Y(y)             if (y < GRAPHICS_TOP  || y > GRAPHICS_BOTTOM) { return; }

// Clip coordinates out of range. Assumes signed coordinates for working correct also with values lesser than 0.
#define CLIP_COORDS(x, y)            { CLIP_COORD_X(x); CLIP_COORD_Y(y); }
#define CLIP_COORD_X(x)              { x = x < GRAPHICS_LEFT ? GRAPHICS_LEFT : x > GRAPHICS_RIGHT ? GRAPHICS_RIGHT : x; }
#define CLIP_COORD_Y(y)              { y = y < GRAPHICS_TOP ? GRAPHICS_TOP : y > GRAPHICS_BOTTOM ? GRAPHICS_BOTTOM : y; }

// Macro to swap two variables using XOR swap.
#define SWAP(a, b)                   { a ^= b; b ^= a; a ^= b; }

uint8_t getCharData(uint16_t charPos);

void clearGraphics();

void drawArrow(uint16_t x, uint16_t y, uint16_t angle, uint16_t size);
void drawBox(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void write_pixel(uint8_t *buff, int x, int y, int mode);
void write_pixel_lm(int x, int y, int mmode, int lmode);

void write_hline(uint8_t *buff, int x0, int x1, int y, int mode);
void write_hline_lm(int x0, int x1, int y, int lmode, int mmode);
void write_hline_outlined(int x0, int x1, int y, int endcap0, int endcap1, int mode, int mmode);

void write_vline(uint8_t *buff, int x, int y0, int y1, int mode);
void write_vline_lm(int x, int y0, int y1, int lmode, int mmode);
void write_vline_outlined(int x, int y0, int y1, int endcap0, int endcap1, int mode, int mmode);

void write_filled_rectangle(uint8_t *buff, int x, int y, int width, int height, int mode);
void write_filled_rectangle_lm(int x, int y, int width, int height, int lmode, int mmode);
void write_rectangle_outlined(int x, int y, int width, int height, int mode, int mmode);

void write_circle(uint8_t *buff, int cx, int cy, int r, int dashp, int mode);
void write_circle_outlined(int cx, int cy, int r, int dashp, int bmode, int mode, int mmode);
void write_circle_filled(uint8_t *buff, int cx, int cy, int r, int mode);

void write_line(uint8_t *buff, int x0, int y0, int x1, int y1, int mode);
void write_line_lm(int x0, int y0, int x1, int y1, int mmode, int lmode);
void write_line_outlined(int x0, int y0, int x1, int y1, int endcap0, int endcap1, int mode, int mmode);
void write_line_outlined_dashed(int x0, int y0, int x1, int y1, int endcap0, int endcap1, int mode, int mmode, int dots);

void write_word_misaligned(uint8_t *buff, uint16_t word, unsigned int addr, unsigned int xoff, int mode);
void write_word_misaligned_NAND(uint8_t *buff, uint16_t word, unsigned int addr, unsigned int xoff);
void write_word_misaligned_OR(uint8_t *buff, uint16_t word, unsigned int addr, unsigned int xoff);

void write_byte_misaligned_NAND(uint8_t *buff, uint8_t byte, unsigned int addr, unsigned int xoff);
void write_byte_misaligned_OR(uint8_t *buff, uint8_t byte, unsigned int addr, unsigned int xoff);

void write_char16(char ch, int x, int y, int font);
void write_char(char ch, int x, int y, int flags, int font);

void write_string(char *str, int x, int y, int xs, int ys, int va, int ha, int flags, int font);

#endif /* OSDGEN_H_ */
