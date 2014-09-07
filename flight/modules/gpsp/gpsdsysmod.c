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
#include <pios_struct_helper.h>
// private includes
#include "inc/gpsdsysmod.h"
#include <pios_hmc5x83.h>


// UAVOs
#include <systemstats.h>
SystemStatsData systemStats;

#define DEBUG_THIS_FILE
extern uint32_t pios_com_main_id;
// #define PIOS_COM_DEBUG pios_com_main_id
#if defined(PIOS_INCLUDE_DEBUG_CONSOLE) && defined(DEBUG_THIS_FILE)
#define DEBUG_MSG(format, ...) PIOS_COM_SendFormattedString(PIOS_COM_DEBUG, format,##__VA_ARGS__)
#else
#define DEBUG_MSG(format, ...)
#endif

#define GPS_I2C_ADDRESS              (0x42 << 1)
#define GPS_I2C_STREAM_REG           0xFF
#define GPS_I2C_STREAM_SIZE_HIGH_REG 0xFD
#define GPS_I2C_STREAM_SIZE_LOW_REG  0xFE

// Private constants
#define SYSTEM_UPDATE_PERIOD_MS      2

#define STACK_SIZE_BYTES             450

#define TASK_PRIORITY                (tskIDLE_PRIORITY + 1)
#define BUFFER_SIZE                  128
uint8_t buffer[BUFFER_SIZE];
char outbuf[40];

// Private types

// Private variables
static xTaskHandle systemTaskHandle;
static enum { STACKOVERFLOW_NONE = 0, STACKOVERFLOW_WARNING = 1, STACKOVERFLOW_CRITICAL = 3 } stackOverflow;
static bool mallocFailed;
static void ReadGPS();
static void ReadMag();

// Private functions
static void updateStats();

static void gpspSystemTask(void *parameters);


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
    /* create all modules thread */
    MODULE_TASKCREATE_ALL;

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

    while (1) {
        // NotificationUpdateStatus();
        // Update the system statistics
        updateStats();
        PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
        vTaskDelay(SYSTEM_UPDATE_PERIOD_MS * configTICK_RATE_HZ / 1000);
        ReadGPS();
        ReadMag();
    }
}


/**
 * Called periodically to update the system stats
 */
static uint16_t GetFreeIrqStackSize(void)
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
    // Get stats and update
    systemStats.FlightTime    = xTaskGetTickCount() * portTICK_RATE_MS;
    systemStats.HeapRemaining = xPortGetFreeHeapSize();
    systemStats.SystemModStackRemaining = uxTaskGetStackHighWaterMark(NULL) * 4;

    // Get Irq stack status
    systemStats.IRQStackRemaining = GetFreeIrqStackSize();
    uint8_t len = snprintf(outbuf, 40, "$POPSY,%u,%d,%d,%d*0\n",
                           (uint)systemStats.FlightTime,
                           systemStats.HeapRemaining,
                           systemStats.IRQStackRemaining,
                           systemStats.SystemModStackRemaining);
    PIOS_COM_SendBuffer(pios_com_main_id, (uint8_t *)outbuf, len);
}

/**
 * Update system alarms
 */
/**
 * Called by the RTOS when the CPU is idle,
 */
void vApplicationIdleHook(void)
{
    // NotificationOnboardLedsRun();
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


int16_t mag[3];
uint8_t data[4];

void ReadMag()
{
    PIOS_HMC5x83_ReadID(data);

    if (PIOS_HMC5x83_ReadMag(mag) == 0) {
        uint8_t len = snprintf(outbuf, 40, "$POPMG,%s,%d,%d,%d,*0\n", data, mag[0], mag[1], mag[2]);
        PIOS_COM_SendBuffer(pios_com_main_id, (uint8_t *)outbuf, len);
        return;
    }
}

void ReadGPS()
{
    uint16_t bytesToRead   = 2;
    uint8_t tmp[2];
    uint8_t addr_buffer[1] = { GPS_I2C_STREAM_SIZE_HIGH_REG };
    struct pios_i2c_txn txn_list[] = {
        {
            .info = __func__,
            .addr = GPS_I2C_ADDRESS,
            .rw   = PIOS_I2C_TXN_WRITE,
            .len  = 1,
            .buf  = addr_buffer,
        }
        ,
        {
            .info = __func__,
            .addr = GPS_I2C_ADDRESS,
            .rw   = PIOS_I2C_TXN_READ,
            .len  = 2,
            .buf  = tmp,
        }
    };

    if (PIOS_I2C_Transfer(PIOS_I2C_GPS, txn_list, NELEMENTS(txn_list)) == 0) {
        bytesToRead     = (tmp[0] << 8) | tmp[1];
        bytesToRead     = bytesToRead < 255 ? bytesToRead : 254;
        addr_buffer[0]  = GPS_I2C_STREAM_REG;
        txn_list[1].len = bytesToRead > BUFFER_SIZE ? BUFFER_SIZE : bytesToRead;
        txn_list[1].buf = buffer;
        if (bytesToRead > 0 && PIOS_I2C_Transfer(PIOS_I2C_GPS, txn_list, NELEMENTS(txn_list)) == 0) {
            PIOS_COM_SendBuffer(pios_com_main_id, buffer, bytesToRead);
        }
    }
}

/**
 * @}
 * @}
 */
