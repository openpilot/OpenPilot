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
#include <pios_helpers.h>
// private includes
#include "inc/gpsdsysmod.h"
#include <pios_hmc5x83.h>
#include <pios_ubx_ddc.h>

// UAVOs
#include <systemstats.h>
SystemStatsData systemStats;

extern uintptr_t flash_id;
#define DEBUG_THIS_FILE
extern uint32_t pios_com_main_id;
// #define PIOS_COM_DEBUG pios_com_main_id
#if defined(PIOS_INCLUDE_DEBUG_CONSOLE) && defined(DEBUG_THIS_FILE)
#define DEBUG_MSG(format, ...) PIOS_COM_SendFormattedString(PIOS_COM_DEBUG, format,##__VA_ARGS__)
#else
#define DEBUG_MSG(format, ...)
#endif

static bool getLastSentence(uint8_t *data, uint16_t bufferCount, uint8_t * *lastSentence, uint16_t *lenght);
extern struct pios_flash_driver pios_jedec_flash_driver;
extern uintptr_t flash_id;

// Private constants
#define SYSTEM_UPDATE_PERIOD_MS 2

#define STACK_SIZE_BYTES        450
#define MAG_RATE                50
#define STAT_RATE               1
#define TASK_PRIORITY           (tskIDLE_PRIORITY + 1)
#define BUFFER_SIZE             200
uint8_t buffer[BUFFER_SIZE];
// Private types

// Private variables
static xTaskHandle systemTaskHandle;
static enum { STACKOVERFLOW_NONE = 0, STACKOVERFLOW_WARNING = 1, STACKOVERFLOW_CRITICAL = 3 } stackOverflow;
static bool mallocFailed;
static void ReadGPS();
static void ReadMag();
static void SetupGPS();
// Private functions
static void updateStats();

static void gpspSystemTask(void *parameters);

const char cfg_settings[] =
    // cfg-prt I2C. In UBX+RTCM, Out UBX, Slave Addr 0x42
    "\xB5\x62\x06\x00\x14\x00\x00\x00\x00\x00\x84\x00\x00\x00\x00\x00\x00\x00\x07\x00\x01\x00\x00\x00\x00\x00\xA6\xC6"
    // cfg-msg: nav-pvt rate 1
    "\xB5\x62\x06\x01\x03\x00\x01\x07\x01\x13\x51"
    // cfg-msg: nav-svinfo rate 10
    "\xB5\x62\x06\x01\x03\x00\x01\x30\x0A\x45\xAC"
    // CFG-RATE meas period 100ms, nav rate 1
    "\xB5\x62\x06\x08\x06\x00\x64\x00\x01\x00\x01\x00\x7A\x12";

typedef struct {
    uint8_t  syn1;
    uint8_t  syn2;
    uint8_t  class;
    uint8_t  id;
    uint16_t len;
} __attribute__((packed)) UBXHeader_t;

typedef struct {
    uint8_t chk1;
    uint8_t chk2;
} __attribute__((packed)) UBXFooter_t;

typedef union {
    uint8_t bynarystream[0];
    struct {
        UBXHeader_t header;
        uint8_t     payload[0];
    } packet;
} UBXPacket_t;


typedef struct {
    int16_t  X;
    int16_t  Y;
    int16_t  Z;
    uint16_t status;
} __attribute__((packed)) MagData;

typedef union {
    struct {
        UBXHeader_t header;
        MagData     data;
        UBXFooter_t footer;
    } __attribute__((packed)) fragments;
    UBXPacket_t packet;
} MagUbxPkt;

typedef struct {
    uint32_t flightTime;
    uint16_t HeapRemaining;
    uint16_t IRQStackRemaining;
    uint16_t SystemModStackRemaining;
    uint16_t options;
} __attribute__((packed)) SysData;

typedef union {
    struct {
        UBXHeader_t header;
        SysData     data;
        UBXFooter_t footer;
    } fragments;
    UBXPacket_t packet;
} SysUbxPkt;

void ubx_appendChecksum_message(UBXPacket_t *pkt);
void ubx_build_packet(UBXPacket_t *pkt, uint8_t packetClass, uint8_t packetId, uint16_t len);

#define SYS_DATA_OPTIONS_FLASH 0x01
#define UBX_HEADER_LEN         (sizeof(UBXHeader_t))

#define UBX_SYN1               0xB5
#define UBX_SYN2               0x62

#define UBX_OP_CUST_CLASS      0x99
#define UBX_OP_SYS             0x01
#define UBX_OP_MAG             0x02

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
    static TickType_t lastUpdate;
    SetupGPS();
    uint8_t counter = 0;
    while (1) {
        // NotificationUpdateStatus();
        // Update the system statistics
        if (!(++counter & 0x7F)) {
            PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
        }
        vTaskDelayUntil(&lastUpdate, SYSTEM_UPDATE_PERIOD_MS * configTICK_RATE_HZ / 1000);

        ReadGPS();
        ReadMag();
        updateStats();
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
SysUbxPkt sysPkt;
static void updateStats()
{
    static uint32_t lastUpdate;

    if (PIOS_DELAY_DiffuS(lastUpdate) < 1000 * configTICK_RATE_HZ / STAT_RATE) {
        return;
    }
    lastUpdate = PIOS_DELAY_GetRaw();
    // Get stats and update
    sysPkt.fragments.data.flightTime        = xTaskGetTickCount() * portTICK_RATE_MS;
    sysPkt.fragments.data.HeapRemaining     = xPortGetFreeHeapSize();
    sysPkt.fragments.data.IRQStackRemaining = GetFreeIrqStackSize();
    sysPkt.fragments.data.SystemModStackRemaining = uxTaskGetStackHighWaterMark(NULL) * 4;
    sysPkt.fragments.data.options = flash_id > 0 ? SYS_DATA_OPTIONS_FLASH : 0;
    ubx_build_packet(&sysPkt.packet, UBX_OP_CUST_CLASS, UBX_OP_SYS, sizeof(SysData));
    PIOS_COM_SendBuffer(pios_com_main_id, sysPkt.packet.bynarystream, sizeof(SysUbxPkt));
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


void ReadMag()
{
    static uint32_t lastUpdate;

    if (PIOS_DELAY_DiffuS(lastUpdate) < 1000 * configTICK_RATE_HZ / MAG_RATE) {
        return;
    }
    lastUpdate = PIOS_DELAY_GetRaw();
    static int16_t mag[3];

    if (PIOS_HMC5x83_ReadMag(mag) == 0) {
        MagUbxPkt magPkt;
        // swap axis so that if side with connector is aligned to revo side with connectors, mags data are aligned
        magPkt.fragments.data.X = mag[0];
        magPkt.fragments.data.Y = - mag[1];
        magPkt.fragments.data.Z = mag[2];
        magPkt.fragments.data.status = 1;
        ubx_build_packet(&magPkt.packet, UBX_OP_CUST_CLASS, UBX_OP_MAG, sizeof(MagData));
        PIOS_COM_SendBuffer(pios_com_main_id, magPkt.packet.bynarystream, sizeof(MagUbxPkt));
        return;
    }
}

uint32_t lastUnsentData = 0;
void ReadGPS()
{
    bool completeSentenceSent = false;
    int8_t maxCount = 3;

    do {
        int32_t datacounter = PIOS_UBX_DDC_GetAvailableBytes(PIOS_I2C_GPS);
        if (datacounter > 0) {
            uint8_t toRead = (uint32_t)datacounter > BUFFER_SIZE - lastUnsentData ? BUFFER_SIZE - lastUnsentData : (uint8_t)datacounter;
            uint8_t toSend = toRead;
            PIOS_UBX_DDC_ReadData(PIOS_I2C_GPS, buffer, toRead);

            uint8_t *lastSentence;
            static uint16_t lastSentenceLenght;
            completeSentenceSent = getLastSentence(buffer, toRead, &lastSentence, &lastSentenceLenght);
            if (completeSentenceSent) {
                toSend = (uint8_t)(lastSentence - buffer + lastSentenceLenght);
            } else {
                lastUnsentData = 0;
            }

            PIOS_COM_SendBuffer(pios_com_main_id, buffer, toSend);
            if (toRead > toSend) {
                // move unsent data at the beginning of buffer to be sent next time
                lastUnsentData = toRead - toSend;
                memcpy(buffer, (buffer + toSend), lastUnsentData);
            }
        }

        datacounter = PIOS_COM_ReceiveBuffer(pios_com_main_id, buffer, BUFFER_SIZE, 0);
        if (datacounter > 0) {
            PIOS_UBX_DDC_WriteData(PIOS_I2C_GPS, buffer, datacounter);
        }
    } while (maxCount--);
}


bool getLastSentence(uint8_t *data, uint16_t bufferCount, uint8_t * *lastSentence, uint16_t *lenght)
{
    const uint8_t packet_overhead = UBX_HEADER_LEN + 2;
    uint8_t *current = data + bufferCount - packet_overhead;

    while (current >= data) {
        // look for a ubx a sentence
        if (current[0] == UBX_SYN1 && current[1] == UBX_SYN2) {
            // check whether it fits the current buffer (whole sentence is into buffer)
            uint16_t len = current[4] + (current[5] << 8);
            if (len + packet_overhead + current <= data + bufferCount) {
                *lastSentence = current;
                *lenght = len + packet_overhead;
                return true;
            }
        }
        current--;
    }
    // no complete sentence found
    return false;
}


typedef struct {
    uint8_t size;
    const uint8_t *sentence;
} ubx_init_sentence;



const ubx_init_sentence gps_config[] = {
    [0] = {
        .sentence = (uint8_t *)cfg_settings,
        .size     = sizeof(cfg_settings),
    },
};
void ubx_build_packet(UBXPacket_t *pkt, uint8_t packetClass, uint8_t packetId, uint16_t len)
{
    pkt->packet.header.syn1 = UBX_SYN1;
    pkt->packet.header.syn2 = UBX_SYN2;

    // don't make any assumption on alignments...
    ((uint8_t *)&pkt->packet.header.len)[0] = len & 0xFF;
    ((uint8_t *)&pkt->packet.header.len)[1] = (len >> 8) & 0xFF;

    pkt->packet.header.class = packetClass;
    pkt->packet.header.id    = packetId;
    ubx_appendChecksum_message(pkt);
}

void ubx_appendChecksum_message(UBXPacket_t *pkt)
{
    uint8_t chkA = 0;
    uint8_t chkB = 0;
    uint16_t len = ((uint8_t *)&pkt->packet.header.len)[0] | ((uint8_t *)&pkt->packet.header.len)[1] << 8;

    // From class field to the end of payload
    for (uint8_t i = 2; i < len + UBX_HEADER_LEN; i++) {
        chkA += pkt->bynarystream[i];
        chkB += chkA;
    }
    ;
    pkt->packet.payload[len]     = chkA;
    pkt->packet.payload[len + 1] = chkB;
}

void SetupGPS()
{
    for (uint8_t i = 0; i < NELEMENTS(gps_config); i++) {
        PIOS_UBX_DDC_WriteData(PIOS_I2C_GPS, gps_config[i].sentence, gps_config[i].size);
    }
}
/**
 * @}
 * @}
 */
