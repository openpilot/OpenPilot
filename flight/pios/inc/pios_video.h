/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_VIDEO Code for OSD video generator
 * @brief OSD generator, Parts from CL-OSD and SUPEROSD project
 * @{
 *
 * @file       pios_video.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      OSD generator, Parts from CL-OSD and SUPEROSD projects
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
 */
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

#ifndef PIOS_VIDEO_H
#define PIOS_VIDEO_H

#include <pios_stm32.h>
#include <pios_spi_priv.h>

// PAL/NTSC specific boundary values
struct pios_video_type_boundary {
    uint16_t graphics_left;
    uint16_t graphics_top;
    uint16_t graphics_right;
    uint16_t graphics_bottom;
};

// PAL/NTSC specific config values
struct pios_video_type_cfg {
    uint16_t graphics_width_real;
    uint16_t graphics_hight_real;
    uint8_t  graphics_column_start;
    uint8_t  graphics_line_start;
    uint8_t  dma_buffer_length;
    uint8_t  period;
    uint8_t  dc;
};

struct pios_video_cfg {
    DMA_TypeDef *mask_dma;
    const struct pios_spi_cfg mask;
    DMA_TypeDef *level_dma;
    const struct pios_spi_cfg  level;

    const struct pios_exti_cfg *hsync;
    const struct pios_exti_cfg *vsync;

    struct pios_tim_channel    pixel_timer;
    struct pios_tim_channel    hsync_capture;

    TIM_OCInitTypeDef tim_oc_init;
};

extern bool PIOS_Vsync_ISR();
extern bool PIOS_Hsync_ISR();
extern void PIOS_Video_Init(const struct pios_video_cfg *cfg);
extern void PIOS_Pixel_Init(void);
uint16_t PIOS_Video_GetLines(void);
uint16_t PIOS_Video_GetType(void);
void PIOS_Video_BoundaryReset(void);
void PIOS_Video_BoundaryLimit(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom);


// video boundary values
extern const struct pios_video_type_boundary *pios_video_type_boundary_act;
#define GRAPHICS_LEFT        pios_video_type_boundary_act->graphics_left
#define GRAPHICS_TOP         pios_video_type_boundary_act->graphics_top
#define GRAPHICS_RIGHT       pios_video_type_boundary_act->graphics_right
#define GRAPHICS_BOTTOM      pios_video_type_boundary_act->graphics_bottom

#define GRAPHICS_X_MIDDLE	((GRAPHICS_RIGHT + 1) / 2)
#define GRAPHICS_Y_MIDDLE	((GRAPHICS_BOTTOM + 1) / 2)


// video type defs for autodetect
#define VIDEO_TYPE_NTSC      0
#define VIDEO_TYPE_PAL       1
#define VIDEO_TYPE_PAL_ROWS  300


// draw area buffer values, for memory allocation, access and calculations we suppose the larger values for PAL, this also works for NTSC
#define GRAPHICS_WIDTH_REAL  400                            // max columns
#define GRAPHICS_HEIGHT_REAL 288                            // max lines
#define BUFFER_WIDTH         (GRAPHICS_WIDTH_REAL / 8 + 1)  // Bytes plus one byte for SPI
#define BUFFER_HEIGHT        (GRAPHICS_HEIGHT_REAL)


// Macro to swap buffers given a temporary pointer.
#define SWAP_BUFFS(tmp, a, b) { tmp = a; a = b; b = tmp; }

#endif /* PIOS_VIDEO_H */
