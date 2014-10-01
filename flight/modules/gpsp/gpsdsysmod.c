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
 * @addtogroup SystemModule GPSV9 System Module
 * @brief Initializes PIOS and other modules runs monitoring, executes mag and gps handlers
 *
 * @{
 *
 * @file       gpsdsystemmod.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      GPS System module
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


// private includes
#include "inc/gpsdsysmod.h"
#include "inc/gps9maghandler.h"
#include "inc/gps9gpshandler.h"
#include "inc/gps9flashhandler.h"
#include "inc/gps9protocol.h"
#include "pios_board_info.h"

extern uint32_t pios_com_main_id;

// Private constants
#define SYSTEM_UPDATE_PERIOD_MS    1
#define HB_LED_BLINK_ON_PERIOD_MS  100
#define HB_LED_BLINK_OFF_PERIOD_MS 1900
#define STACK_SIZE_BYTES           450
#define STAT_UPDATE_PERIOD_MS      10000
#define TASK_PRIORITY              (tskIDLE_PRIORITY + 2)

// Private types

// Private variables
static xTaskHandle systemTaskHandle;
static enum { STACKOVERFLOW_NONE = 0, STACKOVERFLOW_WARNING = 1, STACKOVERFLOW_CRITICAL = 3 } stackOverflow;


static bool mallocFailed;
static SysUbxPkt sysPkt;

// Private functions
static void updateStats();
static void gpspSystemTask(void *parameters);
static void readFirmwareInfo();
/**
 * Create the module task.
 * \returns 0 on success or -1 if initialization failed
 */
int32_t GPSPSystemModStart(void)
{
    // Initialize vars
    stackOverflow = STACKOVERFLOW_NONE;
    mallocFailed  = false;
    // Create system task
    xTaskCreate(gpspSystemTask, (const char *)"G-Sys", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &systemTaskHandle);
#ifdef PIOS_INCLUDE_WDG
    PIOS_WDG_RegisterFlag(PIOS_WDG_SYSTEM);
#endif
    return 0;
}

/**
 * Initialize the module, called on startup.
 * \returns 0 on success or -1 if initialization failed
 */
int32_t GPSPSystemModInitialize(void)
{
    GPSPSystemModStart();

    return 0;
}

MODULE_INITCALL(GPSPSystemModInitialize, 0);
/**
 * System task, periodically executes every SYSTEM_UPDATE_PERIOD_MS
 */
static void gpspSystemTask(__attribute__((unused)) void *parameters)
{
#ifdef PIOS_INCLUDE_WDG
    PIOS_WDG_UpdateFlag(PIOS_WDG_SYSTEM);
#endif
    /* create all modules thread */
    MODULE_TASKCREATE_ALL;

    if (mallocFailed) {
        // Nothing to do, this condition needs to be trapped during development.
        while (true) {
            ;
        }
    }

#if defined(PIOS_INCLUDE_IAP)
    PIOS_IAP_WriteBootCount(0);
#endif
    /* Right now there is no configuration and uart speed is fixed at 57600.
     * TODO:
     * 1) add a tiny ubx parser on gps side to intercept CFG-RINV and use that for config storage;
     * 2) second ubx parser on uart side that intercept custom configuration message and flash commands.
     */
    PIOS_COM_ChangeBaud(pios_com_main_id, GPS_MODULE_DEFAULT_BAUDRATE);
    setupGPS();
    uint32_t ledTimer = 0;
    static TickType_t lastUpdate;
    readFirmwareInfo();

    while (1) {
#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_SYSTEM);
#endif
        uint32_t ledPeriod = PIOS_DELAY_DiffuS(ledTimer) / 1000;
        if (ledPeriod < HB_LED_BLINK_ON_PERIOD_MS) {
            PIOS_LED_Off(PIOS_LED_HEARTBEAT);
        } else {
            PIOS_LED_On(PIOS_LED_HEARTBEAT);
        }
        if (ledPeriod > (HB_LED_BLINK_ON_PERIOD_MS + HB_LED_BLINK_OFF_PERIOD_MS)) {
            ledTimer = PIOS_DELAY_GetRaw();
        }

        handleGPS();
        handleMag();
        updateStats();
        vTaskDelayUntil(&lastUpdate, SYSTEM_UPDATE_PERIOD_MS * configTICK_RATE_HZ / 1000);
    }
}


/**
 * Called periodically to update the system stats
 */
uint16_t GetFreeIrqStackSize(void)
{
    uint32_t i = 0x150;

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
    static uint32_t lastUpdate;

    if (PIOS_DELAY_DiffuS(lastUpdate) < STAT_UPDATE_PERIOD_MS * 1000) {
        return;
    }
    lastUpdate = PIOS_DELAY_GetRaw();

    // Get stats and update
    sysPkt.fragments.data.flightTime = xTaskGetTickCount() * portTICK_RATE_MS;
    sysPkt.fragments.data.options    = SYS_DATA_OPTIONS_MAG | (flash_available() ? SYS_DATA_OPTIONS_FLASH : 0);
    ubx_buildPacket(&sysPkt.packet, UBX_OP_CUST_CLASS, UBX_OP_SYS, sizeof(SysData));
    PIOS_COM_SendBuffer(pios_com_main_id, sysPkt.packet.binarystream, sizeof(SysUbxPkt));
}

// retrieve firmware info and fill syspkt
static void readFirmwareInfo()
{
    const struct pios_board_info *bdinfo = &pios_board_info_blob;

    sysPkt.fragments.data.board_revision = bdinfo->board_rev;
    sysPkt.fragments.data.board_type     = bdinfo->board_type;
    struct fw_version_info *fwinfo = (struct fw_version_info *)(bdinfo->fw_base + bdinfo->fw_size);

    memcpy(&sysPkt.fragments.data.commit_tag_name, &fwinfo->commit_tag_name, sizeof(sysPkt.fragments.data.commit_tag_name));
    memcpy(&sysPkt.fragments.data.sha1sum, &fwinfo->commit_tag_name, sizeof(sysPkt.fragments.data.sha1sum));
}

/**
 * Called by the RTOS when the CPU is idle,
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
