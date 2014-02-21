/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 * @file       task_monitor.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Task monitoring functions
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
#ifndef PIOS_TASK_MONITOR_H
#define PIOS_TASK_MONITOR_H

#include <stdbool.h>

/**
 * Initializes the Task Monitor.
 *
 * For correct results, this must be called prior to any calls to other
 * task monitoring functions. The task_ids of the monitored tasks must
 * be in the range [0, max_tasks-1].
 * Initially, all tasks are marked as not running.
 *
 * @param max_tasks The maximum number of tasks that will be monitored.
 * @return 0 on success, -1 on failure.
 */
extern int32_t PIOS_TASK_MONITOR_Initialize(uint16_t max_tasks);

/**
 * Registers a task with the task monitor.
 * A task that has been registered will be marked as running.
 *
 * @param task_id The id of the task to register. Must be in the range [0, max_tasks-1].
 * @param handle  The FreeRTOS xTaskHandle of the running task.
 * @return 0 on success, -1 on failure.
 */
extern int32_t PIOS_TASK_MONITOR_RegisterTask(uint16_t task_id, xTaskHandle handle);

/**
 * Unregisters a task with the task monitor.
 * A task that has been unregistered will be marked as not running.
 *
 * @param task_id The id of the task to unregister. Must be in the range [0, max_tasks-1].
 * @return 0 on success, -1 on failure.
 */
extern int32_t PIOS_TASK_MONITOR_UnregisterTask(uint16_t task_id);

/**
 * Checks if a given task is running.
 * A task that has not been registered will be marked as not running.
 *
 * @param task_id The id of the task to check. Must be in the range [0, max_tasks-1].
 * @return true if the task is running.
 */
extern bool PIOS_TASK_MONITOR_IsRunning(uint16_t task_id);

/**
 * Information about a running task that has been registered
 * via a call to PIOS_TASK_MONITOR_Add().
 */
struct pios_task_info {
    /** Remaining task stack in bytes. */
    uint32_t stack_remaining;
    /** Flag indicating whether or not the task is running. */
    bool     is_running;
    /** Percentage of cpu time used by the task since the last call
     *  to PIOS_TASK_MONITOR_ForEachTask(). Low-load tasks may
     *  report 0% load even though they have run during the interval. */
    uint8_t running_time_percentage;
};

/**
 * Iterator callback, called for each monitored task by PIOS_TASK_MONITOR_TasksIterate().
 *
 * @param task_id   The id of the task the task_info refers to.
 * @param task_info Information about the task identified by task_id.
 * @param context   Context information optionally provided by the caller to PIOS_TASK_MONITOR_TasksIterate()
 */
typedef void (*TaskMonitorTaskInfoCallback)(uint16_t task_id, const struct pios_task_info *task_info, void *context);

/**
 * Iterator callback, called for each monitored task by PIOS_TASK_MONITOR_TasksIterate().
 *
 * @param task_id   The id of the task the task_info refers to.
 * @param task_info Information about the task identified by task_id.
 * @param context   Context information optionally provided by the caller to PIOS_TASK_MONITOR_TasksIterate()
 */
extern void PIOS_TASK_MONITOR_ForEachTask(TaskMonitorTaskInfoCallback callback, void *context);

/**
 * Return the idle task running time percentage.
 */

extern uint8_t PIOS_TASK_MONITOR_GetIdlePercentage();

#endif // PIOS_TASK_MONITOR_H
