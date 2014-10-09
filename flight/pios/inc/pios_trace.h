/**
 ******************************************************************************
 *
 * @file       pios_trace.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup PiOS
 * @{
 * @addtogroup PiOS
 * @{
 * @brief PiOS debug trace printing interface
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
#ifndef PIOS_TRACE_H
#define PIOS_TRACE_H

extern unsigned int pios_trace_mask;

/*
 * Tracing flags.
 * The flags masked in PIOS_TRACE_ALWAYS are always traced.
 */

#define PIOS_TRACE_OS			0x00000002
#define PIOS_TRACE_ALLOCATE		0x00000004
#define PIOS_TRACE_TEST			0x00000008
#define PIOS_TRACE_BAD_BLOCKS	0x00000010
#define PIOS_TRACE_ERASE		0x00000020
#define PIOS_TRACE_GC			0x00000040
#define PIOS_TRACE_WRITE		0x00000080
#define PIOS_TRACE_TRACING		0x00000100
#define PIOS_TRACE_DELETION		0x00000200
#define PIOS_TRACE_BUFFERS		0x00000400
#define PIOS_TRACE_NANDACCESS	0x00000800
#define PIOS_TRACE_GC_DETAIL	0x00001000
#define PIOS_TRACE_SCAN_DEBUG	0x00002000
#define PIOS_TRACE_MTD			0x00004000
#define PIOS_TRACE_CHECKPOINT	0x00008000

#define PIOS_TRACE_VERIFY		0x00010000
#define PIOS_TRACE_VERIFY_NAND	0x00020000
#define PIOS_TRACE_VERIFY_FULL	0x00040000
#define PIOS_TRACE_VERIFY_ALL	0x000f0000

#define PIOS_TRACE_SYNC			0x00100000
#define PIOS_TRACE_BACKGROUND	0x00200000
#define PIOS_TRACE_LOCK			0x00400000
#define PIOS_TRACE_MOUNT		0x00800000

#define PIOS_TRACE_ERROR		0x40000000
#define PIOS_TRACE_BUG			0x80000000
#define PIOS_TRACE_ALWAYS		0xf0000000



#ifdef PIOS_TRACE
#define pios_trace(msk, fmt, ...) do { \
	if (pios_trace_mask & (msk)) \
		printf("pios_trace: " fmt "\n", ##__VA_ARGS__); \
} while (0)
#else
#define pios_trace(msk, fmt, ...) 
#endif

/* Trace control functions */
unsigned  pios_set_trace(unsigned tm);
unsigned  pios_get_trace(void);


#endif
