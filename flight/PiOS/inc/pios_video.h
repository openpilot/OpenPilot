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
	const struct pios_spi_cfg mask;
	const struct pios_spi_cfg level;

	const struct pios_exti_cfg * hsync;
	const struct pios_exti_cfg * vsync;

	/*struct stm32_exti hsync;
	struct stm32_exti vsync;
	struct stm32_gpio hsync_io;
	struct stm32_gpio vsync_io;
	struct stm32_irq hsync_irq;
	struct stm32_irq vsync_irq;*/
};

// Time vars
typedef struct {
  uint8_t sec;
  uint8_t min;
  uint8_t hour;
} TTime;

extern TTime timex;

extern void PIOS_Video_Init(const struct pios_video_cfg * cfg);
uint16_t PIOS_Video_GetOSDLines(void);
extern bool PIOS_Hsync_ISR();
extern bool PIOS_Vsync_ISR();

// First OSD line
#define GRAPHICS_LINE 32

//top/left deadband
#define GRAPHICS_HDEADBAND 32
#define GRAPHICS_VDEADBAND 0

#define PAL

// Real OSD size
#ifdef PAL
 #define GRAPHICS_WIDTH_REAL (336+GRAPHICS_HDEADBAND)
 #define GRAPHICS_HEIGHT_REAL (270+GRAPHICS_VDEADBAND)
#else
 #define GRAPHICS_WIDTH_REAL (320+GRAPHICS_HDEADBAND)
 #define GRAPHICS_HEIGHT_REAL (225+GRAPHICS_VDEADBAND)
#endif

//draw area
#define GRAPHICS_TOP 0
#define GRAPHICS_LEFT 0
#define GRAPHICS_BOTTOM (GRAPHICS_HEIGHT_REAL-GRAPHICS_VDEADBAND-1)
#define GRAPHICS_RIGHT (GRAPHICS_WIDTH_REAL-GRAPHICS_HDEADBAND-1)


#define GRAPHICS_WIDTH (GRAPHICS_WIDTH_REAL/8)
#define GRAPHICS_HEIGHT GRAPHICS_HEIGHT_REAL

// dma lenght
#define BUFFER_LINE_LENGTH         (GRAPHICS_WIDTH)  //Yes, in bytes.

// line types
#define LINE_TYPE_UNKNOWN 0
#define LINE_TYPE_GRAPHICS 2

// Macro to swap buffers given a temporary pointer.
#define SWAP_BUFFS(tmp, a, b) { tmp = a; a = b; b = tmp; }

#endif /* PIOS_VIDEO_H */
