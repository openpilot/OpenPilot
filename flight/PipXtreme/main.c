/**
 ******************************************************************************
 *
 * @file       main.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Main modem functions
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


// *****************************************************************************

#define USE_WATCHDOG                    // comment this out if you don't want to use the watchdog

// *****************************************************************************
// OpenPilot Includes

#include <string.h>

#include "crc.h"
#include "aes.h"
#include "rfm22b.h"
#include "packet_handler.h"
#include "transparent_comms.h"
#include "api_comms.h"
#include "gpio_in.h"
#include "stopwatch.h"
#include "watchdog.h"
#include "saved_settings.h"

#include "main.h"

// *****************************************************************************
// ADC data

#define ADC_REF                         3.3f		// ADC reference voltage
#define ADC_FULL_RANGE                  4096		// 12 bit ADC

#define ADC_PSU_RESISTOR_TOP            10000.0f	// 10k upper resistor
#define ADC_PSU_RESISTOR_BOTTOM         2700.0f		// 2k7 lower resistor
#define ADC_PSU_RATIO                   (ADC_PSU_RESISTOR_BOTTOM / (ADC_PSU_RESISTOR_TOP + ADC_PSU_RESISTOR_BOTTOM))
#define	ADC_PSU_mV_SCALE                ((ADC_REF * 1000) / (ADC_FULL_RANGE * ADC_PSU_RATIO))

// *****************************************************************************
// Global Variables

uint32_t                flash_size;

char                    serial_number_str[25];
uint32_t                serial_number_crc32;

uint32_t                reset_addr;

bool                    API_Mode;

bool                    booting;

volatile uint32_t       random32;

// *****************************************************************************
// Local Variables

#if defined(USE_WATCHDOG)
  volatile uint16_t     watchdog_timer;
  uint16_t              watchdog_delay;
#endif

volatile bool           inside_timer_int;
volatile uint32_t       uptime_ms;

volatile uint16_t       second_tick_timer;
volatile bool           second_tick;

//volatile int32_t        temp_adc;
//volatile int32_t        psu_adc;

// *****************************************************************************

#if defined(USE_WATCHDOG)

  void processWatchdog(void)
  {
      // random32 = UpdateCRC32(random32, IWDG->SR);

      if (watchdog_timer < watchdog_delay)
          return;

      // the watchdog needs resetting

      watchdog_timer = 0;

      watchdog_Clear();
  }

  void enableWatchdog(void)
  {	// enable a watchdog
      watchdog_timer = 0;
      watchdog_delay = watchdog_Init(1000);	// 1 second watchdog timeout
  }

#endif

// *****************************************************************************
/*
void WWDG_IRQHandler(void)
{
    // Update WWDG counter
    WWDG_SetCounter(0x7F);

    // Clear EWI flag
    WWDG_ClearFlag();
}

void enableWatchdog(void)
{
	// Enable WWDG clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

	// On Value line devices, WWDG clock counter = (PCLK1 (24MHz)/4096)/8 = 732 Hz (~1366 æs)
	// On other devices, WWDG clock counter = (PCLK1(36MHz)/4096)/8 = 1099 Hz (~910 æs)
	WWDG_SetPrescaler(WWDG_Prescaler_8);

	// Set Window value to 65
	WWDG_SetWindowValue(65);

	// On Value line devices, Enable WWDG and set counter value to 127, WWDG timeout = ~1366 æs * 64 = 87.42 ms
	// On other devices, Enable WWDG and set counter value to 127, WWDG timeout = ~910 æs * 64 = 58.25 ms
	WWDG_Enable(127);

	// Clear EWI flag
	WWDG_ClearFlag();

	// Enable EW interrupt
	WWDG_EnableIT();
}
*/
// *****************************************************************************

void sequenceLEDs(void)
{
    for (int i = 0; i < 2; i++)
    {
        USB_LED_ON;
        PIOS_DELAY_WaitmS(80);
        USB_LED_OFF;

        LINK_LED_ON;
        PIOS_DELAY_WaitmS(80);
        LINK_LED_OFF;

        RX_LED_ON;
        PIOS_DELAY_WaitmS(80);
        RX_LED_OFF;

        TX_LED_ON;
        PIOS_DELAY_WaitmS(80);
        TX_LED_OFF;

        #if defined(USE_WATCHDOG)
            processWatchdog();		// process the watchdog
        #endif
    }
}

// *****************************************************************************
/*
void setup_SPI(void)
{
    SPI_InitTypeDef SPI_InitStructure;

	SPI_InitStructure.SPI_Mode = SPI_Mode_Master,
//	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave,

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex,
//	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_RxOnly,
//	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Rx,
//	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx,

//	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b,
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b,

	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft,
//	SPI_InitStructure.SPI_NSS = SPI_NSS_Hard,

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB,
//	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB,

	SPI_InitStructure.SPI_CRCPolynomial = 7,

//	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low,
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High,

//	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge,
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge,

//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2,		// fastest SCLK
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4,
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8,
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16,
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32,
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64,
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128,
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256,	// slowest SCLK

	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, ENABLE);
}
*/
// *****************************************************************************
// timer interrupt

void TIMER_INT_FUNC(void)
{
	inside_timer_int = TRUE;

	if (TIM_GetITStatus(TIMER_INT_TIMER, TIM_IT_Update) != RESET)
	{
		// Clear timer interrupt pending bit
		TIM_ClearITPendingBit(TIMER_INT_TIMER, TIM_IT_Update);

//		random32 = UpdateCRC32(random32, PIOS_DELAY_TIMER->CNT >> 8);
//		random32 = UpdateCRC32(random32, PIOS_DELAY_TIMER->CNT);

		uptime_ms++;

		#if defined(USE_WATCHDOG)
			watchdog_timer++;
		#endif

		// ***********

		if (!booting)
		{
			// ***********

			if (++second_tick_timer >= 1000)
			{
				second_tick_timer -= 1000;
				second_tick = TRUE;
			}

			// ***********

			// read the chip temperature
//			temp_adc = PIOS_ADC_PinGet(0);

			// read the psu voltage
//			psu_adc = PIOS_ADC_PinGet(1);

			// ***********

			rfm22_1ms_tick();			// rf module tick

			ph_1ms_tick();				// packet handler tick

			if (!API_Mode)
				trans_1ms_tick();		// transparent communications tick
			else
				api_1ms_tick();			// api communications tick

			// ***********
		}
	}

	inside_timer_int = FALSE;
}

void setup_TimerInt(uint16_t Hz)
{   // setup the timer interrupt

    // enable timer clock
    switch ((uint32_t)TIMER_INT_TIMER)
    {
        case (uint32_t)TIM1: RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); break;
        case (uint32_t)TIM2: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); break;
        case (uint32_t)TIM3: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); break;
        case (uint32_t)TIM4: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); break;
        #ifdef STM32F10X_HD
            case (uint32_t)TIM5: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); break;
            case (uint32_t)TIM6: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE); break;
            case (uint32_t)TIM7: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE); break;
            case (uint32_t)TIM8: RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE); break;
        #endif
    }

    // enable timer interrupt
    NVIC_InitTypeDef NVIC_InitStructure;
    switch ((uint32_t)TIMER_INT_TIMER)
    {
//      case (uint32_t)TIM1: NVIC_InitStructure.NVIC_IRQChannel = TIM1_IRQn; break;
        case (uint32_t)TIM2: NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; break;
        case (uint32_t)TIM3: NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; break;
        case (uint32_t)TIM4: NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn; break;
        #ifdef STM32F10X_HD
            case (uint32_t)TIM5: NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn; break;
            case (uint32_t)TIM6: NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn; break;
            case (uint32_t)TIM7: NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn; break;
//          case (uint32_t)TIM8: NVIC_InitStructure.NVIC_IRQChannel = TIM8_IRQn; break;
        #endif
    }
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIMER_INT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Time base configuration
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = ((1000000 / Hz) - 1);
    TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;	// For 1 uS accuracy
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIMER_INT_TIMER, &TIM_TimeBaseStructure);

    // Enable the CC2 Interrupt Request
    TIM_ITConfig(TIMER_INT_TIMER, TIM_IT_Update, ENABLE);

    // Clear update pending flag
    TIM_ClearFlag(TIMER_INT_TIMER, TIM_FLAG_Update);

    // Enable counter
    TIM_Cmd(TIMER_INT_TIMER, ENABLE);
}

// *****************************************************************************
// read the CPU's flash and ram sizes

void get_CPUDetails(void)
{
    // read the flash size
    flash_size = (uint32_t)mem16(0x1FFFF7E0) * 1024;

    int j = 0;

    // read the CPU electronic signature (12-bytes)
    serial_number_str[j] = 0;
    for (int i = 0; i < 12; i++)
    {
        uint8_t ms_nibble = mem8(0x1ffff7e8 + i) >> 4;
        uint8_t ls_nibble = mem8(0x1ffff7e8 + i) & 0x0f;
        if (j > sizeof(serial_number_str) - 3) break;
        serial_number_str[j++] = ((ms_nibble > 9) ? ('A' - 10) : '0') + ms_nibble;
        serial_number_str[j++] = ((ls_nibble > 9) ? ('A' - 10) : '0') + ls_nibble;
        serial_number_str[j] = 0;
    }

    // create a 32-bit crc from the serial number hex string
    serial_number_crc32 = UpdateCRC32Data(0xffffffff, serial_number_str, j);
    serial_number_crc32 = UpdateCRC32(serial_number_crc32, j);

//  reset_addr = (uint32_t)&Reset_Handler;
}

// *****************************************************************************

void init_RF_module(void)
{
    int i = -100;

    switch (saved_settings.frequency_band)
    {
        case freqBand_434MHz:
        case freqBand_868MHz:
        case freqBand_915MHz:
            i = rfm22_init(saved_settings.min_frequency_Hz, saved_settings.max_frequency_Hz, 50000);
            break;

        default:
            #if defined(PIOS_COM_DEBUG)
                DEBUG_PRINTF("UNKNOWN BAND ERROR\r\n", i);
            #endif

            for (int j = 0; j < 8; j++)
            {
                USB_LED_ON;
                LINK_LED_OFF;
                RX_LED_ON;
                TX_LED_OFF;

                PIOS_DELAY_WaitmS(200);

                USB_LED_OFF;
                LINK_LED_ON;
                RX_LED_OFF;
                TX_LED_ON;

                PIOS_DELAY_WaitmS(200);

                #if defined(USE_WATCHDOG)
                    processWatchdog();
                #endif
            }

            PIOS_DELAY_WaitmS(1000);

            PIOS_SYS_Reset();

            while (1);
            break;
    }

    if (i < 0)
    {	// RF module error .. flash the LED's

        #if defined(PIOS_COM_DEBUG)
            DEBUG_PRINTF("RF ERROR res: %d\r\n", i);
        #endif

        for (int j = 0; j < 6; j++)
        {
            USB_LED_ON;
            LINK_LED_ON;
            RX_LED_OFF;
            TX_LED_OFF;

            PIOS_DELAY_WaitmS(200);

            USB_LED_OFF;
            LINK_LED_OFF;
            RX_LED_ON;
            TX_LED_ON;

            PIOS_DELAY_WaitmS(200);

            #if defined(USE_WATCHDOG)
                processWatchdog();
            #endif
        }

        PIOS_DELAY_WaitmS(1000);

        PIOS_SYS_Reset();

        while (1);
    }

    // set carrier frequency
    rfm22_setNominalCarrierFrequency(saved_settings.frequency_Hz);

    ph_setDatarate(saved_settings.max_rf_bandwidth);

    ph_setTxPower(saved_settings.max_tx_power);
}

// *****************************************************************************
// find out what caused our reset and act on it

void processReset(void)
{
    if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
    {	// Independant Watchdog Reset

        #if defined(PIOS_COM_DEBUG)
            DEBUG_PRINTF("\r\nINDEPENDANT WATCHDOG CAUSED A RESET\r\n");
        #endif

        // all led's ON
        USB_LED_ON;
        LINK_LED_ON;
        RX_LED_ON;
        TX_LED_ON;

        PIOS_DELAY_WaitmS(500);	// delay a bit

        // all led's OFF
        USB_LED_OFF;
        LINK_LED_OFF;
        RX_LED_OFF;
        TX_LED_OFF;

    }
/*
	if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET)
	{	// Window Watchdog Reset

		DEBUG_PRINTF("\r\nWINDOW WATCHDOG CAUSED A REBOOT\r\n");

		// all led's ON
		USB_LED_ON;
		LINK_LED_ON;
		RX_LED_ON;
		TX_LED_ON;

		PIOS_DELAY_WaitmS(500);	// delay a bit

		// all led's OFF
		USB_LED_OFF;
		LINK_LED_OFF;
		RX_LED_OFF;
		TX_LED_OFF;
	}
*/
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
    {	// Power-On Reset
        #if defined(PIOS_COM_DEBUG)
            DEBUG_PRINTF("\r\nPOWER-ON-RESET\r\n");
        #endif
    }

    if (RCC_GetFlagStatus(RCC_FLAG_SFTRST) != RESET)
    {	// Software Reset
        #if defined(PIOS_COM_DEBUG)
            DEBUG_PRINTF("\r\nSOFTWARE RESET\r\n");
        #endif
    }

    if (RCC_GetFlagStatus(RCC_FLAG_LPWRRST) != RESET)
    {	// Low-Power Reset
        #if defined(PIOS_COM_DEBUG)
            DEBUG_PRINTF("\r\nLOW POWER RESET\r\n");
        #endif
    }

    if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
    {	// Pin Reset
        #if defined(PIOS_COM_DEBUG)
            DEBUG_PRINTF("\r\nPIN RESET\r\n");
        #endif
    }

    // Clear reset flags
    RCC_ClearFlag();
}

// *****************************************************************************
// Main function

int main()
{
    // *************
    // init various variables

    booting = TRUE;

    inside_timer_int = FALSE;

    uptime_ms = 0;

    flash_size = 0;

    serial_number_str[0] = 0;
    serial_number_crc32 = 0;

    reset_addr = 0;

//    temp_adc = -1;
//    psu_adc = -1;

    API_Mode = FALSE;
//      API_Mode = TRUE;			// TEST ONLY

    second_tick_timer = 0;
    second_tick = FALSE;

    saved_settings.frequency_band = freqBand_UNKNOWN;

    // *************

    // Bring up System using CMSIS functions, enables the LEDs.
    PIOS_SYS_Init();

    // turn all the leds on
    USB_LED_ON;
    LINK_LED_ON;
    RX_LED_ON;
    TX_LED_ON;

    CRC_init();

    // read the CPU details
    get_CPUDetails();

    // Delay system
    PIOS_DELAY_Init();

    // UART communication system
    PIOS_COM_Init();

    // ADC system
//    PIOS_ADC_Init();

    // SPI link to master
    PIOS_SPI_Init();

    // setup the GPIO input pins
    GPIO_IN_Init();

    // *************
    // set various GPIO pin states

    // uart serial RTS line high
    SERIAL_RTS_ENABLE;
    SERIAL_RTS_SET;

    // RF module chip-select line high
    RF_CS_ENABLE;
    RF_CS_HIGH;

    // EEPROM chip-select line high
    EE_CS_ENABLE;
    EE_CS_HIGH;

    // *************

    random32 ^= serial_number_crc32;                    // try to randomize the random number
//      random32 ^= serial_number_crc32 ^ CRC_IDR;      // try to randomize the random number

    ph_init(serial_number_crc32, 128000, 0);            // initialise the packet handler

    trans_init();                                       // initialise the tranparent communications module

    api_init();    		                                // initialise the API communications module

    setup_TimerInt(1000);                               // setup a 1kHz timer interrupt

#if defined(USE_WATCHDOG)
    enableWatchdog();                                   // enable the watchdog
#endif

    // *************
    // do a simple LED flash test sequence so the user knows all the led's are working and that we have booted

    PIOS_DELAY_WaitmS(100);

    // turn all the leds off
    USB_LED_OFF;
    LINK_LED_OFF;
    RX_LED_OFF;
    TX_LED_OFF;

    PIOS_DELAY_WaitmS(200);

    sequenceLEDs();

    // turn all the leds off
    USB_LED_OFF;
    LINK_LED_OFF;
    RX_LED_OFF;
    TX_LED_OFF;

    // *************
    // debug stuff

    #if defined(PIOS_COM_DEBUG)
        DEBUG_PRINTF("\r\n");
        DEBUG_PRINTF("PipXtreme v%u.%u rebooted\r\n", version_major, version_minor);
    #endif

    // *************
    // initialize the saved settings module

    saved_settings_init();

    // *************
    // read the API mode pin

    if (!GPIO_IN(API_MODE_PIN))
        API_Mode = TRUE;

    // *************
    // read the 434/868/915 jumper options

    if (!GPIO_IN(_868MHz_PIN) && !GPIO_IN(_915MHz_PIN)) saved_settings.frequency_band = freqBand_434MHz;    // 434MHz band
    else
    if (!GPIO_IN(_868MHz_PIN) &&  GPIO_IN(_915MHz_PIN)) saved_settings.frequency_band = freqBand_868MHz;    // 868MHz band
    else
    if ( GPIO_IN(_868MHz_PIN) && !GPIO_IN(_915MHz_PIN)) saved_settings.frequency_band = freqBand_915MHz;    // 915MHz band

    switch (saved_settings.frequency_band)
    {
        case freqBand_434MHz:
//            if (saved_settings.frequency_Hz == 0xffffffff)
            {
                saved_settings.frequency_Hz = 434000000;
                saved_settings.min_frequency_Hz = 434000000 - 2000000;
                saved_settings.max_frequency_Hz = 434000000 + 2000000;
            }
//            if (saved_settings.max_rf_bandwidth == 0xffffffff)
            {
    //          saved_settings.max_rf_bandwidth = 500;
    //          saved_settings.max_rf_bandwidth = 1000;
    //          saved_settings.max_rf_bandwidth = 2000;
    //          saved_settings.max_rf_bandwidth = 4000;
    //          saved_settings.max_rf_bandwidth = 8000;
    //          saved_settings.max_rf_bandwidth = 9600;
    //          saved_settings.max_rf_bandwidth = 16000;
    //          saved_settings.max_rf_bandwidth = 19200;
    //          saved_settings.max_rf_bandwidth = 24000;
    //          saved_settings.max_rf_bandwidth = 32000;
    //          saved_settings.max_rf_bandwidth = 64000;
                saved_settings.max_rf_bandwidth = 128000;
    //          saved_settings.max_rf_bandwidth = 192000;
            }
//            if (saved_settings.max_tx_power == 0xff)
            {
    //          saved_settings.max_tx_power = 0;        // +1dBm ... 1.25mW
    //          saved_settings.max_tx_power = 1;        // +2dBm ... 1.6mW
    //          saved_settings.max_tx_power = 2;        // +5dBm ... 3.16mW
    //          saved_settings.max_tx_power = 3;        // +8dBm ... 6.3mW
                saved_settings.max_tx_power = 4;        // +11dBm .. 12.6mW
    //          saved_settings.max_tx_power = 5;        // +14dBm .. 25mW
    //          saved_settings.max_tx_power = 6;        // +17dBm .. 50mW
    //          saved_settings.max_tx_power = 7;        // +20dBm .. 100mW
            }
            break;

        case freqBand_868MHz:
//            if (saved_settings.frequency_Hz == 0xffffffff)
            {
                saved_settings.frequency_Hz = 868000000;
                saved_settings.min_frequency_Hz = 868000000 - 10000000;
                saved_settings.max_frequency_Hz = 868000000 + 10000000;
            }
//            if (saved_settings.max_rf_bandwidth == 0xffffffff)
            {
    //          saved_settings.max_rf_bandwidth = 500;
    //          saved_settings.max_rf_bandwidth = 1000;
    //          saved_settings.max_rf_bandwidth = 2000;
    //          saved_settings.max_rf_bandwidth = 4000;
    //          saved_settings.max_rf_bandwidth = 8000;
    //          saved_settings.max_rf_bandwidth = 9600;
    //          saved_settings.max_rf_bandwidth = 16000;
    //          saved_settings.max_rf_bandwidth = 19200;
    //          saved_settings.max_rf_bandwidth = 24000;
    //          saved_settings.max_rf_bandwidth = 32000;
    //          saved_settings.max_rf_bandwidth = 64000;
                saved_settings.max_rf_bandwidth = 128000;
    //          saved_settings.max_rf_bandwidth = 192000;
            }
//            if (saved_settings.max_tx_power == 0xff)
            {
    //          saved_settings.max_tx_power = 0;        // +1dBm ... 1.25mW
    //          saved_settings.max_tx_power = 1;        // +2dBm ... 1.6mW
    //          saved_settings.max_tx_power = 2;        // +5dBm ... 3.16mW
    //          saved_settings.max_tx_power = 3;        // +8dBm ... 6.3mW
                saved_settings.max_tx_power = 4;        // +11dBm .. 12.6mW
    //          saved_settings.max_tx_power = 5;        // +14dBm .. 25mW
    //          saved_settings.max_tx_power = 6;        // +17dBm .. 50mW
    //          saved_settings.max_tx_power = 7;        // +20dBm .. 100mW
            }
            break;

        case freqBand_915MHz:
//            if (saved_settings.frequency_Hz == 0xffffffff)
            {
                saved_settings.frequency_Hz = 915000000;
                saved_settings.min_frequency_Hz = 915000000 - 13000000;
                saved_settings.max_frequency_Hz = 915000000 + 13000000;
            }
//            if (saved_settings.max_rf_bandwidth == 0xffffffff)
            {
    //          saved_settings.max_rf_bandwidth = 500;
    //          saved_settings.max_rf_bandwidth = 1000;
    //          saved_settings.max_rf_bandwidth = 2000;
    //          saved_settings.max_rf_bandwidth = 4000;
    //          saved_settings.max_rf_bandwidth = 8000;
    //          saved_settings.max_rf_bandwidth = 9600;
    //          saved_settings.max_rf_bandwidth = 16000;
    //          saved_settings.max_rf_bandwidth = 19200;
    //          saved_settings.max_rf_bandwidth = 24000;
    //          saved_settings.max_rf_bandwidth = 32000;
    //          saved_settings.max_rf_bandwidth = 64000;
                saved_settings.max_rf_bandwidth = 128000;
    //          saved_settings.max_rf_bandwidth = 192000;
            }
//            if (saved_settings.max_tx_power == 0xff)
            {
    //          saved_settings.max_tx_power = 0;        // +1dBm ... 1.25mW
    //          saved_settings.max_tx_power = 1;        // +2dBm ... 1.6mW
    //          saved_settings.max_tx_power = 2;        // +5dBm ... 3.16mW
    //          saved_settings.max_tx_power = 3;        // +8dBm ... 6.3mW
                saved_settings.max_tx_power = 4;        // +11dBm .. 12.6mW
    //          saved_settings.max_tx_power = 5;        // +14dBm .. 25mW
    //          saved_settings.max_tx_power = 6;        // +17dBm .. 50mW
    //          saved_settings.max_tx_power = 7;        // +20dBm .. 100mW
            }
            break;

        default:
            break;
    }

    if (serial_number_crc32 == 0x176C1EC6) saved_settings.destination_id = 0xA524A3B0;  // modem 1, open a connection to modem 2
    else
    if (serial_number_crc32 == 0xA524A3B0) saved_settings.destination_id = 0x176C1EC6;  // modem 2, open a connection to modem 1

    // *************

    processReset();           // Determine what caused the reset/reboot

    // *************
    // debug stuff

    #if defined(PIOS_COM_DEBUG)
        DEBUG_PRINTF("\r\n");
        DEBUG_PRINTF("CPU flash size: %u\r\n", flash_size);
        DEBUG_PRINTF("CPU serial number: %s %08X\r\n", serial_number_str, serial_number_crc32);
//      DEBUG_PRINTF("Reset address: %08X\r\n", reset_addr);

        if (!API_Mode)
            DEBUG_PRINTF("TRANSPARENT mode\r\n");
        else
            DEBUG_PRINTF("API mode\r\n");

        switch (saved_settings.frequency_band)
        {
            case freqBand_UNKNOWN: DEBUG_PRINTF("UNKNOWN band\r\n"); break;
            case freqBand_434MHz:  DEBUG_PRINTF("434MHz band\r\n"); break;
            case freqBand_868MHz:  DEBUG_PRINTF("868MHz band\r\n"); break;
            case freqBand_915MHz:  DEBUG_PRINTF("915MHz band\r\n"); break;
        }
    #endif

    // *************
    // initialize the RF module

    init_RF_module();

    // *************
    // initialize the USB interface

    #if defined(PIOS_INCLUDE_USB_HID)
//        PIOS_USB_HID_Init(0); // this is not needed as it gets called by the com init routine .. thank you Ed!
    #endif

    // *************

    saved_settings_save();

    booting = FALSE;

    // *************

    ph_set_remote_serial_number(0, saved_settings.destination_id);

    #if defined(PIOS_COM_DEBUG)
        DEBUG_PRINTF("\r\n");
        DEBUG_PRINTF("RF datarate: %dbps\r\n", ph_getDatarate());
        DEBUG_PRINTF("RF frequency: %dHz\r\n", rfm22_getNominalCarrierFrequency());
        DEBUG_PRINTF("RF TX power: %d\r\n", ph_getTxPower());
    #endif

    // *************
    // Main executive loop

    for (;;)
    {
        random32 = UpdateCRC32(random32, PIOS_DELAY_TIMER->CNT >> 8);
        random32 = UpdateCRC32(random32, PIOS_DELAY_TIMER->CNT);

        if (second_tick)
        {
            second_tick = FALSE;

            // *************************
            // display the up-time .. HH:MM:SS

//            #if defined(PIOS_COM_DEBUG)
//		int32_t _uptime_ms = uptime_ms;
//		uint32_t _uptime_sec = _uptime_ms / 1000;
//		DEBUG_PRINTF("Uptime: %02d:%02d:%02d.%03d\r\n", _uptime_sec / 3600, (_uptime_sec / 60) % 60, _uptime_sec % 60, _uptime_ms % 1000);
//	    #endif

            // *************************
            // process the Temperature

//	    if (temp_adc >= 0)
//	    {
//				int32_t degress_C = temp_adc;
//
//				#if defined(PIOS_COM_DEBUG)
//					DEBUG_PRINTF("TEMP: %d %d\r\n", temp_adc, degress_C);
//				#endif
//	    }

            // *************************
            // process the PSU voltage level

//            if (psu_adc >= 0)
//            {
//               int32_t psu_mV = psu_adc * ADC_PSU_mV_SCALE;
//
//                #if defined(PIOS_COM_DEBUG)
//                  DEBUG_PRINTF("PSU: %d, %dmV\r\n", psu_adc, psu_mV);
//                #endif
//            }

            // *************************


//          rfm22_setTxCarrierMode();	// TEST ONLY
        }

        rfm22_process();				// rf hardware layer processing

        ph_process();					// packet handler processing

        if (!API_Mode)
          trans_process();			// tranparent local communication processing (serial port and usb port)
        else
          api_process();			// API local communication processing (serial port and usb port)




















        // ******************
        // TEST ONLY ... get/put data over the radio link .. speed testing .. comment out trans_process() and api_process() if testing with this
/*
		int connection_index = 0;

		if (ph_connected(connection_index))
		{	// we have a connection to a remote modem

			uint8_t buffer[128];

			uint16_t num = ph_getData_used(connection_index);	// number of bytes waiting for us to get
			if (num > 0)
			{	// their is data in the received buffer - fetch it
				if (num > sizeof(buffer)) num = sizeof(buffer);
				num = ph_getData(connection_index, buffer, num);	// fetch the received data
			}

			// keep the tx buffer topped up
			num = ph_putData_free(connection_index);
			if (num > 0)
			{	// their is space available in the transmit buffer - top it up
				if (num > sizeof(buffer)) num = sizeof(buffer);
				for (int16_t i = 0; i < num; i++) buffer[i] = i;
				num = ph_putData(connection_index, buffer, num);
			}
		}
*/
		// ******************
















            #if defined(USE_WATCHDOG)
              processWatchdog();		// process the watchdog
            #endif
    }

    // *************
    // we should never arrive here

    // disable all interrupts
    PIOS_IRQ_Disable();

    // turn off all leds
    USB_LED_OFF;
    LINK_LED_OFF;
    RX_LED_OFF;
    TX_LED_OFF;

    PIOS_SYS_Reset();

    while (1);

    return 0;
}

// *****************************************************************************
