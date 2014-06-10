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

/**
 * @addtogroup PIOS PiOS Instrumentation Support
 * @{
 */

/**
* \par
* This is a collections of helper macros that ease adding instrumentation support.
* \par
* Step by step guide:
*
* Define PIOS_INSTRUMENT_MODULE before including this file to enable instrumentation for a module
*
* <pre>#define PIOS_INSTRUMENT_MODULE
* #include <pios_instrumentation_helper.h></pre>
*
* Declare the variables used to hold counter handlers.
* Place the following code along all module variables declaration.
* <pre>PERF_DEFINE_COUNTER(counterUpd);
* PERF_DEFINE_COUNTER(counterAccelSamples);
* PERF_DEFINE_COUNTER(counterPeriod);
* PERF_DEFINE_COUNTER(counterAtt);</pre>
*
* Counters needs to be initialized before they are used.
* The following code needs to be added to a function called at module initialization.
* the second parameter is a unique counter Id.
* A good pracice is to use the upper half word as module id and lower as counter id
* <pre>PERF_INIT_COUNTER(counterUpd, 0xA7710001);
* PERF_INIT_COUNTER(counterAtt, 0xA7710002);
* PERF_INIT_COUNTER(counterPeriod, 0xA7710003);
* PERF_INIT_COUNTER(counterAccelSamples, 0xA7710004);</pre>
*
* At this point you can start using the counters as in the following samples
*
* Track the time spent on a certain function:
* <pre>PERF_TIMED_SECTION_START(counterAtt);
* updateAttitude(&accelState, &gyros);
* PERF_TIMED_SECTION_END(counterAtt);</pre>
* PERF_TIMED_SECTION_[START!STOP] marks the beginning and the end of the code to monitor
*
* Measure the mean of the period a certain point is reached:
* <pre>PERF_MEASURE_PERIOD(counterPeriod);</pre>
* Note that the value stored in the counter is a long running mean while max and min are single point values
*
* Track an user defined int32_t value:
* <pre>PERF_TRACK_VALUE(counterAccelSamples, i);</pre>
* the counter is then updated with the value of i.
*
* \par
*/

#ifndef PIOS_INSTRUMENTATION_HELPER_H
#define PIOS_INSTRUMENTATION_HELPER_H

#if defined(PIOS_INCLUDE_INSTRUMENTATION) && defined(PIOS_INSTRUMENT_MODULE)

#include <pios_instrumentation.h>
/**
 * include the following macro together with modules variable declaration
 */
#define PERF_DEFINE_COUNTER(x)      pios_counter_t x

/**
 * this mast be called at some module init code
 */
#define PERF_INIT_COUNTER(x, id)    x = PIOS_Instrumentation_CreateCounter(id)

/**
 * those are the monitoring macros
 */
#define PERF_TIMED_SECTION_START(x) PIOS_Instrumentation_TimeStart(x)
#define PERF_TIMED_SECTION_END(x)   PIOS_Instrumentation_TimeEnd(x)
#define PERF_MEASURE_PERIOD(x)      PIOS_Instrumentation_TrackPeriod(x)
#define PERF_TRACK_VALUE(x, y)      PIOS_Instrumentation_updateCounter(x, y)

#else

#define PERF_DEFINE_COUNTER(x)
#define PERF_INIT_COUNTER(x, id)
#define PERF_TIMED_SECTION_START(x)
#define PERF_TIMED_SECTION_END(x)
#define PERF_MEASURE_PERIOD(x)
#define PERF_TRACK_VALUE(x, y)
#endif /* PIOS_INCLUDE_INSTRUMENTATION */
#endif /* PIOS_INSTRUMENTATION_HELPER_H */
