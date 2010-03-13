/**
 ******************************************************************************
 *
 * @file       bootloader.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Bootloader functions header
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

#ifndef BOOTLOADER_H
#define BOOTLOADER_H

/* Global Types */
typedef  void (*pFunction)(void);

/* Global Defines */
#define CMD_STRING_SIZE			128
#define ApplicationAddress		(0x8006000)
#define PAGE_SIZE			(0x800)
#define FLASH_SIZE			(0x80000)	/* 512K */

/* Global macros */
#define SerialPutString(x)		Serial_PutString((uint8_t*)(x))

/* Public Functions */
extern void StartBootloader(void);
extern uint32_t FLASH_PagesMask(volatile uint32_t Size);

#endif  /* BOOTLOADER_H */
