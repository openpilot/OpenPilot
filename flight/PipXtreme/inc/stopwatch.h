/**
 ******************************************************************************
 *
 * @file       stopwatch.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Stop watch function
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

#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__

#include "stm32f10x.h"

// *****************************************************************************

void STOPWATCH_init(uint32_t resolution);
void STOPWATCH_reset(void);
uint32_t STOPWATCH_get_count(void);
uint32_t STOPWATCH_get_us(void);

// *****************************************************************************

#endif
