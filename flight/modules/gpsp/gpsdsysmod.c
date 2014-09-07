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
//#define PIOS_COM_DEBUG pios_com_main_id
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
#define SYSTEM_UPDATE_PERIOD_MS      10

#define STACK_SIZE_BYTES             450

#define TASK_PRIORITY                (tskIDLE_PRIORITY + 1)
#define BUFFER_SIZE                  128
uint8_t buffer[BUFFER_SIZE];

// Private types

// Private variables
static xTaskHandle systemTaskHandle;
static enum { STACKOVERFLOW_NONE = 0, STACKOVERFLOW_WARNING = 1, STACKOVERFLOW_CRITICAL = 3 } stackOverflow;
static bool mallocFailed;
static void ReadGPS();
static void ReadMag();
static void InitGPS();
// Private functions
static void updateStats();
static void gps_lowlevel_init();
static void gpspSystemTask(void *parameters);

static uint32_t I2C_ReadBuffer(I2C_TypeDef* I2Cx,uint8_t* pBuffer, uint8_t reg, uint16_t* NumByteToRead, uint8_t deviceAddress);

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
    InitGPS();
    /* start the delayed callback scheduler */
    // PIOS_CALLBACKSCHEDULER_Start();

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
        // Update the system alarms
        PIOS_COM_SendBuffer(pios_com_main_id, (uint8_t *)&"0000", 4);
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
    // Get stats and update
    systemStats.FlightTime = xTaskGetTickCount() * portTICK_RATE_MS;
    systemStats.HeapRemaining = xPortGetFreeHeapSize();
    systemStats.SystemModStackRemaining = uxTaskGetStackHighWaterMark(NULL) * 4;

    // Get Irq stack status
    systemStats.IRQStackRemaining = GetFreeIrqStackSize();

    systemStats.CPULoad = 100 - PIOS_TASK_MONITOR_GetIdlePercentage();
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

union {
    uint8_t outbuff[11];
    struct {
        uint8_t pre;
        uint8_t header[3];
        int16_t mag[3];
        uint8_t tail;
    } dataMag;
    struct {
        uint8_t pre;
        uint8_t header[3];
        uint8_t data[4];
        uint8_t tail;
    } dataId;
} magbuf;


void ReadMag()
{
    PIOS_HMC5x83_ReadID(magbuf.dataId.data);
    magbuf.dataId.data[3]   = '!';
    magbuf.dataId.pre       = '\n';
    magbuf.dataId.header[0] = 'M';
    magbuf.dataId.header[1] = 'a';
    magbuf.dataId.header[2] = 'g';
    magbuf.dataId.tail      = '\n';


    if (PIOS_HMC5x83_ReadMag(magbuf.dataMag.mag) < 0) {
        PIOS_COM_SendBuffer(pios_com_main_id, magbuf.outbuff, 9);
        return;
    }
    PIOS_COM_SendBuffer(pios_com_main_id, magbuf.outbuff, 11);
}

void ReadGPS(){
    uint16_t bytesToRead = 2;
    uint16_t bytesToSend = 0;
    uint8_t tmp[2];
    if(I2C_ReadBuffer(I2C1,tmp, GPS_I2C_STREAM_SIZE_HIGH_REG,&bytesToRead, GPS_I2C_ADDRESS) == 0){
        bytesToRead = (tmp[0] << 8) | tmp[1];
        bytesToRead = bytesToRead < 255 ? bytesToRead : 254;
        bytesToSend = bytesToRead;
        if(bytesToRead > 0 && I2C_ReadBuffer(I2C1,buffer, GPS_I2C_STREAM_REG,&bytesToRead, GPS_I2C_ADDRESS) == 0){
            PIOS_COM_SendBuffer(pios_com_main_id,buffer, bytesToSend - bytesToRead);
        }
    }
}


void InitGPS(){
    gps_lowlevel_init();
    I2C_InitTypeDef  I2C_InitStructure;
    memset(buffer,0 ,32);
    /* I2C configuration */
    /* sEE_I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
    I2C_InitStructure.I2C_DigitalFilter = 0x00;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_Timing = 0x00210507;

    /* Apply sEE_I2C configuration after enabling it */
    I2C_Init(I2C1, &I2C_InitStructure);

    /* sEE_I2C Peripheral Enable */
    I2C_Cmd(I2C1, ENABLE);
}

void gps_lowlevel_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Configure the I2C clock source. The clock is derived from the HSI */
  RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);

  /* sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

  /* sEE_I2C Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

  /* Connect PXx to I2C_SCL*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_1);

  /* Connect PXx to I2C_SDA*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_1);

  /* GPIO configuration */
  /* Configure sEE_I2C pins: SCL */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Configure sEE_I2C pins: SDA */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

#define sEE_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define sEE_LONG_TIMEOUT         ((uint32_t)(10 * sEE_FLAG_TIMEOUT))

uint32_t I2C_ReadBuffer(I2C_TypeDef* I2Cx,uint8_t* pBuffer, uint8_t reg, uint16_t* NumByteToRead, uint8_t deviceAddress)
{
  static volatile uint32_t  sEETimeout = sEE_LONG_TIMEOUT;
  uint32_t NumbOfSingle = 0, DataNum = 0;

  /* Get number of reload cycles */
  NumbOfSingle = (*NumByteToRead) % 255;



  /* Configure slave address, nbytes, reload and generate start */
  I2C_TransferHandling(I2Cx, deviceAddress, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);

  /* Wait until TXIS flag is set */
  sEETimeout = sEE_LONG_TIMEOUT;
  while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS) == RESET)
  {
    if((sEETimeout--) == 0)
        return -2;
  }

  /* Send memory address */
  I2C_SendData(I2Cx, (uint8_t)reg);

  /* Wait until TC flag is set */
  sEETimeout = sEE_LONG_TIMEOUT;
  while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TC) == RESET)
  {
    if((sEETimeout--) == 0)
        return -2;
  }

    /* Update CR2 : set Slave Address , set read request, generate Start and set end mode */
    I2C_TransferHandling(I2Cx, deviceAddress, (uint32_t)(NumbOfSingle), I2C_AutoEnd_Mode, I2C_Generate_Start_Read);

    /* Reset local variable */
    DataNum = 0;

    /* Wait until all data are received */
    while (DataNum != NumbOfSingle)
    {
        /* Wait until RXNE flag is set */
        sEETimeout = sEE_LONG_TIMEOUT;
        while(I2C_GetFlagStatus(I2Cx, I2C_ISR_RXNE) == RESET)
        {
            if((sEETimeout--) == 0)
                return -2;
        }

        /* Read data from RXDR */
        pBuffer[DataNum]= I2C_ReceiveData(I2Cx);

        /* Update number of received data */
        DataNum++;
        (*NumByteToRead)--;
    }
    return 0;
}

/**
 * @}
 * @}
 */
