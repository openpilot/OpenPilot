/**
 ******************************************************************************
 *
 * @file       pios_trace.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup PiOS
 * @{
 * @addtogroup PiOS
 * @{
 * @brief PiOS trace debug function
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



#include "pios_trace.h"

unsigned pios_trace_mask =

	PIOS_TRACE_TEST |
	PIOS_TRACE_ALWAYS |
	0;

unsigned pios_set_trace(unsigned tm)
{
	pios_trace_mask = tm;
	return pios_trace_mask;
}

unsigned pios_get_trace(void)
{
	return pios_trace_mask;
}

