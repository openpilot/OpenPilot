/*
	FreeRTOS.org V4.2.1 (WIN32 port) - Copyright (C) 2007-2008 Dushara Jayasinghe.

	This file is part of the FreeRTOS.org distribution.

	FreeRTOS.org is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	FreeRTOS.org is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FreeRTOS.org; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	A special exception to the GPL can be applied should you wish to distribute
	a combined work that includes FreeRTOS.org, without being obliged to provide
	the source code for any proprietary components.  See the licensing section
	of http://www.FreeRTOS.org for full details of how and when the exception
	can be applied.

	***************************************************************************
	See http://www.FreeRTOS.org for documentation, latest information, license
	and contact details.  Please ensure to read the configuration and relevant
	port sections of the online documentation.

	Also see http://www.SafeRTOS.com for an IEC 61508 compliant version along
	with commercial development and support options.
	***************************************************************************
*/

#ifndef CPUEMU_H
#define CPUEMU_H

/******************************************************************************
	Defines
******************************************************************************/
#define	CPU_INTR_SWI			0	// Software Interrupt (for yield)
#define CPU_INTR_USER1			1
#define CPU_INTR_USER2			2
#define CPU_INTR_USER3			3
#define CPU_INTR_USER4			4
#define CPU_INTR_USER5			5
#define CPU_INTR_USER6			6
#define CPU_INTR_USER7			7
#define CPU_INTR_USER8			8
#define CPU_INTR_USER9			9
#define CPU_INTR_USER10			10
#define CPU_INTR_USER11			11
#define CPU_INTR_USER12			12
#define CPU_INTR_USER13			13
#define CPU_INTR_USER14			14
#define CPU_INTR_USER15			15
#define CPU_INTR_USER16			16
#define CPU_INTR_USER17			17
#define CPU_INTR_USER18			18
#define CPU_INTR_USER19			19
#define CPU_INTR_USER20			20
#define CPU_INTR_USER21			21
#define CPU_INTR_USER22			22
#define CPU_INTR_USER23			23
#define CPU_INTR_USER24			24
#define CPU_INTR_USER25			25
#define CPU_INTR_USER26			26
#define CPU_INTR_USER27			27
#define CPU_INTR_USER28			28
#define CPU_INTR_USER29			29
#define CPU_INTR_USER30			30	
#define CPU_INTR_TICK			31	// system tick (used by FreeRTOS)

#define CPU_INTR_USER_START		CPU_INTR_USER2
#define CPU_INTR_USER_END		CPU_INTR_USER30
#define CPU_INTR_COUNT			32

/******************************************************************************
	Global variables
******************************************************************************/
/******************************************************************************
	Global functions
******************************************************************************/

#endif
