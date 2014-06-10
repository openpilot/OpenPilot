/**
 ******************************************************************************
 *
 * @file       pios_instrumentation.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      PiOS instrumentation infrastructure
 *             Allow to collects performance indexes and execution times
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
#ifndef PIOS_INSTRUMENTATION_H
#define PIOS_INSTRUMENTATION_H
#include <pios.h>
#include <pios_debug.h>
#include <pios_delay.h>
#include <FreeRTOS.h>
typedef struct {
    uint32_t id;
    int32_t  max;
    int32_t  min;
    int32_t  value;
    uint32_t lastUpdateTS;
} pios_perf_counter_t;

typedef void *pios_counter_t;

extern pios_perf_counter_t *pios_instrumentation_perf_counters;
extern int8_t pios_instrumentation_last_used_counter;

/**
 * Update a counter with a new value
 * @param counterIdx index of the counter to update @see PIOS_Instrumentation_SearchCounter @see PIOS_Instrumentation_CreateCounter
 * @param newValue the updated value.
 */
inline void PIOS_Instrumentation_updateCounter(pios_counter_t counter_handle, int32_t newValue)
{
    PIOS_Assert(pios_instrumentation_perf_counters && counter_handle);
    vPortEnterCritical();
    pios_perf_counter_t *counter = (pios_perf_counter_t *) counter_handle;
    counter->value = newValue;
    if (counter->value > counter->max) {
        counter->max = counter->value;
    }
    if (counter->value < counter->min) {
        counter->min = counter->value;
    }
    counter->lastUpdateTS = PIOS_DELAY_GetRaw();
    vPortExitCritical();
}

/**
 * Used to determine the time duration of a code block, mark the begin of the block. @see PIOS_Instrumentation_TimeEnd
 * @param counterIdx counterIdx index of the counter @see PIOS_Instrumentation_SearchCounter @see PIOS_Instrumentation_CreateCounter
 */
inline void PIOS_Instrumentation_TimeStart(pios_counter_t counter_handle)
{
    PIOS_Assert(pios_instrumentation_perf_counters && counter_handle);
    vPortEnterCritical();
    pios_perf_counter_t *counter = (pios_perf_counter_t *) counter_handle;

    counter->lastUpdateTS = PIOS_DELAY_GetRaw();
    vPortExitCritical();
}

/**
 * Used to determine the time duration of a code block, mark the end of the block. @see PIOS_Instrumentation_TimeStart
 * @param counterIdx counterIdx index of the counter @see PIOS_Instrumentation_SearchCounter @see PIOS_Instrumentation_CreateCounter
 */
inline void PIOS_Instrumentation_TimeEnd(pios_counter_t counter_handle)
{
    PIOS_Assert(pios_instrumentation_perf_counters && counter_handle);
    vPortEnterCritical();
    pios_perf_counter_t *counter = (pios_perf_counter_t *) counter_handle;

    counter->value = PIOS_DELAY_DiffuS(counter->lastUpdateTS);
    if (counter->value > counter->max) {
        counter->max = counter->value;
    }
    if (counter->value < counter->min) {
        counter->min = counter->value;
    }
    counter->lastUpdateTS = PIOS_DELAY_GetRaw();
    vPortExitCritical();
}

/**
 * Used to determine the mean period between each call to the function
 * @param counterIdx counterIdx index of the counter @see PIOS_Instrumentation_SearchCounter @see PIOS_Instrumentation_CreateCounter
 */
inline void PIOS_Instrumentation_TrackPeriod(pios_counter_t counter_handle)
{
    PIOS_Assert(pios_instrumentation_perf_counters && counter_handle);
    pios_perf_counter_t *counter = (pios_perf_counter_t *) counter_handle;
    if(counter->lastUpdateTS != 0){
        vPortEnterCritical();
        uint32_t period = PIOS_DELAY_DiffuS(counter->lastUpdateTS);
        counter->value = (counter->value * 15 + period) / 16;
        if ((int32_t)period > counter->max) {
            counter->max = period;
        }
        if ((int32_t)period < counter->min) {
            counter->min = period;
        }
        vPortExitCritical();
    }
    counter->lastUpdateTS = PIOS_DELAY_GetRaw();
}

/**
 * Initialize the Instrumentation infrastructure
 * @param maxCounters maximum number of allowed counters
 */
void PIOS_Instrumentation_Init(int8_t maxCounters);

/**
 * Create a new counter.
 * @param id the unique id to assig to the counter
 * @return the counter index to be used to manage its content
 */
pios_counter_t PIOS_Instrumentation_CreateCounter(uint32_t id);

/**
 * search a counter index by its unique Id
 * @param id the unique id to assig to the counter
 * @return the counter index to be used to manage its content
 */
pios_counter_t PIOS_Instrumentation_SearchCounter(uint32_t id);

typedef void (*InstrumentationCounterCallback)(const pios_perf_counter_t *counter, const int8_t index, void *context);
/**
 * Retrieve and execute the passed callback for each counter
 * @param callback to be called for each counter
 * @param context a context variable pointer that can be passed to the callback
 */
void PIOS_Instrumentation_ForEachCounter(InstrumentationCounterCallback callback, void *context);

#endif /* PIOS_INSTRUMENTATION_H */
