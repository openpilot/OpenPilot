/**
 ******************************************************************************
 *
 * @file       pios_instrumentation_helper.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Macros to easily add optional performance monitoring to a module
 *
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

#ifndef PIOS_INSTRUMENTATION_HELPER_H
#define PIOS_INSTRUMENTATION_HELPER_H

#if defined(PIOS_INCLUDE_INSTRUMENTATION) && defined(PIOS_INSTRUMENT_MODULE)

#include <pios_instrumentation.h>
/**
 * include the following macro together with modules variable declaration
 */
#define PERF_DEFINE_COUNTER(x) pios_counter_t x

/**
 * this mast be called at some module init code
 */
#define PERF_INIT_COUNTER(x, id) x = PIOS_Instrumentation_CreateCounter(id)

/**
 * those are the monitoring macros
 */
#define PERF_TIMED_SECTION_START(x) PIOS_Instrumentation_TimeStart(x)
#define PERF_TIMED_SECTION_END(x) PIOS_Instrumentation_TimeEnd(x)
#define PERF_MEASURE_PERIOD(x) PIOS_Instrumentation_TrackPeriod(x)
#define PERF_TRACK_VALUE(x,y) PIOS_Instrumentation_updateCounter(x, y)

#else

#define PERF_DEFINE_COUNTER(x)
#define PERF_INIT_COUNTER(x, id)
#define PERF_TIMED_SECTION_START(x)
#define PERF_TIMED_SECTION_END(x)
#define PERF_MEASURE_PERIOD(x)
#define PERF_TRACK_VALUE(x,y)
#endif /* PIOS_INCLUDE_INSTRUMENTATION */
#endif /* PIOS_INSTRUMENTATION_HELPER_H */
