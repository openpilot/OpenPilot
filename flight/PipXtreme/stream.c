/**
 ******************************************************************************
 *
 * @file       stream.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Sends or Receives a continuous packet stream to/from the remote unit
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
#include "fifo_buffer.h"
#include "aes.h"
#include "crc.h"
#include "saved_settings.h"
#include "stream.h"

#if defined(PIOS_COM_DEBUG)
	#define STREAM_DEBUG
#endif

// *************************************************************
// can be called from an interrupt if you wish
// call this once every ms

void stream_1ms_tick(void)
{
	if (saved_settings.mode == MODE_STREAM_TX)
	{
	}
	else
	if (saved_settings.mode == MODE_STREAM_RX)
	{
	}
}

// *************************************************************
// call this from the main loop (not interrupt) as often as possible

void stream_process(void)
{
	if (saved_settings.mode == MODE_STREAM_TX)
	{
	}
	else
	if (saved_settings.mode == MODE_STREAM_RX)
	{
	}
}

// *************************************************************

void stream_init(void)
{
	#if defined(STREAM_DEBUG)
		DEBUG_PRINTF("\r\nSTREAM init\r\n");
	#endif

}

// *************************************************************
