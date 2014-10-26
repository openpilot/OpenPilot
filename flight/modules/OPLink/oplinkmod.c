/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @brief The OpenPilot Modules do the majority of the control in OpenPilot.  The
 * @ref OPLinkModule The OPLink Module is the equivalanet of the System
 * Module for the OPLink modem.  it starts all the other modules.
 #  This is done through the @ref PIOS "PIOS Hardware abstraction layer",
 # which then contains hardware specific implementations
 * (currently only STM32 supported)
 *
 * @{
 * @addtogroup OPLinkModule OPLink Module
 * @brief Initializes PIOS and other modules runs monitoring
 * After initializing all the modules runs basic monitoring and
 * alarms.
 * @{
 *
 * @file       oplinkmod.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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

#include <pios.h>

#include <uavobjectmanager.h>
#include <openpilot.h>

#include <oplinkstatus.h>
#include <taskinfo.h>

#include <pios_rfm22b.h>
#include <pios_board_info.h>

// Private constants
#define SYSTEM_UPDATE_PERIOD_MS 1000

#if defined(PIOS_SYSTEM_STACK_SIZE)
#define STACK_SIZE_BYTES        PIOS_SYSTEM_STACK_SIZE
#else
#define STACK_SIZE_BYTES        924
#endif

#define TASK_PRIORITY           (tskIDLE_PRIORITY + 2)

// Private types

// Private variables
static xTaskHandle systemTaskHandle;
static bool stackOverflow;
static bool mallocFailed;

// Private functions
static void systemTask(void *parameters);

/**
 * Create the module task.
 * \returns 0 on success or -1 if initialization failed
 */
int32_t OPLinkModStart(void)
{
    // Initialize vars
    stackOverflow = false;
    mallocFailed  = false;
    // Create oplink system task
    xTaskCreate(systemTask, "OPLink", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &systemTaskHandle);
    // Register task
    PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_SYSTEM, systemTaskHandle);

    return 0;
}

/**
 * Initialize the module, called on startup.
 * \returns 0 on success or -1 if initialization failed
 */
int32_t OPLinkModInitialize(void)
{
    // Must registers objects here for system thread because ObjectManager started in OpenPilotInit

    // Call the module start function.
    OPLinkModStart();

    return 0;
}

MODULE_INITCALL(OPLinkModInitialize, 0);

/**
 * System task, periodically executes every SYSTEM_UPDATE_PERIOD_MS
 */
static void systemTask(__attribute__((unused)) void *parameters)
{
    portTickType lastSysTime;
    uint16_t prev_tx_count = 0;
    uint16_t prev_rx_count = 0;
    bool first_time = true;

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

    // Initialize vars
    lastSysTime = xTaskGetTickCount();

    // Main system loop
    while (1) {
        // Flash the heartbeat LED
#if defined(PIOS_LED_HEARTBEAT)
        PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
#endif /* PIOS_LED_HEARTBEAT */

        // Update the OPLinkStatus UAVO
        OPLinkStatusData oplinkStatus;
        OPLinkStatusGet(&oplinkStatus);

        // Get the other device stats.
        PIOS_RFM2B_GetPairStats(pios_rfm22b_id, oplinkStatus.PairIDs, oplinkStatus.PairSignalStrengths, OPLINKSTATUS_PAIRIDS_NUMELEM);

        // Get the stats from the radio device
        struct rfm22b_stats radio_stats;
        PIOS_RFM22B_GetStats(pios_rfm22b_id, &radio_stats);

        if (pios_rfm22b_id) {
            // Update the status
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

        if (radio_stats.link_state == OPLINKSTATUS_LINKSTATE_CONNECTED) {
            LINK_LED_ON;
        } else {
            LINK_LED_OFF;
        }

        // Update the object
        OPLinkStatusSet(&oplinkStatus);

        // Wait until next period
        vTaskDelayUntil(&lastSysTime, SYSTEM_UPDATE_PERIOD_MS / portTICK_RATE_MS);
    }
}

/**
 * Called by the RTOS when the CPU is idle, used to measure the CPU idle time.
 */
void vApplicationIdleHook(void)
{}

/**
 * Called by the RTOS when a stack overflow is detected.
 */
#define DEBUG_STACK_OVERFLOW 0
void vApplicationStackOverflowHook(__attribute__((unused)) xTaskHandle *pxTask,
                                   __attribute__((unused)) signed portCHAR *pcTaskName)
{
    stackOverflow = true;
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
