/**
 ******************************************************************************
 *
 * @file       ppm.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Sends or Receives the ppm values to/from the remote unit
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

//#include <string.h>	// memmove

#include "main.h"
#include "rfm22b.h"
#include "saved_settings.h"
#include "ppm.h"

#if defined(PIOS_COM_DEBUG)
//  #define PPM_DEBUG
#endif

// *************************************************************
// can be called from an interrupt if you wish

void ppm_1ms_tick(void)
{	// call this once every ms
}

// *************************************************************
// call this from the main loop (not interrupt) as often as possible

void ppm_process(void)
{
}

// *************************************************************

void ppm_init(void)
{

}

// *************************************************************
