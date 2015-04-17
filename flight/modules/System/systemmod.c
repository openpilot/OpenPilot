/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @brief The OpenPilot Modules do the majority of the control in OpenPilot.  The
 * @ref SystemModule "System Module" starts all the other modules that then take care
 * of all the telemetry and control algorithms and such.  This is done through the @ref PIOS
 * "PIOS Hardware abstraction layer" which then contains hardware specific implementations
 * (currently only STM32 supported)
 *
 * @{
 * @addtogroup SystemModule System Module
 * @brief Initializes PIOS and other modules runs monitoring
 * After initializing all the modules (currently selected by Makefile but in
 * future controlled by configuration on SD card) runs basic monitoring and
 * alarms.
 * @{
 *
 * @file       systemmod.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      System module
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

#include <openpilot.h>
// private includes
#include "inc/systemmod.h"

#include <notification.h>
#ifdef PIOS_INCLUDE_WS2811
#include <lednotification.h>
#endif

// UAVOs
#include <objectpersistence.h>
#include <flightstatus.h>
#include <systemstats.h>
#include <systemsettings.h>
#include <i2cstats.h>
#include <taskinfo.h>
#include <watchdogstatus.h>
#include <callbackinfo.h>
#include <hwsettings.h>
#include <pios_flashfs.h>
#include <pios_notify.h>

#ifdef PIOS_INCLUDE_INSTRUMENTATION
#include <instrumentation.h>
#include <pios_instrumentation.h>
#endif

#if defined(PIOS_INCLUDE_RFM22B)
#include <oplinkstatus.h>
#endif

// Flight Libraries
#include <sanitycheck.h>


// #define DEBUG_THIS_FILE

#if defined(PIOS_INCLUDE_DEBUG_CONSOLE) && defined(DEBUG_THIS_FILE)
#define DEBUG_MSG(format, ...) PIOS_COM_SendFormattedString(PIOS_COM_DEBUG, format,##__VA_ARGS__)
#else
#define DEBUG_MSG(format, ...)
#endif

// Private constants
#define SYSTEM_UPDATE_PERIOD_MS 250

#if defined(PIOS_SYSTEM_STACK_SIZE)
#define STACK_SIZE_BYTES        PIOS_SYSTEM_STACK_SIZE
#else
#define STACK_SIZE_BYTES        1024
#endif

#define TASK_PRIORITY           (tskIDLE_PRIORITY + 1)

// Private types

// Private variables
static xTaskHandle systemTaskHandle;
static xQueueHandle objectPersistenceQueue;
static enum { STACKOVERFLOW_NONE = 0, STACKOVERFLOW_WARNING = 1, STACKOVERFLOW_CRITICAL = 3 } stackOverflow;
static bool mallocFailed;
static HwSettingsData bootHwSettings;
static FrameType_t bootFrameType;
static struct PIOS_FLASHFS_Stats fsStats;

// Private functions
static void objectUpdatedCb(UAVObjEvent *ev);
static void checkSettingsUpdatedCb(UAVObjEvent *ev);
#ifdef DIAG_TASKS
static void taskMonitorForEachCallback(uint16_t task_id, const struct pios_task_info *task_info, void *context);
static void callbackSchedulerForEachCallback(int16_t callback_id, const struct pios_callback_info *callback_info, void *context);
#endif
static void updateStats();
static void updateSystemAlarms();
static void systemTask(void *parameters);
#ifdef DIAG_I2C_WDG_STATS
static void updateI2Cstats();
static void updateWDGstats();
#endif

extern uintptr_t pios_uavo_settings_fs_id;
extern uintptr_t pios_user_fs_id;

/**
 * Create the module task.
 * \returns 0 on success or -1 if initialization failed
 */
int32_t SystemModStart(void)
{
    // Initialize vars
    stackOverflow = STACKOVERFLOW_NONE;
    mallocFailed  = false;
    // Create system task
    xTaskCreate(systemTask, "System", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &systemTaskHandle);
    // Register task
    PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_SYSTEM, systemTaskHandle);

    return 0;
}

/**
 * Initialize the module, called on startup.
 * \returns 0 on success or -1 if initialization failed
 */
int32_t SystemModInitialize(void)
{
    // Must registers objects here for system thread because ObjectManager started in OpenPilotInit
    SystemSettingsInitialize();
    SystemStatsInitialize();
    FlightStatusInitialize();
    ObjectPersistenceInitialize();
#ifdef DIAG_TASKS
    TaskInfoInitialize();
    CallbackInfoInitialize();
#endif
#ifdef DIAG_I2C_WDG_STATS
    I2CStatsInitialize();
    WatchdogStatusInitialize();
#endif

#ifdef PIOS_INCLUDE_INSTRUMENTATION
    InstrumentationInit();
#endif

    objectPersistenceQueue = xQueueCreate(1, sizeof(UAVObjEvent));
    if (objectPersistenceQueue == NULL) {
        return -1;
    }

    SystemModStart();

    return 0;
}

MODULE_INITCALL(SystemModInitialize, 0);
/**
 * System task, periodically executes every SYSTEM_UPDATE_PERIOD_MS
 */
static void systemTask(__attribute__((unused)) void *parameters)
{
    /* create all modules thread */
    MODULE_TASKCREATE_ALL;

    /* start the delayed callback scheduler */
    PIOS_CALLBACKSCHEDULER_Start();

    if (mallocFailed) {
        /* We failed to malloc during task creation,
         * system behaviour is undefined.  Reset and let
         * the BootFault code recover for us.
         */
        PIOS_SYS_Reset();
    }
#if defined(PIOS_INCLUDE_IAP)
    /* Record a successful boot */
    PIOS_IAP_WriteBootCount(0);
#endif
    // Listen for SettingPersistance object updates, connect a callback function
    ObjectPersistenceConnectQueue(objectPersistenceQueue);

    // Load a copy of HwSetting active at boot time
    HwSettingsGet(&bootHwSettings);
    bootFrameType = GetCurrentFrameType();
    // Whenever the configuration changes, make sure it is safe to fly
    HwSettingsConnectCallback(checkSettingsUpdatedCb);
    SystemSettingsConnectCallback(checkSettingsUpdatedCb);

#ifdef DIAG_TASKS
    TaskInfoData taskInfoData;
    CallbackInfoData callbackInfoData;
#endif
    // Main system loop
    while (1) {
        NotificationUpdateStatus();
        // Update the system statistics
        updateStats();
        // Update the system alarms
        updateSystemAlarms();
#ifdef DIAG_I2C_WDG_STATS
        updateI2Cstats();
        updateWDGstats();
#endif

#ifdef PIOS_INCLUDE_INSTRUMENTATION
        InstrumentationPublishAllCounters();
#endif

#ifdef DIAG_TASKS
        // Update the task status object
        PIOS_TASK_MONITOR_ForEachTask(taskMonitorForEachCallback, &taskInfoData);
        TaskInfoSet(&taskInfoData);
        // Update the callback status object
// if(FALSE){
        PIOS_CALLBACKSCHEDULER_ForEachCallback(callbackSchedulerForEachCallback, &callbackInfoData);
        CallbackInfoSet(&callbackInfoData);
// }
#endif
// }


        UAVObjEvent ev;
        int delayTime = SYSTEM_UPDATE_PERIOD_MS;

#if defined(PIOS_INCLUDE_RFM22B)

        // Update the OPLinkStatus UAVO
        OPLinkStatusData oplinkStatus;
        OPLinkStatusGet(&oplinkStatus);

        if (pios_rfm22b_id) {
            // Get the other device stats.
            PIOS_RFM2B_GetPairStats(pios_rfm22b_id, oplinkStatus.PairIDs, oplinkStatus.PairSignalStrengths, OPLINKSTATUS_PAIRIDS_NUMELEM);

            // Get the stats from the radio device
            struct rfm22b_stats radio_stats;
            PIOS_RFM22B_GetStats(pios_rfm22b_id, &radio_stats);

            // Update the OPLInk status
            static bool first_time = true;
            static uint16_t prev_tx_count = 0;
            static uint16_t prev_rx_count = 0;
            oplinkStatus.HeapRemaining = xPortGetFreeHeapSize();
            oplinkStatus.DeviceID = PIOS_RFM22B_DeviceID(pios_rfm22b_id);
            oplinkStatus.RxGood = radio_stats.rx_good;
            oplinkStatus.RxCorrected   = radio_stats.rx_corrected;
            oplinkStatus.RxErrors = radio_stats.rx_error;
            oplinkStatus.RxMissed = radio_stats.rx_missed;
            oplinkStatus.RxFailure     = radio_stats.rx_failure;
            oplinkStatus.TxDropped     = radio_stats.tx_dropped;
            oplinkStatus.TxFailure     = radio_stats.tx_failure;
            oplinkStatus.Resets      = radio_stats.resets;
            oplinkStatus.Timeouts    = radio_stats.timeouts;
            oplinkStatus.RSSI        = radio_stats.rssi;
            oplinkStatus.LinkQuality = radio_stats.link_quality;
            if (first_time) {
                first_time = false;
            } else {
                uint16_t tx_count = radio_stats.tx_byte_count;
                uint16_t rx_count = radio_stats.rx_byte_count;
                uint16_t tx_bytes = (tx_count < prev_tx_count) ? (0xffff - prev_tx_count + tx_count) : (tx_count - prev_tx_count);
                uint16_t rx_bytes = (rx_count < prev_rx_count) ? (0xffff - prev_rx_count + rx_count) : (rx_count - prev_rx_count);
                oplinkStatus.TXRate = (uint16_t)((float)(tx_bytes * 1000) / SYSTEM_UPDATE_PERIOD_MS);
                oplinkStatus.RXRate = (uint16_t)((float)(rx_bytes * 1000) / SYSTEM_UPDATE_PERIOD_MS);
                prev_tx_count = tx_count;
                prev_rx_count = rx_count;
            }
            oplinkStatus.TXSeq     = radio_stats.tx_seq;
            oplinkStatus.RXSeq     = radio_stats.rx_seq;

            oplinkStatus.LinkState = radio_stats.link_state;
        } else {
            oplinkStatus.LinkState = OPLINKSTATUS_LINKSTATE_DISABLED;
        }
        OPLinkStatusSet(&oplinkStatus);

#endif /* if defined(PIOS_INCLUDE_RFM22B) */

        if (xQueueReceive(objectPersistenceQueue, &ev, delayTime) == pdTRUE) {
            // If object persistence is updated call the callback
            objectUpdatedCb(&ev);
        }
    }
}

/**
 * Function called in response to object updates
 */
static void objectUpdatedCb(UAVObjEvent *ev)
{
    ObjectPersistenceData objper;
    UAVObjHandle obj;

    // If the object updated was the ObjectPersistence execute requested action
    if (ev->obj == ObjectPersistenceHandle()) {
        // Get object data
        ObjectPersistenceGet(&objper);

        int retval = 1;
        FlightStatusData flightStatus;
        FlightStatusGet(&flightStatus);

        // When this is called because of this method don't do anything
        if (objper.Operation == OBJECTPERSISTENCE_OPERATION_ERROR || objper.Operation == OBJECTPERSISTENCE_OPERATION_COMPLETED) {
            return;
        }

        // Execute action if disarmed
        if (flightStatus.Armed != FLIGHTSTATUS_ARMED_DISARMED) {
            retval = -1;
        } else if (objper.Operation == OBJECTPERSISTENCE_OPERATION_LOAD) {
            if (objper.Selection == OBJECTPERSISTENCE_SELECTION_SINGLEOBJECT) {
                // Get selected object
                obj = UAVObjGetByID(objper.ObjectID);
                if (obj == 0) {
                    return;
                }
                // Load selected instance
                retval = UAVObjLoad(obj, objper.InstanceID);
            } else if (objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLSETTINGS || objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLOBJECTS) {
                retval = UAVObjLoadSettings();
            } else if (objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLMETAOBJECTS || objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLOBJECTS) {
                retval = UAVObjLoadMetaobjects();
            }
        } else if (objper.Operation == OBJECTPERSISTENCE_OPERATION_SAVE) {
            if (objper.Selection == OBJECTPERSISTENCE_SELECTION_SINGLEOBJECT) {
                // Get selected object
                obj = UAVObjGetByID(objper.ObjectID);
                if (obj == 0) {
                    return;
                }
                // Save selected instance
                retval = UAVObjSave(obj, objper.InstanceID);

                // Not sure why this is needed
                vTaskDelay(10);

                // Verify saving worked
                if (retval == 0) {
                    retval = UAVObjLoad(obj, objper.InstanceID);
                }
            } else if (objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLSETTINGS || objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLOBJECTS) {
                retval = UAVObjSaveSettings();
            } else if (objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLMETAOBJECTS || objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLOBJECTS) {
                retval = UAVObjSaveMetaobjects();
            }
        } else if (objper.Operation == OBJECTPERSISTENCE_OPERATION_DELETE) {
            if (objper.Selection == OBJECTPERSISTENCE_SELECTION_SINGLEOBJECT) {
                // Get selected object
                obj = UAVObjGetByID(objper.ObjectID);
                if (obj == 0) {
                    return;
                }
                // Delete selected instance
                retval = UAVObjDelete(obj, objper.InstanceID);
            } else if (objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLSETTINGS || objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLOBJECTS) {
                retval = UAVObjDeleteSettings();
            } else if (objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLMETAOBJECTS || objper.Selection == OBJECTPERSISTENCE_SELECTION_ALLOBJECTS) {
                retval = UAVObjDeleteMetaobjects();
            }
        } else if (objper.Operation == OBJECTPERSISTENCE_OPERATION_FULLERASE) {
#if defined(PIOS_INCLUDE_FLASH_LOGFS_SETTINGS)
            retval = PIOS_FLASHFS_Format(0);
#else
            retval = -1;
#endif
        }
        switch (retval) {
        case 0:
            objper.Operation = OBJECTPERSISTENCE_OPERATION_COMPLETED;
            ObjectPersistenceSet(&objper);
            break;
        case -1:
            objper.Operation = OBJECTPERSISTENCE_OPERATION_ERROR;
            ObjectPersistenceSet(&objper);
            break;
        default:
            break;
        }
    }
}

/**
 * Called whenever hardware settings changed
 */
static void checkSettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
    HwSettingsData currentHwSettings;

    HwSettingsGet(&currentHwSettings);
    FrameType_t currentFrameType = GetCurrentFrameType();
    // check whether the Hw Configuration has changed from the one used at boot time
    if ((memcmp(&bootHwSettings, &currentHwSettings, sizeof(HwSettingsData)) != 0) ||
        (currentFrameType != bootFrameType)) {
        ExtendedAlarmsSet(SYSTEMALARMS_ALARM_BOOTFAULT, SYSTEMALARMS_ALARM_CRITICAL, SYSTEMALARMS_EXTENDEDALARMSTATUS_REBOOTREQUIRED, 0);
    }
}

#ifdef DIAG_TASKS
static void taskMonitorForEachCallback(uint16_t task_id, const struct pios_task_info *task_info, void *context)
{
    TaskInfoData *taskData = (TaskInfoData *)context;

    // By convention, there is a direct mapping between task monitor task_id's and members
    // of the TaskInfoXXXXElem enums
    PIOS_DEBUG_Assert(task_id < TASKINFO_RUNNING_NUMELEM);
    TaskInfoRunningToArray(taskData->Running)[task_id] = task_info->is_running ? TASKINFO_RUNNING_TRUE : TASKINFO_RUNNING_FALSE;
    ((uint16_t *)&taskData->StackRemaining)[task_id]   = task_info->stack_remaining;
    ((uint8_t *)&taskData->RunningTime)[task_id] = task_info->running_time_percentage;
}

static void callbackSchedulerForEachCallback(int16_t callback_id, const struct pios_callback_info *callback_info, void *context)
{
    CallbackInfoData *callbackData = (CallbackInfoData *)context;

    if (callback_id < 0) {
        return;
    }
    // delayed callback scheduler reports callback stack overflows as remaininng: -1
    if (callback_info->stack_remaining < 0 && stackOverflow == STACKOVERFLOW_NONE) {
        stackOverflow = STACKOVERFLOW_WARNING;
    }
    // By convention, there is a direct mapping between (not negative) callback scheduler callback_id's and members
    // of the CallbackInfoXXXXElem enums
    PIOS_DEBUG_Assert(callback_id < CALLBACKINFO_RUNNING_NUMELEM);
    ((uint8_t *)&callbackData->Running)[callback_id] = callback_info->is_running;
    ((uint32_t *)&callbackData->RunningTime)[callback_id]   = callback_info->running_time_count;
    ((int16_t *)&callbackData->StackRemaining)[callback_id] = callback_info->stack_remaining;
}
#endif /* ifdef DIAG_TASKS */

/**
 * Called periodically to update the I2C statistics
 */
#ifdef DIAG_I2C_WDG_STATS
static void updateI2Cstats()
{
#if defined(PIOS_INCLUDE_I2C)
    I2CStatsData i2cStats;
    I2CStatsGet(&i2cStats);

    struct pios_i2c_fault_history history;
    PIOS_I2C_GetDiagnostics(&history, &i2cStats.event_errors);

    for (uint8_t i = 0; (i < I2C_LOG_DEPTH) && (i < I2CSTATS_EVENT_LOG_NUMELEM); i++) {
        i2cStats.evirq_log[i] = history.evirq[i];
        i2cStats.erirq_log[i] = history.erirq[i];
        i2cStats.event_log[i] = history.event[i];
        i2cStats.state_log[i] = history.state[i];
    }
    i2cStats.last_error_type = history.type;
    I2CStatsSet(&i2cStats);
#endif
}

static void updateWDGstats()
{
    WatchdogStatusData watchdogStatus;

    watchdogStatus.BootupFlags = PIOS_WDG_GetBootupFlags();
    watchdogStatus.ActiveFlags = PIOS_WDG_GetActiveFlags();
    WatchdogStatusSet(&watchdogStatus);
}
#endif /* ifdef DIAG_I2C_WDG_STATS */

/**
 * Called periodically to update the system stats
 */
static uint16_t GetFreeIrqStackSize(void)
{
    uint32_t i = 0x200;

#if !defined(ARCH_POSIX) && !defined(ARCH_WIN32) && defined(CHECK_IRQ_STACK)
    extern uint32_t _irq_stack_top;
    extern uint32_t _irq_stack_end;
    uint32_t pattern    = 0x0000A5A5;
    uint32_t *ptr       = &_irq_stack_end;

#if 1 /* the ugly way accurate but takes more time, useful for debugging */
    uint32_t stack_size = (((uint32_t)&_irq_stack_top - (uint32_t)&_irq_stack_end) & ~3) / 4;

    for (i = 0; i < stack_size; i++) {
        if (ptr[i] != pattern) {
            i = i * 4;
            break;
        }
    }
#else /* faster way but not accurate */
    if (*(volatile uint32_t *)((uint32_t)ptr + IRQSTACK_LIMIT_CRITICAL) != pattern) {
        i = IRQSTACK_LIMIT_CRITICAL - 1;
    } else if (*(volatile uint32_t *)((uint32_t)ptr + IRQSTACK_LIMIT_WARNING) != pattern) {
        i = IRQSTACK_LIMIT_WARNING - 1;
    } else {
        i = IRQSTACK_LIMIT_WARNING;
    }
#endif
#endif /* if !defined(ARCH_POSIX) && !defined(ARCH_WIN32) && defined(CHECK_IRQ_STACK) */
    return i;
}

/**
 * Called periodically to update the system stats
 */
static void updateStats()
{
    SystemStatsData stats;

    // Get stats and update
    SystemStatsGet(&stats);
    stats.FlightTime = xTaskGetTickCount() * portTICK_RATE_MS;
#if defined(ARCH_POSIX) || defined(ARCH_WIN32)
    // POSIX port of FreeRTOS doesn't have xPortGetFreeHeapSize()
    stats.SystemModStackRemaining = 128;
    stats.HeapRemaining = 10240;
#else
    stats.HeapRemaining = xPortGetFreeHeapSize();
    stats.SystemModStackRemaining = uxTaskGetStackHighWaterMark(NULL) * 4;
#endif

    // Get Irq stack status
    stats.IRQStackRemaining = GetFreeIrqStackSize();

#if !defined(ARCH_POSIX) && !defined(ARCH_WIN32)
    if (pios_uavo_settings_fs_id) {
        PIOS_FLASHFS_GetStats(pios_uavo_settings_fs_id, &fsStats);
        stats.SysSlotsFree   = fsStats.num_free_slots;
        stats.SysSlotsActive = fsStats.num_active_slots;
    }
    if (pios_user_fs_id) {
        PIOS_FLASHFS_GetStats(pios_user_fs_id, &fsStats);
        stats.UsrSlotsFree   = fsStats.num_free_slots;
        stats.UsrSlotsActive = fsStats.num_active_slots;
    }
#endif
    stats.CPULoad = 100 - PIOS_TASK_MONITOR_GetIdlePercentage();

#if defined(PIOS_INCLUDE_ADC) && defined(PIOS_ADC_USE_TEMP_SENSOR)
    float temp_voltage = PIOS_ADC_PinGetVolt(PIOS_ADC_TEMPERATURE_PIN);
    stats.CPUTemp = PIOS_CONVERT_VOLT_TO_CPU_TEMP(temp_voltage);;
#endif
    SystemStatsSet(&stats);
}

/**
 * Update system alarms
 */
static void updateSystemAlarms()
{
    SystemStatsData stats;
    UAVObjStats objStats;
    EventStats evStats;

    SystemStatsGet(&stats);

    // Check heap, IRQ stack and malloc failures
    if (mallocFailed || (stats.HeapRemaining < HEAP_LIMIT_CRITICAL)
#if !defined(ARCH_POSIX) && !defined(ARCH_WIN32) && defined(CHECK_IRQ_STACK)
        || (stats.IRQStackRemaining < IRQSTACK_LIMIT_CRITICAL)
#endif
        ) {
        AlarmsSet(SYSTEMALARMS_ALARM_OUTOFMEMORY, SYSTEMALARMS_ALARM_CRITICAL);
    } else if ((stats.HeapRemaining < HEAP_LIMIT_WARNING)
#if !defined(ARCH_POSIX) && !defined(ARCH_WIN32) && defined(CHECK_IRQ_STACK)
               || (stats.IRQStackRemaining < IRQSTACK_LIMIT_WARNING)
#endif
               ) {
        AlarmsSet(SYSTEMALARMS_ALARM_OUTOFMEMORY, SYSTEMALARMS_ALARM_WARNING);
    } else {
        AlarmsClear(SYSTEMALARMS_ALARM_OUTOFMEMORY);
    }

    // Check CPU load
    if (stats.CPULoad > CPULOAD_LIMIT_CRITICAL) {
        AlarmsSet(SYSTEMALARMS_ALARM_CPUOVERLOAD, SYSTEMALARMS_ALARM_CRITICAL);
    } else if (stats.CPULoad > CPULOAD_LIMIT_WARNING) {
        AlarmsSet(SYSTEMALARMS_ALARM_CPUOVERLOAD, SYSTEMALARMS_ALARM_WARNING);
    } else {
        AlarmsClear(SYSTEMALARMS_ALARM_CPUOVERLOAD);
    }

    // Check for stack overflow
    switch (stackOverflow) {
    case STACKOVERFLOW_NONE:
        AlarmsClear(SYSTEMALARMS_ALARM_STACKOVERFLOW);
        break;
    case STACKOVERFLOW_WARNING:
        AlarmsSet(SYSTEMALARMS_ALARM_STACKOVERFLOW, SYSTEMALARMS_ALARM_WARNING);
        break;
    default:
        AlarmsSet(SYSTEMALARMS_ALARM_STACKOVERFLOW, SYSTEMALARMS_ALARM_CRITICAL);
    }

    // Check for event errors
    UAVObjGetStats(&objStats);
    EventGetStats(&evStats);
    UAVObjClearStats();
    EventClearStats();
    if (objStats.eventCallbackErrors > 0 || objStats.eventQueueErrors > 0 || evStats.eventErrors > 0) {
        AlarmsSet(SYSTEMALARMS_ALARM_EVENTSYSTEM, SYSTEMALARMS_ALARM_WARNING);
    } else {
        AlarmsClear(SYSTEMALARMS_ALARM_EVENTSYSTEM);
    }

    if (objStats.lastCallbackErrorID || objStats.lastQueueErrorID || evStats.lastErrorID) {
        SystemStatsData sysStats;
        SystemStatsGet(&sysStats);
        sysStats.EventSystemWarningID    = evStats.lastErrorID;
        sysStats.ObjectManagerCallbackID = objStats.lastCallbackErrorID;
        sysStats.ObjectManagerQueueID    = objStats.lastQueueErrorID;
        SystemStatsSet(&sysStats);
    }
}

/**
 * Called by the RTOS when the CPU is idle,
 */
void vApplicationIdleHook(void)
{
    NotificationOnboardLedsRun();
#ifdef PIOS_INCLUDE_WS2811
    LedNotificationExtLedsRun();
#endif
}
/**
 * Called by the RTOS when a stack overflow is detected.
 */
#define DEBUG_STACK_OVERFLOW 0
void vApplicationStackOverflowHook(__attribute__((unused)) xTaskHandle *pxTask,
                                   __attribute__((unused)) signed portCHAR *pcTaskName)
{
    stackOverflow = STACKOVERFLOW_CRITICAL;
#if DEBUG_STACK_OVERFLOW
    static volatile bool wait_here = true;
    while (wait_here) {
        ;
    }
    wait_here = true;
#endif
}

/**
 * Called by the RTOS when a malloc call fails.
 */
#define DEBUG_MALLOC_FAILURES 0
void vApplicationMallocFailedHook(void)
{
    mallocFailed = true;
#if DEBUG_MALLOC_FAILURES
    static volatile bool wait_here = true;
    while (wait_here) {
        ;
    }
    wait_here = true;
#endif
}

/**
 * @}
 * @}
 */
