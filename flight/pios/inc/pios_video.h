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

struct pios_video_cfg {
    DMA_TypeDef *mask_dma;
    const struct pios_spi_cfg  mask;
    DMA_TypeDef *level_dma;
    const struct pios_spi_cfg  level;

    const struct pios_exti_cfg *hsync;
    const struct pios_exti_cfg *vsync;

    struct pios_tim_channel    pixel_timer;
    struct pios_tim_channel    hsync_capture;

    TIM_OCInitTypeDef tim_oc_init;
};

// Time vars
typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
} TTime;

extern TTime timex;

extern bool PIOS_Vsync_ISR();
extern bool PIOS_Hsync_ISR();
extern void PIOS_Video_Init(const struct pios_video_cfg *cfg);
extern void PIOS_Pixel_Init(void);
uint16_t PIOS_Video_GetLines(void);
uint16_t PIOS_Video_GetType(void);


// video type defs for autodetect
#define VIDEO_TYPE_NTSC			0
#define VIDEO_TYPE_PAL			1
#define VIDEO_TYPE_PAL_ROWS		300


#define PAL

// OSD values
#ifdef PAL
#define GRAPHICS_COLUMN			70			// First visible OSD column (after Hsync)
#define GRAPHICS_LINE			17			// First visible OSD line
#define GRAPHICS_WIDTH_REAL		400			// Real visible columns
#define GRAPHICS_HEIGHT_REAL	288			// Real visible lines
#else
#define GRAPHICS_COLUMN			60			// First visible OSD column (after Hsync)
#define GRAPHICS_LINE			13			// First visible OSD line
#define GRAPHICS_WIDTH_REAL		368			// Real visible columns
#define GRAPHICS_HEIGHT_REAL	241			// Real visible lines
#endif

// draw area
#define GRAPHICS_TOP			0
#define GRAPHICS_LEFT			0
#define GRAPHICS_BOTTOM			(GRAPHICS_HEIGHT_REAL - 1)
#define GRAPHICS_RIGHT			(GRAPHICS_WIDTH_REAL - 1)

// draw and DMA/SPI buffer values
#define BUFFER_WIDTH			(GRAPHICS_WIDTH_REAL / 8 + 1)	// Bytes plus one byte for SPI
#define BUFFER_HEIGHT			(GRAPHICS_HEIGHT_REAL)

// Macro to swap buffers given a temporary pointer.
#define SWAP_BUFFS(tmp, a, b) { tmp = a; a = b; b = tmp; }

#endif /* PIOS_VIDEO_H */
