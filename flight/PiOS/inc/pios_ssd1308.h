/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SSD1308 SSD1308 Functions
 * @brief Deals with the hardware interface to the magnetometers
 * @{
 *
 * @file       pios_ssd1308.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      SSD1308 functions header.
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

#ifndef PIOS_SSD1308_H
#define PIOS_SSD1308_H

#include <pios.h>

/* SSD1308 Addresses */
#define PIOS_SSD1308_Max_X 		127	//128 Pixels
#define PIOS_SSD1308_Max_Y 		63	//64  Pixels

#define PIOS_SSD1308_PAGE_MODE			01
#define PIOS_SSD1308_HORIZONTAL_MODE			02


#define PIOS_SSD1308_I2C_ADDR		0x3c
#define PIOS_SSD1308_Command_Mode		0x80
#define PIOS_SSD1308_Data_Mode		0x40
#define PIOS_SSD1308_Display_Off_Cmd	0xAE
#define PIOS_SSD1308_Display_On_Cmd	0xAF
#define PIOS_SSD1308_Normal_Display_Cmd	0xA6
#define PIOS_SSD1308_Inverse_Display_Cmd	0xA7
#define PIOS_SSD1308_Activate_Scroll_Cmd	0x2F
#define PIOS_SSD1308_Dectivate_Scroll_Cmd	0x2E
#define PIOS_SSD1308_Set_Brightness_Cmd	0x81
#define PIOS_SSD1308_Set_Paging_Cmd	0x20
#define PIOS_SSD1308_SET_COLUMN_ADDRESS 0x21
#define PIOS_SSD1308_SET_PAGE_ADDRESS 0x22

#define PIOS_SSD1308_Paging_Horizontal	0x00
#define PIOS_SSD1308_Paging_Vertical	0x02

#define PIOS_SSD1308_Scroll_Left			0x00
#define PIOS_SSD1308_Scroll_Right			0x01

#define PIOS_SSD1308_Scroll_2Frames			0x7
#define PIOS_SSD1308_Scroll_3Frames			0x4
#define PIOS_SSD1308_Scroll_4Frames			0x5
#define PIOS_SSD1308_Scroll_5Frames			0x0
#define PIOS_SSD1308_Scroll_25Frames			0x6
#define PIOS_SSD1308_Scroll_64Frames			0x1
#define PIOS_SSD1308_Scroll_128Frames		0x2
#define PIOS_SSD1308_Scroll_256Frames		0x3


/* Public Functions */
extern int8_t PIOS_SSD1308_Init(void);
extern void PIOS_SSD1308_setBrightness(uint8_t Brightness);
extern void PIOS_SSD1308_setHorizontalMode();
extern void PIOS_SSD1308_setPageMode();
extern void PIOS_SSD1308_setTextXY(uint8_t Row, uint8_t Column);
extern void PIOS_SSD1308_clearDisplay();
extern void PIOS_SSD1308_putChar(uint8_t C);
extern void PIOS_SSD1308_putString(const uint8_t *String);

extern void PIOS_SSD1308_ClearFrameBuffer();
extern void PIOS_SSD1308_ShowFrameBuffer();
extern void PIOS_SSD1308_drawPixel(uint8_t on, uint8_t x, uint8_t y);
extern uint8_t PIOS_SSD1308_getPixel(uint8_t x, uint8_t y);
extern void PIOS_SSD1308_drawLine(uint8_t on, int x0, int y0, int x1, int y1);
extern void PIOS_SSD1308_drawRectangle(uint8_t on, int x0, int y0, int x1, int y1);
extern void PIOS_SSD1308_drawFullRectangle(uint8_t on, int x0, int y0, int x1, int y1);
extern void PIOS_SSD1308_drawCircle(uint8_t on, int x0, int y0, int radius);
extern void PIOS_SSD1308_drawFullCircle(uint8_t on, int x0, int y0, int radius);
extern void PIOS_SSD1308_drawEllipse(uint8_t on, int x0, int y0, int a, int b);
extern void PIOS_SSD1308_drawFullEllipse(uint8_t on, int x0, int y0, int a, int b);
extern void PIOS_SSD1308_drawText(uint8_t on, uint8_t x0, uint8_t y0, uint8_t * text, uint8_t size, uint8_t clear);
#endif /* PIOS_SSD1308_H */

/** 
  * @}
  * @}
  */
