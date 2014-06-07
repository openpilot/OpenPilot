/**
 ******************************************************************************
 *
 * @file       instrumentation.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Instrumentation infrastructure
 *             UAVObject wrapper layer for PiOS instrumentation
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

#include <openpilot.h>
#include <instrumentation.h>
#include <pios_instrumentation.h>

static uint8_t publishedCountersInstances = 0;
static void counterCallback(const pios_perf_counter_t *counter, const int8_t index, void *context);
static xSemaphoreHandle sem;
void InstrumentationInit()
{
    PerfCounterInitialize();
    publishedCountersInstances = 1;
    vSemaphoreCreateBinary(sem);
}

void InstrumentationPublishAllCounters()
{
    if (xSemaphoreTake(sem, 0) != pdTRUE) {
        return;
    }
    PIOS_Instrumentation_ForEachCounter(&counterCallback, NULL);
    xSemaphoreGive(sem);
}

void counterCallback(const pios_perf_counter_t *counter, const int8_t index, __attribute__((unused)) void *context)
{
    if (publishedCountersInstances < index) {
        PerfCounterCreateInstance();
        publishedCountersInstances++;
    }
    PerfCounterData data;
    data.Id = counter->id;
    data.Counter.Max   = counter->max;
    data.Counter.Min   = counter->min;
    data.Counter.Value = counter->value;
    PerfCounterInstSet(index, &data);
}
