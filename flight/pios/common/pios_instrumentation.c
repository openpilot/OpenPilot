/**
 ******************************************************************************
 *
 * @file       pios_instrumentation.c
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

#include <pios_instrumentation.h>

pios_perf_counter_t *pios_instrumentation_perf_counters = NULL;
int8_t pios_instrumentation_max_counters = -1;
int8_t pios_instrumentation_last_used_counter = -1;

void PIOS_Instrumentation_Init(int8_t maxCounters)
{
    PIOS_Assert(maxCounters >= 0);
    if (maxCounters > 0) {
        pios_instrumentation_perf_counters = (pios_perf_counter_t *)pvPortMalloc(sizeof(pios_perf_counter_t) * maxCounters);
        PIOS_Assert(pios_instrumentation_perf_counters);
        memset(pios_instrumentation_perf_counters, 0, sizeof(pios_perf_counter_t) * maxCounters);
        pios_instrumentation_max_counters  = maxCounters;
    } else {
        pios_instrumentation_perf_counters = NULL;
        pios_instrumentation_max_counters  = -1;
    }
}

pios_counter_t PIOS_Instrumentation_CreateCounter(uint32_t id)
{
    PIOS_Assert(pios_instrumentation_perf_counters && (pios_instrumentation_max_counters > pios_instrumentation_last_used_counter));

    pios_counter_t counter_handle = PIOS_Instrumentation_SearchCounter(id);
    if (!counter_handle) {
        pios_perf_counter_t *newcounter = &pios_instrumentation_perf_counters[++pios_instrumentation_last_used_counter];
        newcounter->id  = id;
        newcounter->max = INT32_MIN + 1;
        newcounter->min = INT32_MAX - 1;
        counter_handle  = (pios_counter_t)newcounter;
    }
    return counter_handle;
}

pios_counter_t PIOS_Instrumentation_SearchCounter(uint32_t id)
{
    PIOS_Assert(pios_instrumentation_perf_counters);
    uint8_t i = 0;
    while (i < pios_instrumentation_last_used_counter && pios_instrumentation_perf_counters[i].id != id) {
        i++;
    }
    if (pios_instrumentation_perf_counters[i].id != id) {
        return NULL;
    }
    return (pios_counter_t)&pios_instrumentation_perf_counters[i];
}

void PIOS_Instrumentation_ForEachCounter(InstrumentationCounterCallback callback, void *context)
{
    PIOS_Assert(pios_instrumentation_perf_counters);
    for (int8_t index = 0; index < pios_instrumentation_last_used_counter + 1; index++) {
        const pios_perf_counter_t *counter = &pios_instrumentation_perf_counters[index];
        callback(counter, index, context);
    }
}
