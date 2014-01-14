/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup OSDgenModule osdgen Module
 * @brief Process OSD information
 * @{
 *
 * @file       osdgen.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

// ****************

// #define DEBUG_TIMING
// #define DEBUG_ALARMS
// #define DEBUG_TELEMETRY
// #define DEBUG_BLACK_WHITE
// #define DEBUG_STUFF
// #define SIMULATE_DATA
#define TEMP_GPS_STATUS_WORKAROUND

#include <openpilot.h>

#include "osdgen.h"

#include "stm32f4xx_dac.h"

#include "attitudestate.h"
#include "gpspositionsensor.h"
#include "homelocation.h"
#include "gpstime.h"
#include "gpssatellites.h"
#include "osdsettings.h"
#include "osdsettings2.h"
#include "barosensor.h"
#include "taskinfo.h"
#include "flightstatus.h"
#include "manualcontrolcommand.h"
#include "gpsvelocitysensor.h"
#ifdef DEBUG_TELEMETRY
#include "flighttelemetrystats.h"
#include "gcstelemetrystats.h"
#endif

#include "fonts.h"
#include "font12x18.h"
#include "font8x10.h"
#include "WMMInternal.h"

#include "splash.h"

extern uint8_t PIOS_Board_Revision(void);

extern uint8_t *draw_buffer_level;
extern uint8_t *draw_buffer_mask;
extern uint8_t *disp_buffer_level;
extern uint8_t *disp_buffer_mask;

// ****************
// Private functions
static void osdgenTask(void *parameters);

// ****************
// Private constants
#define LONG_TIME        0xffff
#define STACK_SIZE_BYTES 4096
#define TASK_PRIORITY    (tskIDLE_PRIORITY + 4)
#define UPDATE_PERIOD    100

// ****************
// Private variables
static xTaskHandle osdgenTaskHandle;
xSemaphoreHandle osdSemaphore = NULL;
TTime timex;
Unit Convert[2];

#ifdef DEBUG_TIMING
static portTickType in_ticks  = 0;
static portTickType out_ticks = 0;
static uint16_t in_time  = 0;
static uint16_t out_time = 0;
#endif

struct splashEntry {
    unsigned int   width, height;
    const uint16_t *level;
    const uint16_t *mask;
};

struct splashEntry splash[3] = {
    { oplogo_width,
      oplogo_height,
      oplogo_bits,
      oplogo_mask_bits },
    { level_width,
      level_height,
      level_bits,
      level_mask_bits },
    { llama_width,
      llama_height,
      llama_bits,
      llama_mask_bits }
};

uint16_t mirror(uint16_t source)
{
    int result = ((source & 0x8000) >> 7) | ((source & 0x4000) >> 5) | ((source & 0x2000) >> 3) | ((source & 0x1000) >> 1) | ((source & 0x0800) << 1)
                 | ((source & 0x0400) << 3) | ((source & 0x0200) << 5) | ((source & 0x0100) << 7) | ((source & 0x0080) >> 7) | ((source & 0x0040) >> 5)
                 | ((source & 0x0020) >> 3) | ((source & 0x0010) >> 1) | ((source & 0x0008) << 1) | ((source & 0x0004) << 3) | ((source & 0x0002) << 5)
                 | ((source & 0x0001) << 7);

    return result;
}

void clearGraphics()
{
    memset((uint8_t *)draw_buffer_mask, 0, BUFFER_HEIGHT * BUFFER_WIDTH);
    memset((uint8_t *)draw_buffer_level, 0, BUFFER_HEIGHT * BUFFER_WIDTH);
}

void copyimage(uint16_t offsetx, uint16_t offsety, int image)
{
    // check top/left position
    if (!validPos(offsetx, offsety)) {
        return;
    }
    struct splashEntry splash_info;
    splash_info = splash[image];
    offsetx     = offsetx / 8;
    for (uint16_t y = offsety; y < ((splash_info.height) + offsety); y++) {
        uint16_t x1 = offsetx;
        for (uint16_t x = offsetx; x < (((splash_info.width) / 16) + offsetx); x++) {
            draw_buffer_level[y * BUFFER_WIDTH + x1 + 1] = (uint8_t)(
                mirror(splash_info.level[(y - offsety) * ((splash_info.width) / 16) + (x - offsetx)]) >> 8);
            draw_buffer_level[y * BUFFER_WIDTH + x1]     = (uint8_t)(
                mirror(splash_info.level[(y - offsety) * ((splash_info.width) / 16) + (x - offsetx)]) & 0xFF);
            draw_buffer_mask[y * BUFFER_WIDTH + x1 + 1]  = (uint8_t)(
                mirror(splash_info.mask[(y - offsety) * ((splash_info.width) / 16) + (x - offsetx)]) >> 8);
            draw_buffer_mask[y * BUFFER_WIDTH + x1] = (uint8_t)(mirror(splash_info.mask[(y - offsety) * ((splash_info.width) / 16) + (x - offsetx)]) & 0xFF);
            x1 += 2;
        }
    }
}

uint8_t validPos(uint16_t x, uint16_t y)
{
    if (x >= GRAPHICS_WIDTH_REAL || y >= GRAPHICS_HEIGHT_REAL) {
        return 0;
    }
    return 1;
}

// Credit for this one goes to wikipedia! :-)
void drawCircle(uint16_t x0, uint16_t y0, uint16_t radius)
{
    int f     = 1 - radius;
    int ddF_x = 1;
    int ddF_y = -2 * radius;
    int x     = 0;
    int y     = radius;

    write_pixel_lm(x0, y0 + radius, 1, 1);
    write_pixel_lm(x0, y0 - radius, 1, 1);
    write_pixel_lm(x0 + radius, y0, 1, 1);
    write_pixel_lm(x0 - radius, y0, 1, 1);

    while (x < y) {
        // ddF_x == 2 * x + 1;
        // ddF_y == -2 * y;
        // f == x*x + y*y - radius*radius + 2*x - y + 1;
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        write_pixel_lm(x0 + x, y0 + y, 1, 1);
        write_pixel_lm(x0 - x, y0 + y, 1, 1);
        write_pixel_lm(x0 + x, y0 - y, 1, 1);
        write_pixel_lm(x0 - x, y0 - y, 1, 1);
        write_pixel_lm(x0 + y, y0 + x, 1, 1);
        write_pixel_lm(x0 - y, y0 + x, 1, 1);
        write_pixel_lm(x0 + y, y0 - x, 1, 1);
        write_pixel_lm(x0 - y, y0 - x, 1, 1);
    }
}

void swap(uint16_t *a, uint16_t *b)
{
    uint16_t temp = *a;

    *a = *b;
    *b = temp;
}

static const int8_t sinData[91] =
{ 0,  2,  3,  5,  7,  9,  10,  12,  14,  16,  17,  19, 21, 22, 24, 26, 28, 29, 31, 33, 34, 36, 37, 39, 41, 42, 44, 45, 47, 48, 50, 52, 53, 54, 56, 57, 59, 60, 62, 63, 64,
  66, 67, 68, 69, 71, 72, 73,  74,  75,  77,  78,  79, 80, 81, 82, 83, 84, 85, 86, 87, 87, 88, 89, 90, 91, 91, 92, 93, 93, 94, 95, 95, 96, 96, 97, 97, 97, 98,
  98, 98, 99, 99, 99, 99, 100, 100, 100, 100, 100, 100 };

static int8_t mySin(uint16_t angle)
{
    uint16_t pos = 0;

    pos = angle % 360;
    int8_t mult  = 1;
    // 180-359 is same as 0-179 but negative.
    if (pos >= 180) {
        pos  = pos - 180;
        mult = -1;
    }
    // 0-89 is equal to 90-179 except backwards.
    if (pos >= 90) {
        pos = 180 - pos;
    }
    return mult * (int8_t)(sinData[pos]);
}

static int8_t myCos(uint16_t angle)
{
    return mySin(angle + 90);
}

/// Draws four points relative to the given center point.
///
/// \li centerX + X, centerY + Y
/// \li centerX + X, centerY - Y
/// \li centerX - X, centerY + Y
/// \li centerX - X, centerY - Y
///
/// \param centerX the x coordinate of the center point
/// \param centerY the y coordinate of the center point
/// \param deltaX the difference between the centerX coordinate and each pixel drawn
/// \param deltaY the difference between the centerY coordinate and each pixel drawn
/// \param color the color to draw the pixels with.
void plotFourQuadrants(int32_t centerX, int32_t centerY, int32_t deltaX, int32_t deltaY)
{
    write_pixel_lm(centerX + deltaX, centerY + deltaY, 1, 1); // Ist      Quadrant
    write_pixel_lm(centerX - deltaX, centerY + deltaY, 1, 1); // IInd     Quadrant
    write_pixel_lm(centerX - deltaX, centerY - deltaY, 1, 1); // IIIrd    Quadrant
    write_pixel_lm(centerX + deltaX, centerY - deltaY, 1, 1); // IVth     Quadrant
}

/// Implements the midpoint ellipse drawing algorithm which is a bresenham
/// style DDF.
///
/// \param centerX the x coordinate of the center of the ellipse
/// \param centerY the y coordinate of the center of the ellipse
/// \param horizontalRadius the horizontal radius of the ellipse
/// \param verticalRadius the vertical radius of the ellipse
/// \param color the color of the ellipse border
void ellipse(int centerX, int centerY, int horizontalRadius, int verticalRadius)
{
    int64_t doubleHorizontalRadius = horizontalRadius * horizontalRadius;
    int64_t doubleVerticalRadius   = verticalRadius * verticalRadius;

    int64_t error = doubleVerticalRadius - doubleHorizontalRadius * verticalRadius + (doubleVerticalRadius >> 2);

    int x = 0;
    int y = verticalRadius;
    int deltaX = 0;
    int deltaY = (doubleHorizontalRadius << 1) * y;

    plotFourQuadrants(centerX, centerY, x, y);

    while (deltaY >= deltaX) {
        x++;
        deltaX += (doubleVerticalRadius << 1);

        error  += deltaX + doubleVerticalRadius;

        if (error >= 0) {
            y--;
            deltaY -= (doubleHorizontalRadius << 1);

            error  -= deltaY;
        }
        plotFourQuadrants(centerX, centerY, x, y);
    }

    error = (int64_t)(doubleVerticalRadius * (x + 1 / 2.0f) * (x + 1 / 2.0f) + doubleHorizontalRadius * (y - 1) * (y - 1) - doubleHorizontalRadius * doubleVerticalRadius);

    while (y >= 0) {
        error  += doubleHorizontalRadius;
        y--;
        deltaY -= (doubleHorizontalRadius << 1);
        error  -= deltaY;

        if (error <= 0) {
            x++;
            deltaX += (doubleVerticalRadius << 1);
            error  += deltaX;
        }

        plotFourQuadrants(centerX, centerY, x, y);
    }
}

void drawArrow(uint16_t x, uint16_t y, uint16_t angle, uint16_t size_quarter)
{
    float sin_angle = sinf(DEG2RAD(angle));
    float cos_angle = cosf(DEG2RAD(angle));
    int16_t peak_x  = (int16_t)(sin_angle * size_quarter * 2);
    int16_t peak_y  = (int16_t)(cos_angle * size_quarter * 2);
    int16_t d_end_x = (int16_t)(cos_angle * size_quarter);
    int16_t d_end_y = (int16_t)(sin_angle * size_quarter);

    write_line_lm(x + peak_x, y - peak_y, x - peak_x - d_end_x, y + peak_y - d_end_y, 1, 1);
    write_line_lm(x + peak_x, y - peak_y, x - peak_x + d_end_x, y + peak_y + d_end_y, 1, 1);
    write_line_lm(x, y, x - peak_x - d_end_x, y + peak_y - d_end_y, 1, 1);
    write_line_lm(x, y, x - peak_x + d_end_x, y + peak_y + d_end_y, 1, 1);
}

void drawBox(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    write_line_lm(x1, y1, x2, y1, 1, 1); // top
    write_line_lm(x1, y1, x1, y2, 1, 1); // left
    write_line_lm(x2, y1, x2, y2, 1, 1); // right
    write_line_lm(x1, y2, x2, y2, 1, 1); // bottom
}

/**
 * write_pixel: Write a pixel at an x,y position to a given surface.
 *
 * @param       buff    pointer to buffer to write in
 * @param       x               x coordinate
 * @param       y               y coordinate
 * @param       mode    0 = clear bit, 1 = set bit, 2 = toggle bit
 */
void write_pixel(uint8_t *buff, unsigned int x, unsigned int y, int mode)
{
    CHECK_COORDS(x, y);
    // Determine the bit in the word to be set and the word
    // index to set it in.
    int bitnum    = CALC_BIT_IN_WORD(x);
    int wordnum   = CALC_BUFF_ADDR(x, y);
    // Apply a mask.
    uint16_t mask = 1 << (7 - bitnum);
    WRITE_WORD_MODE(buff, wordnum, mask, mode);
}

/**
 * write_pixel_lm: write the pixel on both surfaces (level and mask.)
 * Uses current draw buffer.
 *
 * @param       x               x coordinate
 * @param       y               y coordinate
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 * @param       lmode   0 = black, 1 = white, 2 = toggle
 */
void write_pixel_lm(unsigned int x, unsigned int y, int mmode, int lmode)
{
    CHECK_COORDS(x, y);
    // Determine the bit in the word to be set and the word
    // index to set it in.
    int bitnum    = CALC_BIT_IN_WORD(x);
    int wordnum   = CALC_BUFF_ADDR(x, y);
    // Apply the masks.
    uint16_t mask = 1 << (7 - bitnum);
    WRITE_WORD_MODE(draw_buffer_mask, wordnum, mask, mmode);
    WRITE_WORD_MODE(draw_buffer_level, wordnum, mask, lmode);
}

/**
 * write_pixel_lm_truncated: write the pixel on both surfaces (level and mask.)
 * Uses current draw buffer.
 *
 * @param       x       x coordinate
 * @param       y       y coordinate
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 * @param       lmode   0 = black, 1 = white, 2 = toggle
 */
void write_pixel_lm_truncated(int x, int y, int mmode, int lmode)
{
    CHECK_COORDS_TRUNCATED(x, y);
    // Determine the bit in the word to be set and the word
    // index to set it in.
    int bitnum    = CALC_BIT_IN_WORD(x);
    int wordnum   = CALC_BUFF_ADDR(x, y);
    // Apply the masks.
    uint16_t mask = 1 << (7 - bitnum);
    WRITE_WORD_MODE(draw_buffer_mask, wordnum, mask, mmode);
    WRITE_WORD_MODE(draw_buffer_level, wordnum, mask, lmode);
}

/**
 * write_hline: optimised horizontal line writing algorithm
 *
 * @param       buff    pointer to buffer to write in
 * @param       x0              x0 coordinate
 * @param       x1              x1 coordinate
 * @param       y               y coordinate
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
void write_hline(uint8_t *buff, unsigned int x0, unsigned int x1, unsigned int y, int mode)
{
    CLIP_COORDS(x0, y);
    CLIP_COORDS(x1, y);
    if (x0 > x1) {
        SWAP(x0, x1);
    }
    if (x0 == x1) {
        return;
    }
    /* This is an optimised algorithm for writing horizontal lines.
    * We begin by finding the addresses of the x0 and x1 points. */
    int addr0     = CALC_BUFF_ADDR(x0, y);
    int addr1     = CALC_BUFF_ADDR(x1, y);
    int addr0_bit = CALC_BIT_IN_WORD(x0);
    int addr1_bit = CALC_BIT_IN_WORD(x1);
    int mask, mask_l, mask_r, i;
    /* If the addresses are equal, we only need to write one word
     * which is an island. */
    if (addr0 == addr1) {
        mask = COMPUTE_HLINE_ISLAND_MASK(addr0_bit, addr1_bit);
        WRITE_WORD_MODE(buff, addr0, mask, mode);
    } else {
        /* Otherwise we need to write the edges and then the middle. */
        mask_l = COMPUTE_HLINE_EDGE_L_MASK(addr0_bit);
        mask_r = COMPUTE_HLINE_EDGE_R_MASK(addr1_bit);
        WRITE_WORD_MODE(buff, addr0, mask_l, mode);
        WRITE_WORD_MODE(buff, addr1, mask_r, mode);
        // Now write 0xffff words from start+1 to end-1.
        for (i = addr0 + 1; i <= addr1 - 1; i++) {
            uint8_t m = 0xff;
            WRITE_WORD_MODE(buff, i, m, mode);
        }
    }
}

/**
 * write_hline_lm: write both level and mask buffers.
 *
 * @param       x0              x0 coordinate
 * @param       x1              x1 coordinate
 * @param       y               y coordinate
 * @param       lmode   0 = clear, 1 = set, 2 = toggle
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 */
void write_hline_lm(unsigned int x0, unsigned int x1, unsigned int y, int lmode, int mmode)
{
    // TODO: an optimisation would compute the masks and apply to
    // both buffers simultaneously.
    write_hline(draw_buffer_level, x0, x1, y, lmode);
    write_hline(draw_buffer_mask, x0, x1, y, mmode);
}

/**
 * write_hline_outlined: outlined horizontal line with varying endcaps
 * Always uses draw buffer.
 *
 * @param       x0                      x0 coordinate
 * @param       x1                      x1 coordinate
 * @param       y                       y coordinate
 * @param       endcap0         0 = none, 1 = single pixel, 2 = full cap
 * @param       endcap1         0 = none, 1 = single pixel, 2 = full cap
 * @param       mode            0 = black outline, white body, 1 = white outline, black body
 * @param       mmode           0 = clear, 1 = set, 2 = toggle
 */
void write_hline_outlined(unsigned int x0, unsigned int x1, unsigned int y, int endcap0, int endcap1, int mode, int mmode)
{
    int stroke, fill;

    SETUP_STROKE_FILL(stroke, fill, mode);
    if (x0 > x1) {
        SWAP(x0, x1);
    }
    // Draw the main body of the line.
    write_hline_lm(x0 + 1, x1 - 1, y - 1, stroke, mmode);
    write_hline_lm(x0 + 1, x1 - 1, y + 1, stroke, mmode);
    write_hline_lm(x0 + 1, x1 - 1, y, fill, mmode);
    // Draw the endcaps, if any.
    DRAW_ENDCAP_HLINE(endcap0, x0, y, stroke, fill, mmode);
    DRAW_ENDCAP_HLINE(endcap1, x1, y, stroke, fill, mmode);
}

/**
 * write_vline: optimised vertical line writing algorithm
 *
 * @param       buff    pointer to buffer to write in
 * @param       x               x coordinate
 * @param       y0              y0 coordinate
 * @param       y1              y1 coordinate
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
void write_vline(uint8_t *buff, unsigned int x, unsigned int y0, unsigned int y1, int mode)
{
    unsigned int a;

    CLIP_COORDS(x, y0);
    CLIP_COORDS(x, y1);
    if (y0 > y1) {
        SWAP(y0, y1);
    }
    if (y0 == y1) {
        return;
    }
    /* This is an optimised algorithm for writing vertical lines.
     * We begin by finding the addresses of the x,y0 and x,y1 points. */
    unsigned int addr0  = CALC_BUFF_ADDR(x, y0);
    unsigned int addr1  = CALC_BUFF_ADDR(x, y1);
    /* Then we calculate the pixel data to be written. */
    unsigned int bitnum = CALC_BIT_IN_WORD(x);
    uint16_t mask = 1 << (7 - bitnum);
    /* Run from addr0 to addr1 placing pixels. Increment by the number
     * of words n each graphics line. */
    for (a = addr0; a <= addr1; a += BUFFER_WIDTH) {
        WRITE_WORD_MODE(buff, a, mask, mode);
    }
}

/**
 * write_vline_lm: write both level and mask buffers.
 *
 * @param       x               x coordinate
 * @param       y0              y0 coordinate
 * @param       y1              y1 coordinate
 * @param       lmode   0 = clear, 1 = set, 2 = toggle
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 */
void write_vline_lm(unsigned int x, unsigned int y0, unsigned int y1, int lmode, int mmode)
{
    // TODO: an optimisation would compute the masks and apply to
    // both buffers simultaneously.
    write_vline(draw_buffer_level, x, y0, y1, lmode);
    write_vline(draw_buffer_mask, x, y0, y1, mmode);
}

/**
 * write_vline_outlined: outlined vertical line with varying endcaps
 * Always uses draw buffer.
 *
 * @param       x                       x coordinate
 * @param       y0                      y0 coordinate
 * @param       y1                      y1 coordinate
 * @param       endcap0         0 = none, 1 = single pixel, 2 = full cap
 * @param       endcap1         0 = none, 1 = single pixel, 2 = full cap
 * @param       mode            0 = black outline, white body, 1 = white outline, black body
 * @param       mmode           0 = clear, 1 = set, 2 = toggle
 */
void write_vline_outlined(unsigned int x, unsigned int y0, unsigned int y1, int endcap0, int endcap1, int mode, int mmode)
{
    int stroke, fill;

    if (y0 > y1) {
        SWAP(y0, y1);
    }
    SETUP_STROKE_FILL(stroke, fill, mode);
    // Draw the main body of the line.
    write_vline_lm(x - 1, y0 + 1, y1 - 1, stroke, mmode);
    write_vline_lm(x + 1, y0 + 1, y1 - 1, stroke, mmode);
    write_vline_lm(x, y0 + 1, y1 - 1, fill, mmode);
    // Draw the endcaps, if any.
    DRAW_ENDCAP_VLINE(endcap0, x, y0, stroke, fill, mmode);
    DRAW_ENDCAP_VLINE(endcap1, x, y1, stroke, fill, mmode);
}

/**
 * write_filled_rectangle: draw a filled rectangle.
 *
 * Uses an optimised algorithm which is similar to the horizontal
 * line writing algorithm, but optimised for writing the lines
 * multiple times without recalculating lots of stuff.
 *
 * @param       buff    pointer to buffer to write in
 * @param       x               x coordinate (left)
 * @param       y               y coordinate (top)
 * @param       width   rectangle width
 * @param       height  rectangle height
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
void write_filled_rectangle(uint8_t *buff, unsigned int x, unsigned int y, unsigned int width, unsigned int height, int mode)
{
    unsigned int yy, addr0_old, addr1_old;

    CHECK_COORDS(x, y);
    CHECK_COORD_X(x + width);
    CHECK_COORD_Y(y + height);
    if (width <= 0 || height <= 0) {
        return;
    }
    // Calculate as if the rectangle was only a horizontal line. We then
    // step these addresses through each row until we iterate `height` times.
    unsigned int addr0     = CALC_BUFF_ADDR(x, y);
    unsigned int addr1     = CALC_BUFF_ADDR(x + width, y);
    unsigned int addr0_bit = CALC_BIT_IN_WORD(x);
    unsigned int addr1_bit = CALC_BIT_IN_WORD(x + width);
    unsigned int mask, mask_l, mask_r, i;
    // If the addresses are equal, we need to write one word vertically.
    if (addr0 == addr1) {
        mask = COMPUTE_HLINE_ISLAND_MASK(addr0_bit, addr1_bit);
        while (height--) {
            WRITE_WORD_MODE(buff, addr0, mask, mode);
            addr0 += BUFFER_WIDTH;
        }
    } else {
        // Otherwise we need to write the edges and then the middle repeatedly.
        mask_l    = COMPUTE_HLINE_EDGE_L_MASK(addr0_bit);
        mask_r    = COMPUTE_HLINE_EDGE_R_MASK(addr1_bit);
        // Write edges first.
        yy        = 0;
        addr0_old = addr0;
        addr1_old = addr1;
        while (yy < height) {
            WRITE_WORD_MODE(buff, addr0, mask_l, mode);
            WRITE_WORD_MODE(buff, addr1, mask_r, mode);
            addr0 += BUFFER_WIDTH;
            addr1 += BUFFER_WIDTH;
            yy++;
        }
        // Now write 0xffff words from start+1 to end-1 for each row.
        yy    = 0;
        addr0 = addr0_old;
        addr1 = addr1_old;
        while (yy < height) {
            for (i = addr0 + 1; i <= addr1 - 1; i++) {
                uint8_t m = 0xff;
                WRITE_WORD_MODE(buff, i, m, mode);
            }
            addr0 += BUFFER_WIDTH;
            addr1 += BUFFER_WIDTH;
            yy++;
        }
    }
}

/**
 * write_filled_rectangle_lm: draw a filled rectangle on both draw buffers.
 *
 * @param       x               x coordinate (left)
 * @param       y               y coordinate (top)
 * @param       width   rectangle width
 * @param       height  rectangle height
 * @param       lmode   0 = clear, 1 = set, 2 = toggle
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 */
void write_filled_rectangle_lm(unsigned int x, unsigned int y, unsigned int width, unsigned int height, int lmode, int mmode)
{
    write_filled_rectangle(draw_buffer_mask, x, y, width, height, mmode);
    write_filled_rectangle(draw_buffer_level, x, y, width, height, lmode);
}

/**
 * write_rectangle_outlined: draw an outline of a rectangle. Essentially
 * a convenience wrapper for draw_hline_outlined and draw_vline_outlined.
 *
 * @param       x               x coordinate (left)
 * @param       y               y coordinate (top)
 * @param       width   rectangle width
 * @param       height  rectangle height
 * @param       mode    0 = black outline, white body, 1 = white outline, black body
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 */
void write_rectangle_outlined(unsigned int x, unsigned int y, int width, int height, int mode, int mmode)
{
    // CHECK_COORDS(x, y);
    // CHECK_COORDS(x + width, y + height);
    // if((x + width) > DISP_WIDTH) width = DISP_WIDTH - x;
    // if((y + height) > DISP_HEIGHT) height = DISP_HEIGHT - y;
    write_hline_outlined(x, x + width, y, ENDCAP_ROUND, ENDCAP_ROUND, mode, mmode);
    write_hline_outlined(x, x + width, y + height, ENDCAP_ROUND, ENDCAP_ROUND, mode, mmode);
    write_vline_outlined(x, y, y + height, ENDCAP_ROUND, ENDCAP_ROUND, mode, mmode);
    write_vline_outlined(x + width, y, y + height, ENDCAP_ROUND, ENDCAP_ROUND, mode, mmode);
}

/**
 * write_circle: draw the outline of a circle on a given buffer,
 * with an optional dash pattern for the line instead of a normal line.
 *
 * @param       buff    pointer to buffer to write in
 * @param       cx              origin x coordinate
 * @param       cy              origin y coordinate
 * @param       r               radius
 * @param       dashp   dash period (pixels) - zero for no dash
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
void write_circle(uint8_t *buff, unsigned int cx, unsigned int cy, unsigned int r, unsigned int dashp, int mode)
{
    CHECK_COORDS(cx, cy);
    int error = -r, x = r, y = 0;
    while (x >= y) {
        if (dashp == 0 || (y % dashp) < (dashp / 2)) {
            CIRCLE_PLOT_8(buff, cx, cy, x, y, mode);
        }
        error += (y * 2) + 1;
        y++;
        if (error >= 0) {
            --x;
            error -= x * 2;
        }
    }
}

/**
 * write_circle_outlined: draw an outlined circle on the draw buffer.
 *
 * @param       cx              origin x coordinate
 * @param       cy              origin y coordinate
 * @param       r               radius
 * @param       dashp   dash period (pixels) - zero for no dash
 * @param       bmode   0 = 4-neighbour border, 1 = 8-neighbour border
 * @param       mode    0 = black outline, white body, 1 = white outline, black body
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 */
void write_circle_outlined(unsigned int cx, unsigned int cy, unsigned int r, unsigned int dashp, int bmode, int mode, int mmode)
{
    int stroke, fill;

    CHECK_COORDS(cx, cy);
    SETUP_STROKE_FILL(stroke, fill, mode);
    // This is a two step procedure. First, we draw the outline of the
    // circle, then we draw the inner part.
    int error = -r, x = r, y = 0;
    while (x >= y) {
        if (dashp == 0 || (y % dashp) < (dashp / 2)) {
            CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x + 1, y, mmode);
            CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x + 1, y, stroke);
            CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x, y + 1, mmode);
            CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x, y + 1, stroke);
            CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x - 1, y, mmode);
            CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x - 1, y, stroke);
            CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x, y - 1, mmode);
            CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x, y - 1, stroke);
            if (bmode == 1) {
                CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x + 1, y + 1, mmode);
                CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x + 1, y + 1, stroke);
                CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x - 1, y - 1, mmode);
                CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x - 1, y - 1, stroke);
            }
        }
        error += (y * 2) + 1;
        y++;
        if (error >= 0) {
            --x;
            error -= x * 2;
        }
    }
    error = -r;
    x     = r;
    y     = 0;
    while (x >= y) {
        if (dashp == 0 || (y % dashp) < (dashp / 2)) {
            CIRCLE_PLOT_8(draw_buffer_mask, cx, cy, x, y, mmode);
            CIRCLE_PLOT_8(draw_buffer_level, cx, cy, x, y, fill);
        }
        error += (y * 2) + 1;
        y++;
        if (error >= 0) {
            --x;
            error -= x * 2;
        }
    }
}

/**
 * write_circle_filled: fill a circle on a given buffer.
 *
 * @param       buff    pointer to buffer to write in
 * @param       cx              origin x coordinate
 * @param       cy              origin y coordinate
 * @param       r               radius
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
void write_circle_filled(uint8_t *buff, unsigned int cx, unsigned int cy, unsigned int r, int mode)
{
    CHECK_COORDS(cx, cy);
    int error = -r, x = r, y = 0, xch = 0;
    // It turns out that filled circles can take advantage of the midpoint
    // circle algorithm. We simply draw very fast horizontal lines across each
    // pair of X,Y coordinates. In some cases, this can even be faster than
    // drawing an outlined circle!
    //
    // Due to multiple writes to each set of pixels, we have a special exception
    // for when using the toggling draw mode.
    while (x >= y) {
        if (y != 0) {
            write_hline(buff, cx - x, cx + x, cy + y, mode);
            write_hline(buff, cx - x, cx + x, cy - y, mode);
            if (mode != 2 || (mode == 2 && xch && (cx - x) != (cx - y))) {
                write_hline(buff, cx - y, cx + y, cy + x, mode);
                write_hline(buff, cx - y, cx + y, cy - x, mode);
                xch = 0;
            }
        }
        error += (y * 2) + 1;
        y++;
        if (error >= 0) {
            --x;
            xch    = 1;
            error -= x * 2;
        }
    }
    // Handle toggle mode.
    if (mode == 2) {
        write_hline(buff, cx - r, cx + r, cy, mode);
    }
}

/**
 * write_line: Draw a line of arbitrary angle.
 *
 * @param       buff    pointer to buffer to write in
 * @param       x0              first x coordinate
 * @param       y0              first y coordinate
 * @param       x1              second x coordinate
 * @param       y1              second y coordinate
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
void write_line(uint8_t *buff, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, int mode)
{
    // Based on http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    unsigned int steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep) {
        SWAP(x0, y0);
        SWAP(x1, y1);
    }
    if (x0 > x1) {
        SWAP(x0, x1);
        SWAP(y0, y1);
    }
    int deltax     = x1 - x0;
    unsigned int deltay = abs(y1 - y0);
    int error      = deltax / 2;
    int ystep;
    unsigned int y = y0;
    unsigned int x; // , lasty = y, stox = 0;
    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }
    for (x = x0; x < x1; x++) {
        if (steep) {
            write_pixel(buff, y, x, mode);
        } else {
            write_pixel(buff, x, y, mode);
        }
        error -= deltay;
        if (error < 0) {
            y     += ystep;
            error += deltax;
        }
    }
}

/**
 * write_line_lm: Draw a line of arbitrary angle.
 *
 * @param       x0              first x coordinate
 * @param       y0              first y coordinate
 * @param       x1              second x coordinate
 * @param       y1              second y coordinate
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 * @param       lmode   0 = clear, 1 = set, 2 = toggle
 */
void write_line_lm(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, int mmode, int lmode)
{
    write_line(draw_buffer_mask, x0, y0, x1, y1, mmode);
    write_line(draw_buffer_level, x0, y0, x1, y1, lmode);
}

/**
 * write_line_outlined: Draw a line of arbitrary angle, with an outline.
 *
 * @param       x0                      first x coordinate
 * @param       y0                      first y coordinate
 * @param       x1                      second x coordinate
 * @param       y1                      second y coordinate
 * @param       endcap0         0 = none, 1 = single pixel, 2 = full cap
 * @param       endcap1         0 = none, 1 = single pixel, 2 = full cap
 * @param       mode            0 = black outline, white body, 1 = white outline, black body
 * @param       mmode           0 = clear, 1 = set, 2 = toggle
 */
void write_line_outlined(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1,
                         __attribute__((unused)) int endcap0, __attribute__((unused)) int endcap1,
                         int mode, int mmode)
{
    // Based on http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    // This could be improved for speed.
    int omode, imode;

    if (mode == 0) {
        omode = 0;
        imode = 1;
    } else {
        omode = 1;
        imode = 0;
    }
    int steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        SWAP(x0, y0);
        SWAP(x1, y1);
    }
    if (x0 > x1) {
        SWAP(x0, x1);
        SWAP(y0, y1);
    }
    int deltax     = x1 - x0;
    unsigned int deltay = abs(y1 - y0);
    int error      = deltax / 2;
    int ystep;
    unsigned int y = y0;
    unsigned int x;
    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }
    // Draw the outline.
    for (x = x0; x < x1; x++) {
        if (steep) {
            write_pixel_lm(y - 1, x, mmode, omode);
            write_pixel_lm(y + 1, x, mmode, omode);
            write_pixel_lm(y, x - 1, mmode, omode);
            write_pixel_lm(y, x + 1, mmode, omode);
        } else {
            write_pixel_lm(x - 1, y, mmode, omode);
            write_pixel_lm(x + 1, y, mmode, omode);
            write_pixel_lm(x, y - 1, mmode, omode);
            write_pixel_lm(x, y + 1, mmode, omode);
        }
        error -= deltay;
        if (error < 0) {
            y     += ystep;
            error += deltax;
        }
    }
    // Now draw the innards.
    error = deltax / 2;
    y     = y0;
    for (x = x0; x < x1; x++) {
        if (steep) {
            write_pixel_lm(y, x, mmode, imode);
        } else {
            write_pixel_lm(x, y, mmode, imode);
        }
        error -= deltay;
        if (error < 0) {
            y     += ystep;
            error += deltax;
        }
    }
}

/**
 * write_line_outlined_dashed_truncated: Draw a line of arbitrary angle, with an outline, potentially dashed and truncated when out of field.
 *
 * @param       x0              first x coordinate
 * @param       y0              first y coordinate
 * @param       x1              second x coordinate
 * @param       y1              second y coordinate
 * @param       endcap0         0 = none, 1 = single pixel, 2 = full cap
 * @param       endcap1         0 = none, 1 = single pixel, 2 = full cap
 * @param       mode            0 = black outline, white body, 1 = white outline, black body
 * @param       mmode           0 = clear, 1 = set, 2 = toggle
 * @param       dots			0 = not dashed, > 0 = # of set/unset dots for the dashed innards
 */
void write_line_outlined_dashed_truncated(int x0, int y0, int x1, int y1,
                                          __attribute__((unused)) int endcap0, __attribute__((unused)) int endcap1,
                                          int mode, int mmode, int dots)
{
    // Based on http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    // This could be improved for speed.
    int omode, imode;

    if (mode == 0) {
        omode = 0;
        imode = 1;
    } else {
        omode = 1;
        imode = 0;
    }
    int steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        SWAP(x0, y0);
        SWAP(x1, y1);
    }
    if (x0 > x1) {
        SWAP(x0, x1);
        SWAP(y0, y1);
    }
    int deltax = x1 - x0;
    unsigned int deltay = abs(y1 - y0);
    int error  = deltax / 2;
    int ystep;
    int y = y0;
    int x;
    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }
    // Draw the outline.
    for (x = x0; x < x1; x++) {
        if (steep) {
            write_pixel_lm_truncated(y - 1, x, mmode, omode);
            write_pixel_lm_truncated(y + 1, x, mmode, omode);
            write_pixel_lm_truncated(y, x - 1, mmode, omode);
            write_pixel_lm_truncated(y, x + 1, mmode, omode);
        } else {
            write_pixel_lm_truncated(x - 1, y, mmode, omode);
            write_pixel_lm_truncated(x + 1, y, mmode, omode);
            write_pixel_lm_truncated(x, y - 1, mmode, omode);
            write_pixel_lm_truncated(x, y + 1, mmode, omode);
        }
        error -= deltay;
        if (error < 0) {
            y     += ystep;
            error += deltax;
        }
    }
    // Now draw the innards.
    error = deltax / 2;
    y     = y0;
    int dot_cnt = 0;
    int draw    = 1;
    for (x = x0; x < x1; x++) {
        if (dots && !(dot_cnt++ % dots)) {
            draw++;
        }
        if (draw % 2) {
            if (steep) {
                write_pixel_lm_truncated(y, x, mmode, imode);
            } else {
                write_pixel_lm_truncated(x, y, mmode, imode);
            }
        }
        error -= deltay;
        if (error < 0) {
            y     += ystep;
            error += deltax;
        }
    }
}

/**
 * write_word_misaligned: Write a misaligned word across two addresses
 * with an x offset.
 *
 * This allows for many pixels to be set in one write.
 *
 * @param       buff    buffer to write in
 * @param       word    word to write (16 bits)
 * @param       addr    address of first word
 * @param       xoff    x offset (0-15)
 * @param       mode    0 = clear, 1 = set, 2 = toggle
 */
void write_word_misaligned(uint8_t *buff, uint16_t word, unsigned int addr, unsigned int xoff, int mode)
{
    int16_t firstmask = word >> xoff;
    int16_t lastmask  = word << (16 - xoff);

    WRITE_WORD_MODE(buff, addr + 1, firstmask && 0x00ff, mode);
    WRITE_WORD_MODE(buff, addr, (firstmask & 0xff00) >> 8, mode);
    if (xoff > 0) {
        WRITE_WORD_MODE(buff, addr + 2, (lastmask & 0xff00) >> 8, mode);
    }
}

/**
 * write_word_misaligned_NAND: Write a misaligned word across two addresses
 * with an x offset, using a NAND mask.
 *
 * This allows for many pixels to be set in one write.
 *
 * @param       buff    buffer to write in
 * @param       word    word to write (16 bits)
 * @param       addr    address of first word
 * @param       xoff    x offset (0-15)
 *
 * This is identical to calling write_word_misaligned with a mode of 0 but
 * it doesn't go through a lot of switch logic which slows down text writing
 * a lot.
 */
void write_word_misaligned_NAND(uint8_t *buff, uint16_t word, unsigned int addr, unsigned int xoff)
{
    uint16_t firstmask = word >> xoff;
    uint16_t lastmask  = word << (16 - xoff);

    WRITE_WORD_NAND(buff, addr + 1, firstmask & 0x00ff);
    WRITE_WORD_NAND(buff, addr, (firstmask & 0xff00) >> 8);
    if (xoff > 0) {
        WRITE_WORD_NAND(buff, addr + 2, (lastmask & 0xff00) >> 8);
    }
}

/**
 * write_word_misaligned_OR: Write a misaligned word across two addresses
 * with an x offset, using an OR mask.
 *
 * This allows for many pixels to be set in one write.
 *
 * @param       buff    buffer to write in
 * @param       word    word to write (16 bits)
 * @param       addr    address of first word
 * @param       xoff    x offset (0-15)
 *
 * This is identical to calling write_word_misaligned with a mode of 1 but
 * it doesn't go through a lot of switch logic which slows down text writing
 * a lot.
 */
void write_word_misaligned_OR(uint8_t *buff, uint16_t word, unsigned int addr, unsigned int xoff)
{
    uint16_t firstmask = word >> xoff;
    uint16_t lastmask  = word << (16 - xoff);

    WRITE_WORD_OR(buff, addr + 1, firstmask & 0x00ff);
    WRITE_WORD_OR(buff, addr, (firstmask & 0xff00) >> 8);
    if (xoff > 0) {
        WRITE_WORD_OR(buff, addr + 2, (lastmask & 0xff00) >> 8);
    }
}

/**
 * write_word_misaligned_lm: Write a misaligned word across two
 * words, in both level and mask buffers. This is core to the text
 * writing routines.
 *
 * @param       buff    buffer to write in
 * @param       word    word to write (16 bits)
 * @param       addr    address of first word
 * @param       xoff    x offset (0-15)
 * @param       lmode   0 = clear, 1 = set, 2 = toggle
 * @param       mmode   0 = clear, 1 = set, 2 = toggle
 */
void write_word_misaligned_lm(uint16_t wordl, uint16_t wordm, unsigned int addr, unsigned int xoff, int lmode, int mmode)
{
    write_word_misaligned(draw_buffer_level, wordl, addr, xoff, lmode);
    write_word_misaligned(draw_buffer_mask, wordm, addr, xoff, mmode);
}

/**
 * fetch_font_info: Fetch font info structs.
 *
 * @param       ch              character
 * @param       font    font id
 */
int fetch_font_info(uint8_t ch, int font, struct FontEntry *font_info, char *lookup)
{
    // First locate the font struct.
    if ((unsigned int)font > SIZEOF_ARRAY(fonts)) {
        return 0; // font does not exist, exit.
    }
    // Load the font info; IDs are always sequential.
    *font_info = fonts[font];
    // Locate character in font lookup table. (If required.)
    if (lookup != NULL) {
        *lookup = font_info->lookup[ch];
        if (*lookup == 0xff) {
            return 0; // character doesn't exist, don't bother writing it.
        }
    }
    return 1;
}

/**
 * write_char16: Draw a character on the current draw buffer.
 *
 * @param       ch      character to write
 * @param       x       x coordinate (left)
 * @param       y       y coordinate (top)
 * @param       font    font to use
 */
void write_char16(char ch, int x, int y, int font)
{
    int yy, row, xshift;
    uint16_t and_mask, or_mask, levels;
    struct FontEntry font_info;

    fetch_font_info(0, font, &font_info, NULL);

    // check if char is partly out of boundary
    uint8_t partly_out = (x < GRAPHICS_LEFT) || (x + font_info.width > GRAPHICS_RIGHT) || (y < GRAPHICS_TOP) || (y + font_info.height > GRAPHICS_BOTTOM);
    // check if char is totally out of boundary, if so return
    if (partly_out && ((x + font_info.width < GRAPHICS_LEFT) || (x > GRAPHICS_RIGHT) || (y + font_info.height < GRAPHICS_TOP) || (y > GRAPHICS_BOTTOM))) {
        return;
    }

    // Compute starting address of character
    int addr = CALC_BUFF_ADDR(x, y);
    int wbit = CALC_BIT_IN_WORD(x);

    // If font only supports lowercase or uppercase, make the letter lowercase or uppercase
    // if (font_info.flags & FONT_LOWERCASE_ONLY) ch = tolower(ch);
    // if (font_info.flags & FONT_UPPERCASE_ONLY) ch = toupper(ch);

    // How wide is the character? We handle characters from 8 pixels up in this function
    if (font_info.width >= 8) {
        // Load data pointer.
        row    = ch * font_info.height;
        xshift = 16 - font_info.width;
        // We can write mask words easily.
        // Level bits are more complicated. We need to set or clear level bits, but only where the mask bit is set; otherwise, we need to leave them alone.
        // To do this, for each word, we construct an AND mask and an OR mask, and apply each individually.
        for (yy = y; yy < y + font_info.height; yy++) {
            if (!partly_out || ((x >= GRAPHICS_LEFT) && (x + font_info.width <= GRAPHICS_RIGHT) && (yy >= GRAPHICS_TOP) && (yy <= GRAPHICS_BOTTOM))) {
                if (font == 3) {
                    // mask
                    write_word_misaligned_OR(draw_buffer_mask, font_mask12x18[row] << xshift, addr, wbit);
                    // level
                    levels   = font_frame12x18[row];
                    // if (!(flags & FONT_INVERT)) // data is normally inverted
                    levels   = ~levels;
                    or_mask  = font_mask12x18[row] << xshift;
                    and_mask = (font_mask12x18[row] & levels) << xshift;
                } else {
                    // mask
                    write_word_misaligned_OR(draw_buffer_mask, font_mask8x10[row] << xshift, addr, wbit);
                    // level
                    levels   = font_frame8x10[row];
                    // if (!(flags & FONT_INVERT)) // data is normally inverted
                    levels   = ~levels;
                    or_mask  = font_mask8x10[row] << xshift;
                    and_mask = (font_mask8x10[row] & levels) << xshift;
                }
                write_word_misaligned_OR(draw_buffer_level, or_mask, addr, wbit);
                // If we're not bold write the AND mask.
                // if (!(flags & FONT_BOLD))
                write_word_misaligned_NAND(draw_buffer_level, and_mask, addr, wbit);
            }
            addr += BUFFER_WIDTH;
            row++;
        }
    }
}

/**
 * write_char: Draw a character on the current draw buffer.
 * Currently supports outlined characters and characters with a width of up to 8 pixels.
 *
 * @param       ch      character to write
 * @param       x       x coordinate (left)
 * @param       y       y coordinate (top)
 * @param       flags   flags to write with
 * @param       font    font to use
 */
void write_char(char ch, int x, int y, int flags, int font)
{
    int yy, row, xshift;
    uint16_t and_mask, or_mask, levels;
    struct FontEntry font_info;
    char lookup = 0;

    fetch_font_info(ch, font, &font_info, &lookup);

    // check if char is partly out of boundary
    uint8_t partly_out = (x < GRAPHICS_LEFT) || (x + font_info.width > GRAPHICS_RIGHT) || (y < GRAPHICS_TOP) || (y + font_info.height > GRAPHICS_BOTTOM);
    // check if char is totally out of boundary, if so return
    if (partly_out && ((x + font_info.width < GRAPHICS_LEFT) || (x > GRAPHICS_RIGHT) || (y + font_info.height < GRAPHICS_TOP) || (y > GRAPHICS_BOTTOM))) {
        return;
    }

    // Compute starting address of character
    unsigned int addr = CALC_BUFF_ADDR(x, y);
    unsigned int wbit = CALC_BIT_IN_WORD(x);

    // If font only supports lowercase or uppercase, make the letter lowercase or uppercase
    // if (font_info.flags & FONT_LOWERCASE_ONLY) ch = tolower(ch);
    // if (font_info.flags & FONT_UPPERCASE_ONLY) ch = toupper(ch);

    // How wide is the character? We handle characters up to 8 pixels in this function
    if (font_info.width <= 8) {
        // Load data pointer.
        row    = lookup * font_info.height * 2;
        xshift = 16 - font_info.width;
        // We can write mask words easily.
        // Level bits are more complicated. We need to set or clear level bits, but only where the mask bit is set; otherwise, we need to leave them alone.
        // To do this, for each word, we construct an AND mask and an OR mask, and apply each individually.
        for (yy = y; yy < y + font_info.height; yy++) {
            if (!partly_out || ((x >= GRAPHICS_LEFT) && (x + font_info.width <= GRAPHICS_RIGHT) && (yy >= GRAPHICS_TOP) && (yy <= GRAPHICS_BOTTOM))) {
                // mask
                write_word_misaligned_OR(draw_buffer_mask, font_info.data[row] << xshift, addr, wbit);
                // level
                levels = font_info.data[row + font_info.height];
                if (!(flags & FONT_INVERT)) { // data is normally inverted
                    levels = ~levels;
                }
                or_mask  = font_info.data[row] << xshift;
                and_mask = (font_info.data[row] & levels) << xshift;
                write_word_misaligned_OR(draw_buffer_level, or_mask, addr, wbit);
                // If we're not bold write the AND mask.
                // if (!(flags & FONT_BOLD))
                write_word_misaligned_NAND(draw_buffer_level, and_mask, addr, wbit);
            }
            addr += BUFFER_WIDTH;
            row++;
        }
    }
}

/**
 * calc_text_dimensions: Calculate the dimensions of a
 * string in a given font. Supports new lines and
 * carriage returns in text.
 *
 * @param       str                     string to calculate dimensions of
 * @param       font_info       font info structure
 * @param       xs                      horizontal spacing
 * @param       ys                      vertical spacing
 * @param       dim                     return result: struct FontDimensions
 */
void calc_text_dimensions(char *str, struct FontEntry font, int xs, int ys, struct FontDimensions *dim)
{
    int max_length = 0, line_length = 0, lines = 1;

    while (*str != 0) {
        line_length++;
        if (*str == '\n' || *str == '\r') {
            if (line_length > max_length) {
                max_length = line_length;
            }
            line_length = 0;
            lines++;
        }
        str++;
    }
    if (line_length > max_length) {
        max_length = line_length;
    }
    dim->width  = max_length * (font.width + xs);
    dim->height = lines * (font.height + ys);
}

/**
 * write_string: Draw a string on the screen with certain
 * alignment parameters.
 *
 * @param       str             string to write
 * @param       x               x coordinate
 * @param       y               y coordinate
 * @param       xs              horizontal spacing
 * @param       ys              horizontal spacing
 * @param       va              vertical align
 * @param       ha              horizontal align
 * @param       flags   flags (passed to write_char)
 * @param       font    font
 */
void write_string(char *str, int x, int y, int xs, int ys, int va, int ha, int flags, int font)
{
    int xx = 0, yy = 0, xx_original = 0;
    struct FontEntry font_info;
    struct FontDimensions dim;

    // Determine font info and dimensions/position of the string.
    fetch_font_info(0, font, &font_info, NULL);
    calc_text_dimensions(str, font_info, xs, ys, &dim);
    switch (va) {
    case TEXT_VA_TOP:
        yy = y;
        break;
    case TEXT_VA_MIDDLE:
        yy = y - (dim.height / 2);
        break;
    case TEXT_VA_BOTTOM:
        yy = y - dim.height;
        break;
    }
    switch (ha) {
    case TEXT_HA_LEFT:
        xx = x;
        break;
    case TEXT_HA_CENTER:
        xx = x - (dim.width / 2);
        break;
    case TEXT_HA_RIGHT:
        xx = x - dim.width;
        break;
    }
    // Then write each character.
    xx_original = xx;
    while (*str != 0) {
        if (*str == '\n' || *str == '\r') {
            yy += ys + font_info.height;
            xx  = xx_original;
        } else {
            if (xx >= 0 && xx < GRAPHICS_WIDTH_REAL) {
                if (font_info.id < 2) {
                    write_char(*str, xx, yy, flags, font);
                } else {
                    write_char16(*str, xx, yy, font);
                }
            }
            xx += font_info.width + xs;
        }
        str++;
    }
}

/**
 * write_string_formatted: Draw a string with format escape
 * sequences in it. Allows for complex text effects.
 *
 * @param       str             string to write (with format data)
 * @param       x               x coordinate
 * @param       y               y coordinate
 * @param       xs              default horizontal spacing
 * @param       ys              default horizontal spacing
 * @param       va              vertical align
 * @param       ha              horizontal align
 * @param       flags   flags (passed to write_char)
 */
void write_string_formatted(char *str, int x, int y, int xs, int ys,
                            __attribute__((unused)) int va, __attribute__((unused)) int ha, int flags)
{
    int fcode = 0, fptr = 0, font = 0, fwidth = 0, fheight = 0, xx = x, yy = y, max_xx = 0, max_height = 0;
    struct FontEntry font_info;

    // Retrieve sizes of the fonts: bigfont and smallfont.
    fetch_font_info(0, 0, &font_info, NULL);
    int smallfontwidth = font_info.width, smallfontheight = font_info.height;
    fetch_font_info(0, 1, &font_info, NULL);
    int bigfontwidth   = font_info.width, bigfontheight = font_info.height;
    // 11 byte stack with last byte as NUL.
    char fstack[11];
    fstack[10] = '\0';
    // First, we need to parse the string for format characters and
    // work out a bounding box. We'll parse again for the final output.
    // This is a simple state machine parser.
    char *ostr = str;
    while (*str) {
        if (*str == '<' && fcode == 1) {
            // escape code: skip
            fcode = 0;
        }
        if (*str == '<' && fcode == 0) {
            // begin format code?
            fcode = 1;
            fptr  = 0;
        }
        if (*str == '>' && fcode == 1) {
            fcode = 0;
            if (strcmp(fstack, "B")) {
                // switch to "big" font (font #1)
                fwidth  = bigfontwidth;
                fheight = bigfontheight;
            } else if (strcmp(fstack, "S")) {
                // switch to "small" font (font #0)
                fwidth  = smallfontwidth;
                fheight = smallfontheight;
            }
            if (fheight > max_height) {
                max_height = fheight;
            }
            // Skip over this byte. Go to next byte.
            str++;
            continue;
        }
        if (*str != '<' && *str != '>' && fcode == 1) {
            // Add to the format stack (up to 10 bytes.)
            if (fptr > 10) {
                // stop adding bytes
                str++; // go to next byte
                continue;
            }
            fstack[fptr++] = *str;
            fstack[fptr]   = '\0'; // clear next byte (ready for next char or to terminate string.)
        }
        if (fcode == 0) {
            // Not a format code, raw text.
            xx += fwidth + xs;
            if (*str == '\n') {
                if (xx > max_xx) {
                    max_xx = xx;
                }
                xx  = x;
                yy += fheight + ys;
            }
        }
        str++;
    }
    // Reset string pointer.
    str = ostr;
    // Now we've parsed it and got a bbox, we need to work out the dimensions of it
    // and how to align it.
    /*int width = max_xx - x;
       int height = yy - y;
       int ay, ax;
       switch(va)
       {
       case TEXT_VA_TOP:               ay = yy; break;
       case TEXT_VA_MIDDLE:    ay = yy - (height / 2); break;
       case TEXT_VA_BOTTOM:    ay = yy - height; break;
       }
       switch(ha)
       {
       case TEXT_HA_LEFT:              ax = x; break;
       case TEXT_HA_CENTER:    ax = x - (width / 2); break;
       case TEXT_HA_RIGHT:             ax = x - width; break;
       }*/
    // So ax,ay is our new text origin. Parse the text format again and paint
    // the text on the display.
    fcode = 0;
    fptr  = 0;
    font  = 0;
    xx    = 0;
    yy    = 0;
    while (*str) {
        if (*str == '<' && fcode == 1) {
            // escape code: skip
            fcode = 0;
        }
        if (*str == '<' && fcode == 0) {
            // begin format code?
            fcode = 1;
            fptr  = 0;
        }
        if (*str == '>' && fcode == 1) {
            fcode = 0;
            if (strcmp(fstack, "B")) {
                // switch to "big" font (font #1)
                fwidth  = bigfontwidth;
                fheight = bigfontheight;
                font    = 1;
            } else if (strcmp(fstack, "S")) {
                // switch to "small" font (font #0)
                fwidth  = smallfontwidth;
                fheight = smallfontheight;
                font    = 0;
            }
            // Skip over this byte. Go to next byte.
            str++;
            continue;
        }
        if (*str != '<' && *str != '>' && fcode == 1) {
            // Add to the format stack (up to 10 bytes.)
            if (fptr > 10) {
                // stop adding bytes
                str++; // go to next byte
                continue;
            }
            fstack[fptr++] = *str;
            fstack[fptr]   = '\0'; // clear next byte (ready for next char or to terminate string.)
        }
        if (fcode == 0) {
            // Not a format code, raw text. So we draw it.
            // TODO - different font sizes.
            write_char(*str, xx, yy + (max_height - fheight), flags, font);
            xx += fwidth + xs;
            if (*str == '\n') {
                if (xx > max_xx) {
                    max_xx = xx;
                }
                xx  = x;
                yy += fheight + ys;
            }
        }
        str++;
    }
}

void drawAttitude(uint16_t x, uint16_t y, int16_t pitch, int16_t roll, uint16_t size)
{
    int16_t a = mySin(roll + 360);
    int16_t b = myCos(roll + 360);
    int16_t c = mySin(roll + 90 + 360) * 5 / 100;
    int16_t d = myCos(roll + 90 + 360) * 5 / 100;

    int16_t k;
    int16_t l;

    int16_t indi30x1 = myCos(30) * (size / 2 + 1) / 100;
    int16_t indi30y1 = mySin(30) * (size / 2 + 1) / 100;

    int16_t indi30x2 = myCos(30) * (size / 2 + 4) / 100;
    int16_t indi30y2 = mySin(30) * (size / 2 + 4) / 100;

    int16_t indi60x1 = myCos(60) * (size / 2 + 1) / 100;
    int16_t indi60y1 = mySin(60) * (size / 2 + 1) / 100;

    int16_t indi60x2 = myCos(60) * (size / 2 + 4) / 100;
    int16_t indi60y2 = mySin(60) * (size / 2 + 4) / 100;

    pitch = pitch % 90;
    if (pitch > 90) {
        pitch = pitch - 90;
    }
    if (pitch < -90) {
        pitch = pitch + 90;
    }
    a = (a * (size / 2)) / 100;
    b = (b * (size / 2)) / 100;

    if (roll < -90 || roll > 90) {
        pitch = pitch * -1;
    }
    k = a * pitch / 90;
    l = b * pitch / 90;

    // scale
    // 0
    // drawLine((x)-1-(size/2+4), (y)-1, (x)-1 - (size/2+1), (y)-1);
    // drawLine((x)-1+(size/2+4), (y)-1, (x)-1 + (size/2+1), (y)-1);
    write_line_outlined((x) - 1 - (size / 2 + 4), (y) - 1, (x) - 1 - (size / 2 + 1), (y) - 1, 0, 0, 0, 1);
    write_line_outlined((x) - 1 + (size / 2 + 4), (y) - 1, (x) - 1 + (size / 2 + 1), (y) - 1, 0, 0, 0, 1);

    // 30
    // drawLine((x)-1+indi30x1, (y)-1-indi30y1, (x)-1 + indi30x2, (y)-1 - indi30y2);
    // drawLine((x)-1-indi30x1, (y)-1-indi30y1, (x)-1 - indi30x2, (y)-1 - indi30y2);
    write_line_outlined((x) - 1 + indi30x1, (y) - 1 - indi30y1, (x) - 1 + indi30x2, (y) - 1 - indi30y2, 0, 0, 0, 1);
    write_line_outlined((x) - 1 - indi30x1, (y) - 1 - indi30y1, (x) - 1 - indi30x2, (y) - 1 - indi30y2, 0, 0, 0, 1);
    // 60
    // drawLine((x)-1+indi60x1, (y)-1-indi60y1, (x)-1 + indi60x2, (y)-1 - indi60y2);
    // drawLine((x)-1-indi60x1, (y)-1-indi60y1, (x)-1 - indi60x2, (y)-1 - indi60y2);
    write_line_outlined((x) - 1 + indi60x1, (y) - 1 - indi60y1, (x) - 1 + indi60x2, (y) - 1 - indi60y2, 0, 0, 0, 1);
    write_line_outlined((x) - 1 - indi60x1, (y) - 1 - indi60y1, (x) - 1 - indi60x2, (y) - 1 - indi60y2, 0, 0, 0, 1);
    // 90
    // drawLine((x)-1, (y)-1-(size/2+4), (x)-1, (y)-1 - (size/2+1));
    write_line_outlined((x) - 1, (y) - 1 - (size / 2 + 4), (x) - 1, (y) - 1 - (size / 2 + 1), 0, 0, 0, 1);

    // roll
    // drawLine((x)-1 - b, (y)-1 + a, (x)-1 + b, (y)-1 - a); //Direction line
    write_line_outlined((x) - 1 - b, (y) - 1 + a, (x) - 1 + b, (y) - 1 - a, 0, 0, 0, 1); // Direction line
    // "wingtips"
    // drawLine((x)-1 - b, (y)-1 + a, (x)-1 - b + d, (y)-1 + a - c);
    // drawLine((x)-1 + b + d, (y)-1 - a - c, (x)-1 + b, (y)-1 - a);
    write_line_outlined((x) - 1 - b, (y) - 1 + a, (x) - 1 - b + d, (y) - 1 + a - c, 0, 0, 0, 1);
    write_line_outlined((x) - 1 + b + d, (y) - 1 - a - c, (x) - 1 + b, (y) - 1 - a, 0, 0, 0, 1);

    // pitch
    // drawLine((x)-1, (y)-1, (x)-1 - k, (y)-1 - l);
    write_line_outlined((x) - 1, (y) - 1, (x) - 1 - k, (y) - 1 - l, 0, 0, 0, 1);

    // drawCircle(x-1, y-1, 5);
    // write_circle_outlined(x-1, y-1, 5,0,0,0,1);
    // drawCircle(x-1, y-1, size/2+4);
    // write_circle_outlined(x-1, y-1, size/2+4,0,0,0,1);
}

void drawBattery(uint16_t x, uint16_t y, uint8_t battery, uint16_t size)
{
    int i = 0;
    int batteryLines;

    // top
    /*drawLine((x)-1+(size/2-size/4), (y)-1, (x)-1 + (size/2+size/4), (y)-1);
       drawLine((x)-1+(size/2-size/4), (y)-1+1, (x)-1 + (size/2+size/4), (y)-1+1);

       drawLine((x)-1, (y)-1+2, (x)-1 + size, (y)-1+2);
       //bottom
       drawLine((x)-1, (y)-1+size*3, (x)-1 + size, (y)-1+size*3);
       //left
       drawLine((x)-1, (y)-1+2, (x)-1, (y)-1+size*3);

       //right
       drawLine((x)-1+size, (y)-1+2, (x)-1+size, (y)-1+size*3);*/

    write_rectangle_outlined((x) - 1, (y) - 1 + 2, size, size * 3, 0, 1);
    write_vline_lm((x) - 1 + (size / 2 + size / 4) + 1, (y) - 2, (y) - 1 + 1, 0, 1);
    write_vline_lm((x) - 1 + (size / 2 - size / 4) - 1, (y) - 2, (y) - 1 + 1, 0, 1);
    write_hline_lm((x) - 1 + (size / 2 - size / 4), (x) - 1 + (size / 2 + size / 4), (y) - 2, 0, 1);
    write_hline_lm((x) - 1 + (size / 2 - size / 4), (x) - 1 + (size / 2 + size / 4), (y) - 1, 1, 1);
    write_hline_lm((x) - 1 + (size / 2 - size / 4), (x) - 1 + (size / 2 + size / 4), (y) - 1 + 1, 1, 1);

    batteryLines = battery * (size * 3 - 2) / 100;
    for (i = 0; i < batteryLines; i++) {
        write_hline_lm((x) - 1, (x) - 1 + size, (y) - 1 + size * 3 - i, 1, 1);
    }
}

/**
 * hud_draw_vertical_scale: Draw a vertical scale.
 *
 * @param       v                   value to display as an integer
 * @param       range               range about value to display (+/- range/2 each direction)
 * @param       halign              horizontal alignment: -1 = left, +1 = right.
 * @param       x                   x displacement
 * @param       y                   y displacement
 * @param       height              height of scale
 * @param       mintick_step        how often a minor tick is shown
 * @param       majtick_step        how often a major tick is shown
 * @param       mintick_len         minor tick length
 * @param       majtick_len         major tick length
 * @param       boundtick_len       boundary tick length
 * @param       max_val             maximum expected value (used to compute size of arrow ticker)
 * @param       flags               special flags (see hud.h.)
 */
// #define VERTICAL_SCALE_BRUTE_FORCE_BLANK_OUT
// #define VERTICAL_SCALE_FILLED_NUMBER
void hud_draw_vertical_scale(int v, int range, int halign, int x, int y, int height, int mintick_step, int majtick_step, int mintick_len, int majtick_len,
                             int boundtick_len, __attribute__((unused)) int max_val, int flags)
{
    char temp[15];
    struct FontEntry font_info;
    struct FontDimensions dim;
    // Compute the position of the elements.
    int majtick_start = 0, majtick_end = 0, mintick_start = 0, mintick_end = 0, boundtick_start = 0, boundtick_end = 0;

    majtick_start   = x;
    mintick_start   = x;
    boundtick_start = x;
    if (halign == -1) {
        majtick_end     = x + majtick_len;
        mintick_end     = x + mintick_len;
        boundtick_end   = x + boundtick_len;
    } else if (halign == +1) {
        majtick_end     = x - majtick_len;
        mintick_end     = x - mintick_len;
        boundtick_end   = x - boundtick_len;
    }
    // Retrieve width of large font (font #0); from this calculate the x spacing.
    fetch_font_info(0, 0, &font_info, NULL);
    int arrow_len      = (font_info.height / 2) + 1;
    int text_x_spacing = (font_info.width / 2);
    int max_text_y     = 0, text_length = 0;
    int small_font_char_width = font_info.width + 1; // +1 for horizontal spacing = 1
    // For -(range / 2) to +(range / 2), draw the scale.
    int range_2 = range / 2; // , height_2 = height / 2;
    int r = 0, rr = 0, rv = 0, ys = 0, style = 0; // calc_ys = 0,
    // Iterate through each step.
    for (r = -range_2; r <= +range_2; r++) {
        style = 0;
        rr    = r + range_2 - v; // normalise range for modulo, subtract value to move ticker tape
        rv    = -rr + range_2; // for number display
        if (flags & HUD_VSCALE_FLAG_NO_NEGATIVE) {
            rr += majtick_step / 2;
        }
        if (rr % majtick_step == 0) {
            style = 1; // major tick
        } else if (rr % mintick_step == 0) {
            style = 2; // minor tick
        } else {
            style = 0;
        }
        if (flags & HUD_VSCALE_FLAG_NO_NEGATIVE && rv < 0) {
            continue;
        }
        if (style) {
            // Calculate y position.
            ys = ((long int)(r * height) / (long int)range) + y;
            // Depending on style, draw a minor or a major tick.
            if (style == 1) {
                write_hline_outlined(majtick_start, majtick_end, ys, 2, 2, 0, 1);
                memset(temp, ' ', 10);
                sprintf(temp, "%d", rv);
                text_length = (strlen(temp) + 1) * small_font_char_width; // add 1 for margin
                if (text_length > max_text_y) {
                    max_text_y = text_length;
                }
                if (halign == -1) {
                    write_string(temp, majtick_end + text_x_spacing + 1, ys, 1, 0, TEXT_VA_MIDDLE, TEXT_HA_LEFT, 0, 1);
                } else {
                    write_string(temp, majtick_end - text_x_spacing + 1, ys, 1, 0, TEXT_VA_MIDDLE, TEXT_HA_RIGHT, 0, 1);
                }
            } else if (style == 2) {
                write_hline_outlined(mintick_start, mintick_end, ys, 2, 2, 0, 1);
            }
        }
    }
    // Generate the string for the value, as well as calculating its dimensions.
    memset(temp, ' ', 10);
    // my_itoa(v, temp);
    sprintf(temp, "%d", v);
    // TODO: add auto-sizing.
    calc_text_dimensions(temp, font_info, 1, 0, &dim);
    int xx = 0, i = 0;
    if (halign == -1) {
        xx = majtick_end + text_x_spacing;
    } else {
        xx = majtick_end - text_x_spacing;
    }
    y++;
    // Draw an arrow from the number to the point.
    for (i = 0; i < arrow_len; i++) {
        if (halign == -1) {
            write_pixel_lm(xx - arrow_len + i, y - i - 1, 1, 1);
            write_pixel_lm(xx - arrow_len + i, y + i - 1, 1, 1);
#ifdef VERTICAL_SCALE_FILLED_NUMBER
            write_hline_lm(xx + dim.width - 1, xx - arrow_len + i + 1, y - i - 1, 0, 1);
            write_hline_lm(xx + dim.width - 1, xx - arrow_len + i + 1, y + i - 1, 0, 1);
#else
            write_hline_lm(xx + dim.width - 1, xx - arrow_len + i + 1, y - i - 1, 0, 0);
            write_hline_lm(xx + dim.width - 1, xx - arrow_len + i + 1, y + i - 1, 0, 0);
#endif
        } else {
            write_pixel_lm(xx + arrow_len - i, y - i - 1, 1, 1);
            write_pixel_lm(xx + arrow_len - i, y + i - 1, 1, 1);
#ifdef VERTICAL_SCALE_FILLED_NUMBER
            write_hline_lm(xx - dim.width - 1, xx + arrow_len - i - 1, y - i - 1, 0, 1);
            write_hline_lm(xx - dim.width - 1, xx + arrow_len - i - 1, y + i - 1, 0, 1);
#else
            write_hline_lm(xx - dim.width - 1, xx + arrow_len - i - 1, y - i - 1, 0, 0);
            write_hline_lm(xx - dim.width - 1, xx + arrow_len - i - 1, y + i - 1, 0, 0);
#endif
        }
    }
    if (halign == -1) {
        write_hline_lm(xx, xx + dim.width - 1, y - arrow_len, 1, 1);
        write_hline_lm(xx, xx + dim.width - 1, y + arrow_len - 2, 1, 1);
        write_vline_lm(xx + dim.width - 1, y - arrow_len, y + arrow_len - 2, 1, 1);
    } else {
        write_hline_lm(xx, xx - dim.width - 1, y - arrow_len, 1, 1);
        write_hline_lm(xx, xx - dim.width - 1, y + arrow_len - 2, 1, 1);
        write_vline_lm(xx - dim.width - 1, y - arrow_len, y + arrow_len - 2, 1, 1);
    }
    // Draw the text.
    if (halign == -1) {
        write_string(temp, xx, y, 1, 0, TEXT_VA_MIDDLE, TEXT_HA_LEFT, 0, 0);
    } else {
        write_string(temp, xx, y, 1, 0, TEXT_VA_MIDDLE, TEXT_HA_RIGHT, 0, 0);
    }
#ifdef VERTICAL_SCALE_BRUTE_FORCE_BLANK_OUT
    // This is a bad brute force method destuctive to other things that maybe drawn underneath like e.g. the artificial horizon:
    // Then, add a slow cut off on the edges, so the text doesn't sharply
    // disappear. We simply clear the areas above and below the ticker, and we
    // use little markers on the edges.
    if (halign == -1) {
        write_filled_rectangle_lm(majtick_end + text_x_spacing, y + (height / 2) - (font_info.height / 2), max_text_y - boundtick_start, font_info.height, 0, 0);
        write_filled_rectangle_lm(majtick_end + text_x_spacing, y - (height / 2) - (font_info.height / 2), max_text_y - boundtick_start, font_info.height, 0, 0);
    } else {
        write_filled_rectangle_lm(majtick_end - text_x_spacing - max_text_y, y + (height / 2) - (font_info.height / 2), max_text_y, font_info.height, 0, 0);
        write_filled_rectangle_lm(majtick_end - text_x_spacing - max_text_y, y - (height / 2) - (font_info.height / 2), max_text_y, font_info.height, 0, 0);
    }
#endif
    y--;
    write_hline_outlined(boundtick_start, boundtick_end, y + (height / 2), 2, 2, 0, 1);
    write_hline_outlined(boundtick_start, boundtick_end, y - (height / 2), 2, 2, 0, 1);
}

/**
 * hud_draw_compass: Draw a compass.
 *
 * @param       v               value for the compass
 * @param       range           range about value to display (+/- range/2 each direction)
 * @param       width           length in pixels
 * @param       x               x displacement
 * @param       y               y displacement
 * @param       mintick_step    how often a minor tick is shown
 * @param       majtick_step    how often a major tick (heading "xx") is shown
 * @param       mintick_len     minor tick length
 * @param       majtick_len     major tick length
 * @param       flags           special flags (see hud.h.)
 */
#define COMPASS_SMALL_NUMBER
// #define COMPASS_FILLED_NUMBER
void hud_draw_linear_compass(int v, int range, int width, int x, int y, int mintick_step, int majtick_step, int mintick_len, int majtick_len, __attribute__((unused)) int flags)
{
    v %= 360; // wrap, just in case.
    struct FontEntry font_info;
    int majtick_start = 0, majtick_end = 0, mintick_start = 0, mintick_end = 0, textoffset = 0;
    char headingstr[4];
    majtick_start = y;
    majtick_end   = y - majtick_len;
    mintick_start = y;
    mintick_end   = y - mintick_len;
    textoffset    = 8;
    int r, style, rr, xs; // rv,
    int range_2 = range / 2;
    for (r = -range_2; r <= +range_2; r++) {
        style = 0;
        rr    = (v + r + 360) % 360; // normalise range for modulo, add to move compass track
        // rv = -rr + range_2; // for number display
        if (rr % majtick_step == 0) {
            style = 1; // major tick
        } else if (rr % mintick_step == 0) {
            style = 2; // minor tick
        }
        if (style) {
            // Calculate x position.
            xs = ((long int)(r * width) / (long int)range) + x;
            // Draw it.
            if (style == 1) {
                write_vline_outlined(xs, majtick_start, majtick_end, 2, 2, 0, 1);
                // Draw heading above this tick.
                // If it's not one of north, south, east, west, draw the heading.
                // Otherwise, draw one of the identifiers.
                if (rr % 90 != 0) {
                    // We abbreviate heading to two digits. This has the side effect of being easy to compute.
                    headingstr[0] = '0' + (rr / 100);
                    headingstr[1] = '0' + ((rr / 10) % 10);
                    headingstr[2] = 0;
                    headingstr[3] = 0; // nul to terminate
                } else {
                    switch (rr) {
                    case 0:
                        headingstr[0] = 'N';
                        break;
                    case 90:
                        headingstr[0] = 'E';
                        break;
                    case 180:
                        headingstr[0] = 'S';
                        break;
                    case 270:
                        headingstr[0] = 'W';
                        break;
                    }
                    headingstr[1] = 0;
                    headingstr[2] = 0;
                    headingstr[3] = 0;
                }
                // +1 fudge...!
                write_string(headingstr, xs + 1, majtick_start + textoffset, 1, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, 1);
            } else if (style == 2) {
                write_vline_outlined(xs, mintick_start, mintick_end, 2, 2, 0, 1);
            }
        }
    }
    // Then, draw a rectangle with the present heading in it.
    // We want to cover up any other markers on the bottom.
    // First compute font size.
    headingstr[0] = '0' + (v / 100);
    headingstr[1] = '0' + ((v / 10) % 10);
    headingstr[2] = '0' + (v % 10);
    headingstr[3] = 0;
    fetch_font_info(0, 3, &font_info, NULL);
#ifdef COMPASS_SMALL_NUMBER
    int rect_width = font_info.width * 3;
#ifdef COMPASS_FILLED_NUMBER
    write_filled_rectangle_lm(x - (rect_width / 2), majtick_start - 7, rect_width, font_info.height, 0, 1);
#else
    write_filled_rectangle_lm(x - (rect_width / 2), majtick_start - 7, rect_width, font_info.height, 0, 0);
#endif
    write_rectangle_outlined(x - (rect_width / 2), majtick_start - 7, rect_width, font_info.height, 0, 1);
    write_string(headingstr, x + 1, majtick_start + textoffset - 5, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 1, 0);
#else
    int rect_width = (font_info.width + 1) * 3 + 2;
#ifdef COMPASS_FILLED_NUMBER
    write_filled_rectangle_lm(x - (rect_width / 2), majtick_start + 2, rect_width, font_info.height + 2, 0, 1);
#else
    write_filled_rectangle_lm(x - (rect_width / 2), majtick_start + 2, rect_width, font_info.height + 2, 0, 0);
#endif
    write_rectangle_outlined(x - (rect_width / 2), majtick_start + 2, rect_width, font_info.height + 2, 0, 1);
    write_string(headingstr, x + 1, majtick_start + textoffset + 2, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 1, 3);
#endif
}

void draw_artificial_horizon(float angle, float pitch, int16_t l_x, int16_t l_y, int16_t size)
{
    float alpha;
    uint8_t vertical = 0, horizontal = 0;
    int16_t x1, x2;
    int16_t y1, y2;
    int16_t refx, refy;

    alpha = DEG2RAD(angle);
    refx  = l_x + size / 2;
    refy  = l_y + size / 2;

    //
    float k    = 0;
    float dx   = sinf(alpha) * (pitch / 90.0f * (size / 2));
    float dy   = cosf(alpha) * (pitch / 90.0f * (size / 2));
    int16_t x0 = (size / 2) - dx;
    int16_t y0 = (size / 2) + dy;
    // calculate the line function
    if ((angle < 90.0f) && (angle > -90)) {
        vertical = 0;
        if (fabsf(angle) < 1e-5f) {
            horizontal = 1;
        } else {
            k = tanf(alpha);
        }
    } else {
        vertical = 1;
    }

    // crossing point of line
    if (!vertical && !horizontal) {
        // y-y0=k(x-x0)
        int16_t x = 0;
        int16_t y = k * (x - x0) + y0;
        // find right crossing point
        x1 = x;
        y1 = y;
        if (y < 0) {
            y1 = 0;
            x1 = ((y1 - y0) + k * x0) / k;
        }
        if (y > size) {
            y1 = size;
            x1 = ((y1 - y0) + k * x0) / k;
        }
        // left crossing point
        x  = size;
        y  = k * (x - x0) + y0;
        x2 = x;
        y2 = y;
        if (y < 0) {
            y2 = 0;
            x2 = ((y2 - y0) + k * x0) / k;
        }
        if (y > size) {
            y2 = size;
            x2 = ((y2 - y0) + k * x0) / k;
        }
        // move to location
        // horizon line
        write_line_outlined(x1 + l_x, y1 + l_y, x2 + l_x, y2 + l_y, 0, 0, 0, 1);
        // fill
        if (angle <= 0.0f && angle > -90.0f) {
            // write_string("1", GRAPHICS_RIGHT/2, GRAPHICS_BOTTOM-10, 0, 0, TEXT_VA_BOTTOM, TEXT_HA_CENTER, 0, 3);
            for (int i = y2; i < size; i++) {
                x2 = ((i - y0) + k * x0) / k;
                if (x2 > size) {
                    x2 = size;
                }
                if (x2 < 0) {
                    x2 = 0;
                }
                write_hline_lm(x2 + l_x, size + l_x, i + l_y, 1, 1);
            }
        } else if (angle < -90.0f) {
            // write_string("2", GRAPHICS_RIGHT/2, GRAPHICS_BOTTOM-10, 0, 0, TEXT_VA_BOTTOM, TEXT_HA_CENTER, 0, 3);
            for (int i = 0; i < y2; i++) {
                x2 = ((i - y0) + k * x0) / k;
                if (x2 > size) {
                    x2 = size;
                }
                if (x2 < 0) {
                    x2 = 0;
                }
                write_hline_lm(size + l_x, x2 + l_x, i + l_y, 1, 1);
            }
        } else if (angle > 0.0f && angle < 90.0f) {
            // write_string("3", GRAPHICS_RIGHT/2, GRAPHICS_BOTTOM-10, 0, 0, TEXT_VA_BOTTOM, TEXT_HA_CENTER, 0, 3);
            for (int i = y1; i < size; i++) {
                x2 = ((i - y0) + k * x0) / k;
                if (x2 > size) {
                    x2 = size;
                }
                if (x2 < 0) {
                    x2 = 0;
                }
                write_hline_lm(0 + l_x, x2 + l_x, i + l_y, 1, 1);
            }
        } else if (angle > 90.0f) {
            // write_string("4", GRAPHICS_RIGHT/2, GRAPHICS_BOTTOM-10, 0, 0, TEXT_VA_BOTTOM, TEXT_HA_CENTER, 0, 3);
            for (int i = 0; i < y1; i++) {
                x2 = ((i - y0) + k * x0) / k;
                if (x2 > size) {
                    x2 = size;
                }
                if (x2 < 0) {
                    x2 = 0;
                }
                write_hline_lm(x2 + l_x, 0 + l_x, i + l_y, 1, 1);
            }
        }
    } else if (vertical) {
        // horizon line
        write_line_outlined(x0 + l_x, 0 + l_y, x0 + l_x, size + l_y, 0, 0, 0, 1);
        if (angle >= 90.0f) {
            // write_string("5", GRAPHICS_RIGHT/2, GRAPHICS_BOTTOM-10, 0, 0, TEXT_VA_BOTTOM, TEXT_HA_CENTER, 0, 3);
            for (int i = 0; i < size; i++) {
                write_hline_lm(0 + l_x, x0 + l_x, i + l_y, 1, 1);
            }
        } else {
            // write_string("6", GRAPHICS_RIGHT/2, GRAPHICS_BOTTOM-10, 0, 0, TEXT_VA_BOTTOM, TEXT_HA_CENTER, 0, 3);
            for (int i = 0; i < size; i++) {
                write_hline_lm(size + l_x, x0 + l_x, i + l_y, 1, 1);
            }
        }
    } else if (horizontal) {
        // horizon line
        write_hline_outlined(0 + l_x, size + l_x, y0 + l_y, 0, 0, 0, 1);
        if (angle < 0) {
            // write_string("7", GRAPHICS_RIGHT/2, GRAPHICS_BOTTOM-10, 0, 0, TEXT_VA_BOTTOM, TEXT_HA_CENTER, 0, 3);
            for (int i = 0; i < y0; i++) {
                write_hline_lm(0 + l_x, size + l_x, i + l_y, 1, 1);
            }
        } else {
            // write_string("8", GRAPHICS_RIGHT/2, GRAPHICS_BOTTOM-10, 0, 0, TEXT_VA_BOTTOM, TEXT_HA_CENTER, 0, 3);
            for (int i = y0; i < size; i++) {
                write_hline_lm(0 + l_x, size + l_x, i + l_y, 1, 1);
            }
        }
    }

    // sides
    write_line_outlined(l_x, l_y, l_x, l_y + size, 0, 0, 0, 1);
    write_line_outlined(l_x + size, l_y, l_x + size, l_y + size, 0, 0, 0, 1);
    // plane
    write_line_outlined(refx - 5, refy, refx + 6, refy, 0, 0, 0, 1);
    write_line_outlined(refx, refy, refx, refy - 3, 0, 0, 0, 1);
}

// JR_HINT work in progress
// artificial horizon in HUD design
// #define DEBUG_HUD_AH
#define SUB_HORIZON_WIDTH 70
#define SUB_NUMBERS_WIDTH 85
#define SUB_HORIZON_GAP   20
#define UP_DOWN_LENGTH    8
#define CENTER_BODY       3
#define CENTER_WING       7
#define CENTER_RUDDER     5
void hud_draw_artificial_horizon(float roll, float pitch, __attribute__((unused)) float yaw, int16_t x, int16_t y, int8_t max_pitch_visible, int8_t delta_degree, int16_t main_line_width, int16_t size)
{
    char temp[20] = { 0 };
    float sin_roll;
    float cos_roll;
    int8_t i;
    int16_t pp_x; // pitch point x
    int16_t pp_y; // pitch point y
    int16_t mp_x; // middle point x
    int16_t mp_y; // middle point y
    int16_t s_x; // sum x
    int16_t s_y; // sum y
    int16_t d_x; // delta x
    int16_t d_y; // delta y
    int16_t side_length;
    int16_t pitch_delta;

// int16_t gap_length;													// JR_HINT gap umsetzen
// int16_t ud_length;													// JR_HINT up/down umsetzen

#ifdef DEBUG_HUD_AH
    sprintf(temp, "Roll: %4d", (int)roll);
    write_string(temp, GRAPHICS_LEFT + 8, 55, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
    sprintf(temp, "Pitch:%4d", (int)pitch);
    write_string(temp, GRAPHICS_LEFT + 8, 65, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
    sprintf(temp, "Yaw:  %4d", (int)yaw);
    write_string(temp, GRAPHICS_LEFT + 8, 75, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
#endif

    sin_roll    = sinf(DEG2RAD(roll));
    cos_roll    = cosf(DEG2RAD(roll));

    // roll to pitch transformation
    pp_x        = x * (1 + (sin_roll * pitch) / (float)max_pitch_visible);
    pp_y        = y * (1 + (cos_roll * pitch) / (float)max_pitch_visible);

    // main horizon
    side_length = main_line_width * size / 200;
    d_x = cos_roll * side_length;
    d_y = sin_roll * side_length;
    write_line_outlined_dashed_truncated(pp_x - d_x, pp_y + d_y, pp_x + d_x, pp_y - d_y, 2, 2, 0, 1, 0);

    // sub horizons
    pitch_delta = GRAPHICS_BOTTOM / (2 * (float)max_pitch_visible / (float)delta_degree) + 2;
// gap_length = SUB_HORIZON_GAP * size / 200;
// ud_length = UP_DOWN_LENGTH * size / 100;

    for (i = 1; i <= 180 / delta_degree; i++) {
        sprintf(temp, "%2d", delta_degree * i); // string for the sub horizon numbers
        mp_x = sin_roll * pitch_delta * i; // x middle point of the current sub horizon
        mp_y = cos_roll * pitch_delta * i; // y middle point of the current sub horizon

        // deltas for the sub horizon lines
        side_length = SUB_HORIZON_WIDTH * size / 200;
        d_x = cos_roll * side_length;
        d_y = sin_roll * side_length;

        // positive sub horizon line (solid)
        s_x = pp_x - mp_x;
        s_y = pp_y - mp_y;
        write_line_outlined_dashed_truncated(s_x - d_x, s_y + d_y, s_x + d_x, s_y - d_y, 2, 2, 0, 1, 0);

        // negative sub horizon line (dashed)
        s_x = pp_x + mp_x;
        s_y = pp_y + mp_y;
        write_line_outlined_dashed_truncated(s_x - d_x, s_y + d_y, s_x + d_x, s_y - d_y, 2, 2, 0, 1, 4);

        // deltas for the sub horizon numbers
        side_length = SUB_NUMBERS_WIDTH * size / 200;
        d_x = cos_roll * side_length;
        d_y = sin_roll * side_length;

        // positive sub horizon numbers
        s_x = pp_x - mp_x;
        s_y = pp_y - mp_y;
        write_string(temp, s_x - d_x, s_y + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, 2);
        write_string(temp, s_x + d_x, s_y - d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, 2);

        // negative sub horizon numbers
        s_x = pp_x + mp_x;
        s_y = pp_y + mp_y;
        write_string(temp, s_x - d_x, s_y + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, 2);
        write_string(temp, s_x + d_x, s_y - d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, 2);
    }

    // center mark
    write_circle_outlined(x, y, CENTER_BODY, 0, 0, 0, 1);
    write_line_outlined(x - CENTER_WING - CENTER_BODY, y, x - CENTER_BODY, y, 2, 0, 0, 1);
    write_line_outlined(x + 1 + CENTER_BODY, y, x + 1 + CENTER_BODY + CENTER_WING, y, 0, 2, 0, 1);
    write_line_outlined(x, y - CENTER_RUDDER - CENTER_BODY, x, y - CENTER_BODY, 2, 0, 0, 1);
}

void introGraphics(int16_t x, int16_t y)
{
    int i;

    /* logo */
    copyimage(x - splash[0].width / 2, y - splash[0].height / 2, 0);

    /* frame */
    for (i = 0; i <= 20; i += 10) {
        drawBox(i, i, GRAPHICS_RIGHT - i, GRAPHICS_BOTTOM - i);
    }
}

void introText(int16_t x, int16_t y)
{
    write_string("v 0.0.1", x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_CENTER, 0, 3);
}

void showVideoType(int16_t x, int16_t y)
{
    if (PIOS_Video_GetType() == VIDEO_TYPE_NTSC) {
        write_string("NTSC", x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_CENTER, 0, 3);
    } else {
        write_string("PAL", x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_CENTER, 0, 3);
    }
}

void calcHomeArrow(int16_t m_yaw)
{
    HomeLocationData home;

    HomeLocationGet(&home);
    GPSPositionSensorData gpsData;
    GPSPositionSensorGet(&gpsData);

    /** http://www.movable-type.co.uk/scripts/latlong.html **/
    float lat1, lat2, lon1, lon2, a, c, d, x, y, brng, u2g;
    float elevation;
    float gcsAlt = home.Altitude; // Home MSL altitude
    float uavAlt = gpsData.Altitude; // UAV MSL altitude
    float dAlt   = uavAlt - gcsAlt; // Altitude difference

    // Convert to radians
    lat1 = DEG2RAD(home.Latitude) / 10000000.0f; // Home lat
    lon1 = DEG2RAD(home.Longitude) / 10000000.0f; // Home lon
    lat2 = DEG2RAD(gpsData.Latitude) / 10000000.0f; // UAV lat
    lon2 = DEG2RAD(gpsData.Longitude) / 10000000.0f; // UAV lon

    // Bearing
    /**
       var y = Math.sin(dLon) * Math.cos(lat2);
       var x = Math.cos(lat1)*Math.sin(lat2) -
       Math.sin(lat1)*Math.cos(lat2)*Math.cos(dLon);
       var brng = Math.atan2(y, x).toDeg();
     **/
    y    = sinf(lon2 - lon1) * cosf(lat2);
    x    = cosf(lat1) * sinf(lat2) - sinf(lat1) * cosf(lat2) * cosf(lon2 - lon1);
    brng = RAD2DEG(atan2f(y, x));
    if (brng < 0) {
        brng += 360.0f;
    }

    // yaw corrected bearing, needs compass
    u2g = brng - 180.0f - m_yaw;
    if (u2g < 0) {
        u2g += 360.0f;
    }

    // Haversine formula for distance
    /**
       var R = 6371; // km
       var dLat = (lat2-lat1).toRad();
       var dLon = (lon2-lon1).toRad();
       var a = Math.sin(dLat/2) * Math.sin(dLat/2) +
       Math.cos(lat1.toRad()) * Math.cos(lat2.toRad()) *
       Math.sin(dLon/2) * Math.sin(dLon/2);
       var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
       var d = R * c;
     **/
    a = sinf((lat2 - lat1) / 2) * sinf((lat2 - lat1) / 2) + cosf(lat1) * cosf(lat2) * sinf((lon2 - lon1) / 2) * sinf((lon2 - lon1) / 2);
    c = 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));
    d = 6371.0f * 1000.0f * c;

    // Elevation  v depends servo direction
    if (d > 0.0f) {
        elevation = 90.0f - RAD2DEG(atanf(dAlt / d));
    } else {
        elevation = 0.0f;
    }
    // ! TODO: sanity check

    char temp[50] =
    { 0 };
    sprintf(temp, "hea:%d", (int)brng);
    write_string(temp, GRAPHICS_RIGHT / 2 - 30, 30, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
    sprintf(temp, "ele:%d", (int)elevation);
    write_string(temp, GRAPHICS_RIGHT / 2 - 30, 40, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
    sprintf(temp, "dis:%d", (int)d);
    write_string(temp, GRAPHICS_RIGHT / 2 - 30, 50, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
    sprintf(temp, "u2g:%d", (int)u2g);
    write_string(temp, GRAPHICS_RIGHT / 2 - 30, 60, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);

    sprintf(temp, "%c%c", (int)(u2g / 22.5f) * 2 + 0x90, (int)(u2g / 22.5f) * 2 + 0x91);
    write_string(temp, 250, 60, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 3);
}


// check for stable gps as home position
// criteria for a stable home position:
// - GPS status > GPSPOSITIONSENSOR_STATUS_FIX2D
// - with at least CHECK_HOME_MIN_SATS satellites
// - and gpsData->Altitude delta is lower CHECK_HOME_MAX_DEV m for CHECK_HOME_STABLE ms
#define CHECK_HOME_MIN_SATS 5
#define CHECK_HOME_MAX_DEV  0.5f            // [m]
#define CHECK_HOME_STABLE   3000            // [ms]
void check_gps_home(HomePosition *homePos, GPSPositionSensorData *gpsData)
{
    static portTickType stable_time = 0;
    portTickType current_time = xTaskGetTickCount();
    static float alt_prev     = 0.0f;

    if (!homePos->GotHome && gpsData->Status > GPSPOSITIONSENSOR_STATUS_FIX2D && gpsData->Satellites >= CHECK_HOME_MIN_SATS) {
        if (fabsf(alt_prev - gpsData->Altitude) > CHECK_HOME_MAX_DEV) {
            stable_time = current_time;
            alt_prev    = gpsData->Altitude;
        } else {
            if (current_time - stable_time > CHECK_HOME_STABLE) {
                homePos->Latitude  = gpsData->Latitude;          // take this Latitude as home Latitude
                homePos->Longitude = gpsData->Longitude; // take this Longitude as home Longitude
                homePos->Altitude  = gpsData->Altitude;          // take this stable Altitude as home Altitude
                homePos->GotHome   = TRUE;                                        // we got home
            }
        }
    }
}


// calculate home distance and direction
void calc_home_data(HomePosition *homePos, GPSPositionSensorData *gpsData)
{
    // shrinking factor for longitude going to poles direction
    float rad = DEG2RAD(fabsf((float)homePos->Latitude / 10000000.0f));
    float scaleLongDown = cosf(rad);
    float scaleLongUp   = 1.0f / scaleLongDown;

    // deltas
    float dLat   = (float)(homePos->Latitude - gpsData->Latitude) / 10000000.0f;
    float dLon   = (float)(homePos->Longitude - gpsData->Longitude) / 10000000.0f;

    // distance to home
    float dstlat = fabsf(dLat) * 111319.5f;
    float dstlon = fabsf(dLon) * 111319.5f * scaleLongDown;

    homePos->Distance = (uint32_t)sqrtf(dstlat * dstlat + dstlon * dstlon);

    // direction to home
    dstlat = dLat * scaleLongUp;
    dstlon = dLon;
    int16_t direction = (int16_t)(90.0f + RAD2DEG(atan2f(dstlat, -dstlon))); // absolut home direction
    if (direction < 0) {
        direction += 360; // normalization
    }
    direction = direction - 180; // absolut return direction
    if (direction < 0) {
        direction += 360; // normalization
    }
    direction = direction - (int16_t)gpsData->Heading; // relative home direction
    if (direction < 0) {
        direction += 360; // normalization
    }
    homePos->Direction = (uint16_t)direction;
}


uint8_t check_enable_and_srceen(uint8_t info, OsdSettingsWarningsSetupData *setup, uint8_t screen, int16_t *x, int16_t *y)
{
    if (!info) {
        return 0;
    }

    switch (screen) {
    case 1:
        if (setup->XOffset1 > -1000 && setup->YOffset1 > -1000) {
            *x = setup->XOffset1;
            *y = setup->YOffset1;
            return info;
        }
        break;
    case 2:
        if (setup->XOffset2 > -1000 && setup->YOffset2 > -1000) {
            *x = setup->XOffset2;
            *y = setup->YOffset2;
            return info;
        }
        break;
    case 3:
        if (setup->XOffset3 > -1000 && setup->YOffset3 > -1000) {
            *x = setup->XOffset3;
            *y = setup->YOffset3;
            return info;
        }
        break;
    default:
        return 0;

        break;
    }

    return 0;
}


#define ACCUMULATE_MIN_PERIOD 35
void accumulate_current(double current_amp, double *current_total)
{
    static portTickType callTimer = 0;
    portTickType current_ms;
    portTickType delta_ms;

    current_ms = xTaskGetTickCount();

    if (callTimer + ACCUMULATE_MIN_PERIOD <= current_ms) {
        delta_ms  = current_ms - callTimer;
        callTimer = current_ms;
        *current_total += current_amp * (double)delta_ms * 0.0002778d; // .0002778 is 1/3600 (conversion to hours)
    }
}


#define WARN_ON_TIME         500                    // [ms]
#define WARN_OFF_TIME        250                    // [ms]
#define WARN_DO_NOT_MOVE     0x0001
#define WARN_NO_SAT_FIX      0x0002
#define WARN_HOME_NOT_SET    0x0004
#define WARN_DISARMED        0x0008
#define WARN_RSSI_LOW        0x0010
#define WARN_BATT_FLIGHT_LOW 0x0020
#define WARN_BATT_VIDEO_LOW  0x0040
#define WARN_BATT_SCURR_HIGH 0x0080
#define WARN_BATT_SVOLT_LOW  0x0100
void draw_warnings(uint32_t WarnMask, int16_t x, int16_t y, int8_t v_spacing, int8_t char_size)
{
    static portTickType on_off_time = 0;
    portTickType current_time = xTaskGetTickCount();
    char temp[20] = { 0 };
    int d_y = 0;

    if (!WarnMask || current_time - on_off_time > WARN_ON_TIME + WARN_OFF_TIME) {
        on_off_time = current_time;
    }

    if (WarnMask && current_time - on_off_time < WARN_ON_TIME) {
        if (WarnMask & WARN_DO_NOT_MOVE) {
            sprintf(temp, "DO NOT MOVE");
            write_string(temp, x, y + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, char_size);
            d_y += v_spacing;
        }
        if (WarnMask & WARN_NO_SAT_FIX) {
            sprintf(temp, "NO SAT FIX");
            write_string(temp, x, y + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, char_size);
            d_y += v_spacing;
        }
        if (WarnMask & WARN_HOME_NOT_SET) {
            sprintf(temp, "HOME NOT SET");
            write_string(temp, x, y + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, char_size);
            d_y += v_spacing;
        }
        if (WarnMask & WARN_DISARMED) {
            sprintf(temp, "DISARMED");
            write_string(temp, x, y + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, char_size);
            d_y += v_spacing;
        }
        if (WarnMask & WARN_RSSI_LOW) {
            sprintf(temp, "RSSI LOW");
            write_string(temp, x, y + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, char_size);
            d_y += v_spacing;
        }
        if (WarnMask & WARN_BATT_FLIGHT_LOW) {
            sprintf(temp, "FLIGHT BATT LOW");
            write_string(temp, x, y + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, char_size);
            d_y += v_spacing;
        }
        if (WarnMask & WARN_BATT_VIDEO_LOW) {
            sprintf(temp, "VIDEO BATT LOW");
            write_string(temp, x, y + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, char_size);
            d_y += v_spacing;
        }
        if (WarnMask & WARN_BATT_SCURR_HIGH) {
            sprintf(temp, "CURRENT HIGH");
            write_string(temp, x, y + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, char_size);
            d_y += v_spacing;
        }
        if (WarnMask & WARN_BATT_SVOLT_LOW) {
            sprintf(temp, "VOLTAGE LOW");
            write_string(temp, x, y + d_y, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, char_size);
            d_y += v_spacing;
        }
    }
}


void draw_flight_mode(uint8_t FlightMode, int16_t x, int16_t y, int8_t char_size)
{
    char temp[20] = { 0 };

    switch (FlightMode) {
    case FLIGHTSTATUS_FLIGHTMODE_MANUAL:
        sprintf(temp, "Man");
        break;
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED1:
        sprintf(temp, "Stab1");
        break;
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED2:
        sprintf(temp, "Stab2");
        break;
    case FLIGHTSTATUS_FLIGHTMODE_STABILIZED3:
        sprintf(temp, "Stab3");
        break;
    case FLIGHTSTATUS_FLIGHTMODE_AUTOTUNE:
        sprintf(temp, "Tune");
        break;
    case FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD:
        sprintf(temp, "AHold");
        break;
    case FLIGHTSTATUS_FLIGHTMODE_ALTITUDEVARIO:
        sprintf(temp, "AVario");
        break;
    case FLIGHTSTATUS_FLIGHTMODE_VELOCITYCONTROL:
        sprintf(temp, "VCont");
        break;
    case FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD:
        sprintf(temp, "PH");
        break;
    case FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE:
        sprintf(temp, "RTB");
        break;
    case FLIGHTSTATUS_FLIGHTMODE_LAND:
        sprintf(temp, "Land");
        break;
    case FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER:
        sprintf(temp, "Path");
        break;
    case FLIGHTSTATUS_FLIGHTMODE_POI:
        sprintf(temp, "POI");
        break;
    default:
        sprintf(temp, "Mode%2d", FlightMode);
        break;
    }
    write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, char_size);
}


void updateGraphics()
{
    OsdSettingsData OsdSettings;

    OsdSettingsGet(&OsdSettings);
    OsdSettings2Data OsdSettings2;
    OsdSettings2Get(&OsdSettings2);
    AttitudeStateData attitude;
    AttitudeStateGet(&attitude);
    GPSPositionSensorData gpsData;
    GPSPositionSensorGet(&gpsData);
    GPSVelocitySensorData gpsVelocityData;
    GPSVelocitySensorGet(&gpsVelocityData);
    HomeLocationData home;
    HomeLocationGet(&home);
    BaroSensorData baro;
    BaroSensorGet(&baro);
    FlightStatusData status;
    FlightStatusGet(&status);
    ManualControlCommandData mcc;
    ManualControlCommandGet(&mcc);
#ifdef DEBUG_TELEMETRY
    FlightTelemetryStatsData f_telemetry;
    FlightTelemetryStatsGet(&f_telemetry);
    GCSTelemetryStatsData g_telemetry;
    GCSTelemetryStatsGet(&g_telemetry);
#endif

    switch (OsdSettings.Screen) {
    // show main flight screen
    case 0:
    {
#ifdef SIMULATE_DATA
        static int alt = 0;
        gpsData.Altitude    = alt++;
        static int spd = 0;
        gpsData.Groundspeed = spd++;
#endif

        static uint8_t power_on_time = 0;
        static uint8_t airborne = FALSE;
        static double current_total  = 0;                // accumulated sensor current [mAh]
        static HomePosition homePos;
        static ADCfiltered filteredADC;
        char temp[50]     = { 0 };
        int8_t screen     = 2;
        int8_t check;
        int16_t x, y;
        uint32_t WarnMask = 0;
        Unit *convert;

#ifdef TEMP_GPS_STATUS_WORKAROUND
        static uint8_t gps_status = 0;
        if (gpsData.Status == GPSPOSITIONSENSOR_STATUS_FIX3D && gpsData.Satellites >= 5) {
            gps_status = GPSPOSITIONSENSOR_STATUS_FIX3D;
        }
        if (gps_status == GPSPOSITIONSENSOR_STATUS_FIX3D && gpsData.Satellites >= 3) {
            gpsData.Status = GPSPOSITIONSENSOR_STATUS_FIX3D;
        }
#endif

        // JR_HINT TODO
        // RSSI version PacketRxOk
        // RSSI version Scherrer digital
        // use nice icons for some of the displayed values
        // use homePos.Altitude for baro.Altitude correction?

        // Screen switching via RC-RX or GCS
        if (mcc.Connected) {
            if (mcc.Channel[OsdSettings.ScreenSwitching.SwitchChannel] < OsdSettings.ScreenSwitching.Switch1Pulse) {
                screen = 1;
            }
            if (mcc.Channel[OsdSettings.ScreenSwitching.SwitchChannel] > OsdSettings.ScreenSwitching.Switch3Pulse) {
                screen = 3;
            }
        } else {
            screen = OsdSettings.ScreenSwitching.UnconnectedScreen;
        }

        // Set the units to metric or imperial
        convert = OsdSettings.Units == OSDSETTINGS_UNITS_METRIC ? &Convert[0] : &Convert[1];

        // Home position calculations
        if (homePos.GotHome) {
            calc_home_data(&homePos, &gpsData);
        } else {
            if (OsdSettings.HomeSource == OSDSETTINGS_HOMESOURCE_CONFIG && home.Set == HOMELOCATION_SET_TRUE) {
                homePos.Latitude  = home.Latitude;
                homePos.Longitude = home.Longitude;
                homePos.Altitude  = home.Altitude;
                homePos.GotHome   = TRUE;
            } else {
                check_gps_home(&homePos, &gpsData);
            }
        }

        // Draw AH first so that it is underneath everything else
        // Artificial horizon in HUD design (centered relative to x, y)
        if (check_enable_and_srceen(OsdSettings.ArtificialHorizon, (OsdSettingsWarningsSetupData *)&OsdSettings.ArtificialHorizonSetup, screen, &x, &y)) {
#if 1
            hud_draw_artificial_horizon(attitude.Roll, attitude.Pitch, attitude.Yaw, GRAPHICS_X_MIDDLE + x, GRAPHICS_Y_MIDDLE + y, OsdSettings.ArtificialHorizonSetup.MaxPitchVisible, OsdSettings.ArtificialHorizonSetup.DeltaDegree, OsdSettings.ArtificialHorizonSetup.MainLineWidth, 100);
#else
            sprintf(temp, "Roll: %7.2f", (double)attitude.Roll);
            write_string(temp, 200, 100, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, 3);
            sprintf(temp, "Pitch:%7.2f", (double)attitude.Pitch);
            write_string(temp, 200, 120, 0, 0, TEXT_VA_MIDDLE, TEXT_HA_CENTER, 0, 3);
#endif
        }
        // GPS coordinates
        if (check_enable_and_srceen(OsdSettings.GPSLatitude, (OsdSettingsWarningsSetupData *)&OsdSettings.GPSLatitudeSetup, screen, &x, &y)) {
            sprintf(temp, "Lat%11.6f", homePos.GotHome ? (double)(gpsData.Latitude / 10000000.0f + OsdSettings2.PositionStealth) : (double)0.0f);
            write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings.GPSLatitudeSetup.CharSize);
        }
        if (check_enable_and_srceen(OsdSettings.GPSLongitude, (OsdSettingsWarningsSetupData *)&OsdSettings.GPSLongitudeSetup, screen, &x, &y)) {
            sprintf(temp, "Lon%11.6f", homePos.GotHome ? (double)(gpsData.Longitude / 10000000.0f + OsdSettings2.PositionStealth) : (double)0.0f);
            write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings.GPSLongitudeSetup.CharSize);
        }
        // GPS satellite info
        if (check_enable_and_srceen(OsdSettings.GPSSatInfo, (OsdSettingsWarningsSetupData *)&OsdSettings.GPSSatInfoSetup, screen, &x, &y)) {
            uint8_t fix = gpsData.Status < GPSPOSITIONSENSOR_STATUS_FIX2D ? '-' : gpsData.Status - 1;
            sprintf(temp, "Sat%4d%c", gpsData.Satellites, fix);
            write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings.GPSSatInfoSetup.CharSize);
        }
        // Ground speed in HUD design as vertical scale left side (centered relative to y)
        if (check_enable_and_srceen(OsdSettings.Speed, (OsdSettingsWarningsSetupData *)&OsdSettings.SpeedSetup, screen, &x, &y)) {
            hud_draw_vertical_scale((int)(gpsData.Groundspeed * convert->ms_to_kmh_mph), 100, OsdSettings.SpeedSetup.Orientation, GRAPHICS_X_MIDDLE + x, GRAPHICS_Y_MIDDLE + y, 100, 10, 20, 5, 8, 11, 100, HUD_VSCALE_FLAG_NO_NEGATIVE);
        }
        // Home altitude in HUD design as vertical scale right side (centered relative to y)
        if (check_enable_and_srceen(OsdSettings.Altitude, (OsdSettingsWarningsSetupData *)&OsdSettings.AltitudeSetup, screen, &x, &y)) {
            hud_draw_vertical_scale(OsdSettings.AltitudeSource == OSDSETTINGS_ALTITUDESOURCE_GPS ? (int)((gpsData.Altitude - homePos.Altitude) * convert->m_to_m_feet) : (int)(baro.Altitude * convert->m_to_m_feet), 100, OsdSettings.AltitudeSetup.Orientation, GRAPHICS_X_MIDDLE + x, GRAPHICS_Y_MIDDLE + y, 100, 10, 20, 5, 8, 11, 100, 0);
        }
        // Heading in HUD design (centered relative to x)
        // JR_HINT TODO use and test mag heading in-flight
        if (check_enable_and_srceen(OsdSettings.Heading, (OsdSettingsWarningsSetupData *)&OsdSettings.HeadingSetup, screen, &x, &y)) {
            int16_t heading = OsdSettings.HeadingSource == OSDSETTINGS_HEADINGSOURCE_GPS ? (int16_t)gpsData.Heading : (int16_t)attitude.Yaw;
            hud_draw_linear_compass(heading < 0 ? heading + 360 : heading, 150, 120, GRAPHICS_X_MIDDLE + x, GRAPHICS_Y_MIDDLE + y, 15, 30, 5, 8, 0);
        }
        // Home direction visualization
        if (check_enable_and_srceen(OsdSettings.HomeArrow, (OsdSettingsWarningsSetupData *)&OsdSettings.HomeArrowSetup, screen, &x, &y)) {
            drawArrow(GRAPHICS_X_MIDDLE + x, GRAPHICS_Y_MIDDLE + y, homePos.Direction, OsdSettings.HomeArrowSetup.Size);
        }
        // Home distance
        if (check_enable_and_srceen(OsdSettings.HomeDistance, (OsdSettingsWarningsSetupData *)&OsdSettings.HomeDistanceSetup, screen, &x, &y)) {
            int d = (int)(homePos.Distance * convert->m_to_m_feet);
            if (homePos.GotHome) {
                sprintf(temp, "HD%5d%c", d <= 99999 ? d : 99999, convert->char_m_feet);
            } else {
                sprintf(temp, "HD  ---%c", convert->char_m_feet);
            }
            write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings.HomeDistanceSetup.CharSize);
        }
        // Vertical speed
        if (check_enable_and_srceen(OsdSettings.VerticalSpeed, (OsdSettingsWarningsSetupData *)&OsdSettings.VerticalSpeedSetup, screen, &x, &y)) {
            sprintf(temp, "VS%5.1f%c", (double)-gpsVelocityData.Down, 0x88); // TODO currently m/s for both
            // sprintf(temp, "VS%5.1f%c", (double)(-gpsVelocityData.Down * convert->ms_to_ms_fts), convert->char_ms_fts);	// TODO is ft/s or ft/m the common unit for imperial?
            write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings.VerticalSpeedSetup.CharSize);
        }
        // Flight mode
        if (check_enable_and_srceen(OsdSettings.FlightMode, (OsdSettingsWarningsSetupData *)&OsdSettings.FlightModeSetup, screen, &x, &y)) {
            draw_flight_mode(status.FlightMode, x, y, OsdSettings.FlightModeSetup.CharSize);
        }
        // Throttle
        if (check_enable_and_srceen(OsdSettings.Throttle, (OsdSettingsWarningsSetupData *)&OsdSettings.ThrottleSetup, screen, &x, &y)) {
            int throttle = (int)(mcc.Throttle * 100.0f);
            sprintf(temp, "Thr%4d%c", throttle < 0 ? 0 : throttle, '%');
            write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings.ThrottleSetup.CharSize);
        }
        // Flight time
        check = check_enable_and_srceen(OsdSettings.Time, (OsdSettingsWarningsSetupData *)&OsdSettings.TimeSetup, screen, &x, &y);
        if (check == OSDSETTINGS_TIME_HOURMINSEC) {
            sprintf(temp, "%02d:%02d:%02d", timex.hour, timex.min, timex.sec);
        }
        if (check == OSDSETTINGS_TIME_MINSEC) {
            sprintf(temp, "FT%02d:%02d", timex.min, timex.sec);
        }
        if (check != OSDSETTINGS_TIME_DISABLED) {
            write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings.TimeSetup.CharSize);
        }

#define ADC_FILTER     (double)0.1f
#define ADC_REFERENCE  3.0f
#define ADC_RESOLUTION 4096.0f
#define ADC_VOLT       0
#define ADC_CURR       1
#define ADC_FLIGHT     2
#define ADC_TEMP       3
#define ADC_VIDEO      4
#define ADC_RSSI       5
#define ADC_VREF       6
        // ADC RSSI
        if (OsdSettings2.RSSI) {
            filteredADC.rssi = filteredADC.rssi * ((double)1.0f - ADC_FILTER) + (double)(PIOS_ADC_PinGet(ADC_RSSI) * ADC_REFERENCE * OsdSettings2.RSSICalibration.Factor / ADC_RESOLUTION + OsdSettings2.RSSICalibration.Offset) * ADC_FILTER;
            WarnMask |= filteredADC.rssi < (double)OsdSettings2.RSSICalibration.Warning ? WARN_RSSI_LOW : 0x00;
            check     = check_enable_and_srceen(OsdSettings2.RSSI, (OsdSettingsWarningsSetupData *)&OsdSettings2.RSSISetup, screen, &x, &y);
            if (check == OSDSETTINGS2_RSSI_ANALOG) {
                sprintf(temp, "RI%5.2f%%", filteredADC.rssi);
            }
            if (check == OSDSETTINGS2_RSSI_PWM) {
                sprintf(temp, "RI ----%%"); // JR_HINT TODO
            }
            if (check != OSDSETTINGS2_RSSI_DISABLED) {
                write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings2.RSSISetup.CharSize);
            }
        }
        // ADC Flight
        if (OsdSettings2.FlightVoltage) {
            filteredADC.flight = filteredADC.flight * ((double)1.0f - ADC_FILTER) + (double)(PIOS_ADC_PinGet(ADC_FLIGHT) * ADC_REFERENCE * OsdSettings2.FlightVoltageCalibration.Factor / ADC_RESOLUTION + OsdSettings2.FlightVoltageCalibration.Offset) * ADC_FILTER;
            WarnMask |= filteredADC.flight < (double)OsdSettings2.FlightVoltageCalibration.Warning ? WARN_BATT_FLIGHT_LOW : 0x00;
            if (check_enable_and_srceen(OsdSettings2.FlightVoltage, (OsdSettingsWarningsSetupData *)&OsdSettings2.FlightVoltageSetup, screen, &x, &y)) {
                sprintf(temp, "FV%5.2fV", filteredADC.flight);
                write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings2.FlightVoltageSetup.CharSize);
            }
        }
        // ADC Video
        if (OsdSettings2.VideoVoltage) {
            filteredADC.video = filteredADC.video * ((double)1.0f - ADC_FILTER) + (double)(PIOS_ADC_PinGet(ADC_VIDEO) * ADC_REFERENCE * OsdSettings2.VideoVoltageCalibration.Factor / ADC_RESOLUTION + OsdSettings2.VideoVoltageCalibration.Offset) * ADC_FILTER;
            WarnMask |= filteredADC.video < (double)OsdSettings2.VideoVoltageCalibration.Warning ? WARN_BATT_VIDEO_LOW : 0x00;
            if (check_enable_and_srceen(OsdSettings2.VideoVoltage, (OsdSettingsWarningsSetupData *)&OsdSettings2.VideoVoltageSetup, screen, &x, &y)) {
                sprintf(temp, "VV%5.2fV", filteredADC.video);
                write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings2.VideoVoltageSetup.CharSize);
            }
        }
        // ADC Sensor voltage
        if (OsdSettings2.SensorVoltage) {
            filteredADC.volt = filteredADC.volt * ((double)1.0f - ADC_FILTER) + (double)(PIOS_ADC_PinGet(ADC_VOLT) * ADC_REFERENCE * OsdSettings2.SensorVoltageCalibration.Factor / ADC_RESOLUTION + OsdSettings2.SensorVoltageCalibration.Offset) * ADC_FILTER;
            WarnMask |= filteredADC.volt < (double)OsdSettings2.SensorVoltageCalibration.Warning ? WARN_BATT_SVOLT_LOW : 0x00;
            if (check_enable_and_srceen(OsdSettings2.SensorVoltage, (OsdSettingsWarningsSetupData *)&OsdSettings2.SensorVoltageSetup, screen, &x, &y)) {
                sprintf(temp, "SV%5.2fV", filteredADC.volt);
                write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings2.SensorVoltageSetup.CharSize);
            }
        }
        // ADC Sensor current or ADC Sensor current consumed
        if (OsdSettings2.SensorCurrent || OsdSettings2.SensorCurrentConsumed || OsdSettings2.AirborneResetTime) {
            filteredADC.curr = filteredADC.curr * ((double)1.0f - ADC_FILTER) + (double)(PIOS_ADC_PinGet(ADC_CURR) * ADC_REFERENCE * OsdSettings2.SensorCurrentCalibration.Factor / ADC_RESOLUTION + OsdSettings2.SensorCurrentCalibration.Offset) * ADC_FILTER;
        }
        // ADC Sensor current
        if (OsdSettings2.SensorCurrent) {
            WarnMask |= filteredADC.curr > (double)OsdSettings2.SensorCurrentCalibration.Warning ? WARN_BATT_SCURR_HIGH : 0x00;
            if (check_enable_and_srceen(OsdSettings2.SensorCurrent, (OsdSettingsWarningsSetupData *)&OsdSettings2.SensorCurrentSetup, screen, &x, &y)) {
                sprintf(temp, "SC%5.2fA", filteredADC.curr);
                write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings2.SensorCurrentSetup.CharSize);
            }
        }
        // ADC Sensor current consumed
        if (OsdSettings2.SensorCurrentConsumed) {
            accumulate_current(filteredADC.curr, &current_total);
            if (check_enable_and_srceen(OsdSettings2.SensorCurrentConsumed, (OsdSettingsWarningsSetupData *)&OsdSettings2.SensorCurrentConsumedSetup, screen, &x, &y)) {
                sprintf(temp, "T%4dmAh", (int)current_total);
                write_string(temp, x, y, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, OsdSettings2.SensorCurrentConsumedSetup.CharSize);
            }
        }
        // Airborne reset time
        if (OsdSettings2.AirborneResetTime && !airborne && filteredADC.curr >= (double)OsdSettings2.AirborneResetTimeCurrent) {
            airborne   = TRUE;
            timex.sec  = 0;
            timex.min  = 0;
            timex.hour = 0;
        }

#define DO_NOT_MOVE_SECONDS 10
        // Draw warnings last so that they are above everything else
        // Warnings (centered relative to x)
        if (check_enable_and_srceen(OsdSettings.Warnings, (OsdSettingsWarningsSetupData *)&OsdSettings.WarningsSetup, screen, &x, &y)) {
            power_on_time = power_on_time < DO_NOT_MOVE_SECONDS ? timex.sec : DO_NOT_MOVE_SECONDS;
            WarnMask |= power_on_time < DO_NOT_MOVE_SECONDS ? WARN_DO_NOT_MOVE : 0x00;
            WarnMask |= gpsData.Status < GPSPOSITIONSENSOR_STATUS_FIX3D ? WARN_NO_SAT_FIX : 0x00;
            WarnMask |= !homePos.GotHome && home.Set == HOMELOCATION_SET_FALSE ? WARN_HOME_NOT_SET : 0x00;
            WarnMask |= status.Armed < FLIGHTSTATUS_ARMED_ARMED ? WARN_DISARMED : 0x00;
            draw_warnings(OsdSettings.WarningsSetup.Mask & WarnMask, GRAPHICS_X_MIDDLE + x, GRAPHICS_Y_MIDDLE + y, OsdSettings.WarningsSetup.VerticalSpacing, OsdSettings.WarningsSetup.CharSize);
        }

#ifdef DEBUG_TIMING
        // show in time
        sprintf(temp, "T.in: %3dms", in_time);
        write_string(temp, 50, 30, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
        // show out time
        sprintf(temp, "T.out:%3dms", out_time);
        write_string(temp, 50, 40, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
#endif

#ifdef DEBUG_ALARMS
        // show alarms
        int k;
        for (k = 0; k <= 17; k++) {
            SystemAlarmsAlarmOptions alarm = AlarmsGet(k);
            if (alarm > SYSTEMALARMS_ALARM_OK) {
                sprintf(temp, "A%2d:%d", k, alarm);
                write_string(temp, 50, 50 + 10 * k, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
            }
        }
#endif

#ifdef DEBUG_TELEMETRY
#define DEBUG_TELEMETRY_ON_TIME 5000 // [ms]
        static portTickType on_time = 0;
        portTickType current_time   = xTaskGetTickCount();
        static uint8_t fts   = 10;
        static uint8_t gts   = 10;
        static uint32_t frxf = 0;
        static uint32_t ftxf = 0;
        static uint32_t grxf = 0;
        static uint32_t gtxf = 0;

        if (
            fts != f_telemetry.Status
            || gts != g_telemetry.Status
            || frxf != f_telemetry.RxFailures
            || ftxf != f_telemetry.TxFailures
            || grxf != g_telemetry.RxFailures
            || gtxf != g_telemetry.TxFailures
            ) {
            on_time = current_time;
            fts     = f_telemetry.Status;
            gts     = g_telemetry.Status;
            frxf    = f_telemetry.RxFailures;
            ftxf    = f_telemetry.TxFailures;
            grxf    = g_telemetry.RxFailures;
            gtxf    = g_telemetry.TxFailures;
        }

        if (current_time - on_time < DEBUG_TELEMETRY_ON_TIME) {
            sprintf(temp, "FTS: %6d", f_telemetry.Status);
            write_string(temp, 270, 30, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);

            sprintf(temp, "GTS: %6d", g_telemetry.Status);
            write_string(temp, 270, 40, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);

            sprintf(temp, "FRXF:%6d", (int)f_telemetry.RxFailures);
            write_string(temp, 270, 50, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);

            sprintf(temp, "FTXF:%6d", (int)f_telemetry.TxFailures);
            write_string(temp, 270, 60, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);

            sprintf(temp, "GRXF:%6d", (int)g_telemetry.RxFailures);
            write_string(temp, 270, 70, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);

            sprintf(temp, "GTXF:%6d", (int)g_telemetry.TxFailures);
            write_string(temp, 270, 80, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
        }
#endif /* ifdef DEBUG_TELEMETRY */

#ifdef DEBUG_BLACK_WHITE
        int bw;
        for (bw = 0; bw < 20; bw++) {
            write_hline_lm(140, 259, 30 + bw, 1, 1);
            write_hline_lm(140, 259, 50 + bw, 0, 1);
        }
#endif

#ifdef DEBUG_STUFF
        // show heap
        sprintf(temp, "Heap:%6d", xPortGetFreeHeapSize());
        write_string(temp, 250, 30, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
        // show detected video type
        sprintf(temp, "V.type:%4s", PIOS_Video_GetType() == VIDEO_TYPE_NTSC ? "NTSC" : "PAL");
        write_string(temp, 250, 40, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
        // show number of video columns
        sprintf(temp, "Columns:%3d", GRAPHICS_RIGHT + 1);
        write_string(temp, 250, 50, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
        // show number of detected video lines
        sprintf(temp, "Lines:%5d", PIOS_Video_GetLines());
        write_string(temp, 250, 60, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
#endif
    }
    break;
    // show fonts
    case 1:
    case 2:
    case 3:
    case 4:
    {
        char temp[10] = { 0 };
        int f, i, j;
        f = OsdSettings.Screen - 1;
        sprintf(temp, "Font: %d", f);
        write_string(temp, 10, 0, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, 2);
        for (i = 0; i < 16; i++) {
            sprintf(temp, "%03d:", i * 16);
            write_string(temp, 10, 15 + i * 17, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, f);
            for (j = 0; j < 16; j++) {
                sprintf(temp, "%c", j + i * 16);
                write_string(temp, 60 + j * 20, 15 + i * 17, 0, 0, TEXT_VA_TOP, TEXT_HA_LEFT, 0, f);
            }
        }
    }
    break;
    // show splash graphics
    case 5:
    case 6:
    case 7:
    {
        int image = OsdSettings.Screen - 5;
        struct splashEntry splash_info;
        splash_info = splash[image];
        copyimage(GRAPHICS_RIGHT / 2 - (splash_info.width) / 2, GRAPHICS_BOTTOM / 2 - (splash_info.height) / 2, image);
    }
    break;
    // show grid
    default:
    {
        int i;
        for (i = 0; i < GRAPHICS_RIGHT / 2; i += 16) {
            write_vline_lm(GRAPHICS_RIGHT / 2 + i, 0, GRAPHICS_BOTTOM, 1, 1);
            write_vline_lm(GRAPHICS_RIGHT / 2 - i, 0, GRAPHICS_BOTTOM, 1, 1);
        }
        for (i = 0; i < GRAPHICS_BOTTOM / 2; i += 16) {
            write_hline_lm(0, GRAPHICS_RIGHT, GRAPHICS_BOTTOM / 2 + i, 1, 1);
            write_hline_lm(0, GRAPHICS_RIGHT, GRAPHICS_BOTTOM / 2 - i, 1, 1);
        }
        drawBox(0, 0, GRAPHICS_RIGHT, GRAPHICS_BOTTOM);
    }
    break;
    }
}


/**
 * Start the osd module
 */
int32_t osdgenStart(void)
{
    vSemaphoreCreateBinary(osdSemaphore);
    xTaskCreate(osdgenTask, (signed char *)"OSDGEN", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &osdgenTaskHandle);
    PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_OSDGEN, osdgenTaskHandle);
#ifdef PIOS_INCLUDE_WDG
    PIOS_WDG_RegisterFlag(PIOS_WDG_OSDGEN);
#endif

    return 0;
}


/**
 * Initialise the osd module
 */
int32_t osdgenInitialize(void)
{
    AttitudeStateInitialize();
    GPSPositionSensorInitialize();
    GPSVelocitySensorInitialize();
    GPSTimeInitialize();
    GPSSatellitesInitialize();
    HomeLocationInitialize();
    OsdSettingsInitialize();
    OsdSettings2Initialize();
    BaroSensorInitialize();
    FlightStatusInitialize();
    ManualControlCommandInitialize();
    TaskInfoInitialize();

#if 0 // JR_HINT an idea
    UAVObjMetadata metadata;
    metadata.flags =
        ACCESS_READWRITE << UAVOBJ_ACCESS_SHIFT |
        ACCESS_READWRITE << UAVOBJ_GCS_ACCESS_SHIFT |
        0 << UAVOBJ_TELEMETRY_ACKED_SHIFT |
        0 << UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT |
        UPDATEMODE_MANUAL << UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT |
        UPDATEMODE_MANUAL << UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT;
    metadata.telemetryUpdatePeriod    = 0;
    metadata.gcsTelemetryUpdatePeriod = 0;
    metadata.loggingUpdatePeriod = 0;

    // OSD listen only mode
    FlightStatusSetMetadata(&metadata);
    AttitudeStateSetMetadata(&metadata);
    GPSPositionSensorSetMetadata(&metadata);
    GPSVelocitySensorSetMetadata(&metadata);
    GPSTimeSetMetadata(&metadata);
    GPSSatellitesSetMetadata(&metadata);
    HomeLocationSetMetadata(&metadata);
    BaroSensorSetMetadata(&metadata);
    ManualControlCommandSetMetadata(&metadata);
    TaskInfoSetMetadata(&metadata);
#endif /* if 0 */

#if 0 // JR_HINT an idea
    UAVObjMetadata metadata;
    metadata.flags =
        ACCESS_READWRITE << UAVOBJ_ACCESS_SHIFT |
        ACCESS_READWRITE << UAVOBJ_GCS_ACCESS_SHIFT |
        0 << UAVOBJ_TELEMETRY_ACKED_SHIFT |
        0 << UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT |
        UPDATEMODE_MANUAL << UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT |
        UPDATEMODE_PERIODIC << UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT;
    metadata.telemetryUpdatePeriod    = 0;
    metadata.gcsTelemetryUpdatePeriod = 5000;
    metadata.loggingUpdatePeriod = 0;

    GCSTelemetryStatsSetMetadata(&metadata);
#endif

    return 0;
}


MODULE_INITCALL(osdgenInitialize, osdgenStart);

/**
 * Main osd task. It does not return.
 */
#define BLANK_TIME 1000
#define INTRO_TIME 4000
static void osdgenTask(__attribute__((unused)) void *parameters)
{
    // portTickType lastSysTime;
    // Loop forever
    // lastSysTime = xTaskGetTickCount();
    OsdSettingsData OsdSettings;

    OsdSettingsGet(&OsdSettings);

    switch (PIOS_Board_Revision()) {
    case 1:
        PIOS_Servo_Set(0, OsdSettings.White);
        PIOS_Servo_Set(1, OsdSettings.Black);
        break;
    case 2:
        // DAC buffer enabled:  0.2 V ... 3.1 V		Vout = DAC_value / 4095 * 2.9 + 0.2		DAC_value = (Vout - 0.2) / 2.9 * 4095
        // DAC buffer disabled: 0.0 V ... 3.3 V		Vout = DAC_value / 4095 * 3.3			DAC_value = Vout / 3.3 * 4095
        DAC_SetChannel1Data(DAC_Align_12b_R, OsdSettings.Black);
        DAC_SetChannel2Data(DAC_Align_12b_R, OsdSettings.White);
        break;
    default:
        PIOS_DEBUG_Assert(0);
    }

    Convert[0].m_to_m_feet   = 1.0f;
    Convert[0].ms_to_ms_fts  = 1.0f;
    Convert[0].ms_to_kmh_mph = 3.6f;
    Convert[0].char_m_feet   = 'm';
    Convert[0].char_ms_fts   = 0x88;

    Convert[1].m_to_m_feet   = 3.280840f;
    Convert[1].ms_to_ms_fts  = 3.280840f;
    Convert[1].ms_to_kmh_mph = 2.236936f;
    Convert[1].char_m_feet   = 'f';
    Convert[1].char_ms_fts   = ' ';                  // TODO design a char for font 2 and 3

    // blank
    while (xTaskGetTickCount() <= BLANK_TIME) {
        if (xSemaphoreTake(osdSemaphore, LONG_TIME) == pdTRUE) {
#ifdef PIOS_INCLUDE_WDG
            PIOS_WDG_UpdateFlag(PIOS_WDG_OSDGEN);
#endif
            clearGraphics();
        }
    }

    // intro
    while (xTaskGetTickCount() <= BLANK_TIME + INTRO_TIME) {
        if (xSemaphoreTake(osdSemaphore, LONG_TIME) == pdTRUE) {
#ifdef PIOS_INCLUDE_WDG
            PIOS_WDG_UpdateFlag(PIOS_WDG_OSDGEN);
#endif
            clearGraphics();
            if (PIOS_Video_GetType() == VIDEO_TYPE_NTSC) {
                introGraphics(GRAPHICS_RIGHT / 2, GRAPHICS_BOTTOM / 2 - 20);
                introText(GRAPHICS_RIGHT / 2, GRAPHICS_BOTTOM - 70);
                showVideoType(GRAPHICS_RIGHT / 2, GRAPHICS_BOTTOM - 45);
            } else {
                introGraphics(GRAPHICS_RIGHT / 2, GRAPHICS_BOTTOM / 2 - 30);
                introText(GRAPHICS_RIGHT / 2, GRAPHICS_BOTTOM - 90);
                showVideoType(GRAPHICS_RIGHT / 2, GRAPHICS_BOTTOM - 60);
            }
        }
    }

    while (1) {
        if (xSemaphoreTake(osdSemaphore, LONG_TIME) == pdTRUE) {
#ifdef PIOS_INCLUDE_WDG
            PIOS_WDG_UpdateFlag(PIOS_WDG_OSDGEN);
#endif

#ifdef DEBUG_TIMING
            in_ticks = xTaskGetTickCount();
            out_time = in_ticks - out_ticks;
#endif


#if 1
            OsdSettingsGet(&OsdSettings);
            switch (PIOS_Board_Revision()) {
            case 1:
                PIOS_Servo_Set(0, OsdSettings.White);
                PIOS_Servo_Set(1, OsdSettings.Black);
                break;
            case 2:
                // DAC buffer enabled:  0.2 V ... 3.1 V		Vout = DAC_value / 4095 * 2.9 + 0.2		DAC_value = (Vout - 0.2) / 2.9 * 4095
                // DAC buffer disabled: 0.0 V ... 3.3 V		Vout = DAC_value / 4095 * 3.3			DAC_value = Vout / 3.3 * 4095
                DAC_SetChannel1Data(DAC_Align_12b_R, OsdSettings.Black);
                DAC_SetChannel2Data(DAC_Align_12b_R, OsdSettings.White);
                break;
            default:
                PIOS_DEBUG_Assert(0);
            }
#endif


            clearGraphics();
            updateGraphics();
#ifdef DEBUG_TIMING
            out_ticks = xTaskGetTickCount();
            in_time   = out_ticks - in_ticks;
#endif
        }
        // xSemaphoreTake(osdSemaphore, portMAX_DELAY);
        // vTaskDelayUntil(&lastSysTime, 10 / portTICK_RATE_MS);
    }
}

// ****************

/**
 * @}
 * @}
 */
