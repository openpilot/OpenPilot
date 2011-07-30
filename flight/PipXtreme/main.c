
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
#include "stream.h"
#include "ppm.h"
#include "transparent_comms.h"
//#include "api_comms.h"
#include "api_config.h"
#include "gpio_in.h"
#include "stopwatch.h"
#include "watchdog.h"
#include "saved_settings.h"

#include "main.h"

/* Prototype of PIOS_Board_Init() function */
extern void PIOS_Board_Init(void);

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

//volatile int32_t      temp_adc;
//volatile int32_t      psu_adc;

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
// timer interrupt

void TIMER_INT_FUNC(void)
{
	inside_timer_int = TRUE;

	if (TIM_GetITStatus(TIMER_INT_TIMER, TIM_IT_Update) != RESET)
	{
		// Clear timer interrupt pending bit
		TIM_ClearITPendingBit(TIMER_INT_TIMER, TIM_IT_Update);

//		random32 = UpdateCRC32(random32, PIOS_DELAY_GetuS() >> 8);
//		random32 = UpdateCRC32(random32, PIOS_DELAY_GetuS());

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

			uint8_t mode = saved_settings.mode;
//				modeTxBlankCarrierTest,	// blank carrier Tx mode (for calibrating the carrier frequency say)
//				modeTxSpectrumTest		// pseudo random Tx data mode (for checking the Tx carrier spectrum)

			rfm22_1ms_tick();			// rf module tick

			if (mode == MODE_NORMAL)
				ph_1ms_tick();			// packet handler tick

			if (mode == MODE_STREAM_TX || mode == MODE_STREAM_RX)
				stream_1ms_tick();		// continuous data stream tick

			if (mode == MODE_PPM_TX || mode == MODE_PPM_RX)
				ppm_1ms_tick();			// ppm tick

			if (!API_Mode)
				trans_1ms_tick();		// transparent communications tick
			else
//				api_1ms_tick();			// api communications tick
				apiconfig_1ms_tick();	// api communications tick

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
    TIM_TimeBaseStructure.TIM_Period = (1000000 / Hz) - 1;
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
    serial_number_crc32 = updateCRC32Data(0xffffffff, serial_number_str, j);
    serial_number_crc32 = updateCRC32(serial_number_crc32, j);

//  reset_addr = (uint32_t)&Reset_Handler;
}

// *****************************************************************************

void init_RF_module(void)
{
    int i = -100;

    switch (saved_settings.frequency_band)
    {
        case FREQBAND_434MHz:
        case FREQBAND_868MHz:
        case FREQBAND_915MHz:
            i = rfm22_init_normal(saved_settings.min_frequency_Hz, saved_settings.max_frequency_Hz, 50000);
            break;

        default:
            #if defined(PIOS_COM_DEBUG)
                DEBUG_PRINTF("UNKNOWN BAND ERROR\r\n\r\n", i);
            #endif

            for (int j = 0; j < 16; j++)
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
            DEBUG_PRINTF("RF ERROR res: %d\r\n\r\n", i);
        #endif

        for (int j = 0; j < 16; j++)
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

    rfm22_setFreqCalibration(saved_settings.rf_xtal_cap);
    rfm22_setNominalCarrierFrequency(saved_settings.frequency_Hz);
    rfm22_setDatarate(saved_settings.max_rf_bandwidth, TRUE);
    rfm22_setTxPower(saved_settings.max_tx_power);
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

//	API_Mode = FALSE;
    API_Mode = TRUE;			// TEST ONLY

    second_tick_timer = 0;
    second_tick = FALSE;

    saved_settings.frequency_band = FREQBAND_UNKNOWN;

    // *************

    PIOS_Board_Init();

    CRC_init();

    // read the CPU details
    get_CPUDetails();

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

    // PPM OUT low
    PPM_OUT_ENABLE;
    PPM_OUT_LOW;

    // pin high
    SPARE1_ENABLE;
    SPARE1_HIGH;

    // pin high
    SPARE2_ENABLE;
    SPARE2_HIGH;

    // pin high
    SPARE3_ENABLE;
    SPARE3_HIGH;

    // pin high
    SPARE4_ENABLE;
    SPARE4_HIGH;

    // pin high
    SPARE5_ENABLE;
    SPARE5_HIGH;

    // *************

    random32 ^= serial_number_crc32;                    // try to randomise the random number
//      random32 ^= serial_number_crc32 ^ CRC_IDR;      // try to randomise the random number

    trans_init();                                       // initialise the transparent communications module

//	api_init();  	  	                                // initialise the API communications module
    apiconfig_init();                                    // initialise the API communications module

    setup_TimerInt(1000);                               // setup a 1kHz timer interrupt

	#if defined(USE_WATCHDOG)
    	enableWatchdog();								// enable the watchdog
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

    #if defined(PIOS_COM_DEBUG)
        DEBUG_PRINTF("\r\n");
        DEBUG_PRINTF("PipXtreme v%u.%u rebooted\r\n", VERSION_MAJOR, VERSION_MINOR);
        DEBUG_PRINTF("\r\n");
        DEBUG_PRINTF("CPU flash size: %u\r\n", flash_size);
        DEBUG_PRINTF("CPU serial number: %s %08X\r\n", serial_number_str, serial_number_crc32);
//      DEBUG_PRINTF("Reset address: %08X\r\n", reset_addr);
    #endif

    // *************
    // initialise the saved settings module

    saved_settings_init();

    if (saved_settings.mode == 0xff  ||
    	saved_settings.mode == MODE_TX_BLANK_CARRIER_TEST ||
    	saved_settings.mode == MODE_TX_SPECTRUM_TEST ||
    	saved_settings.mode == MODE_SCAN_SPECTRUM)
    	saved_settings.mode = MODE_NORMAL;	// go back to NORMAL mode

    if (saved_settings.rts_time == 0xff || saved_settings.rts_time > 100)
    	saved_settings.rts_time = 10;	// default to 10ms

	#if !defined(PIOS_COM_DEBUG)
    	if (saved_settings.serial_baudrate != 0xffffffff)
    		PIOS_COM_ChangeBaud(PIOS_COM_SERIAL, saved_settings.serial_baudrate);
	#endif

    // *************
    // read the API mode pin

    if (!GPIO_IN(API_MODE_PIN))
        API_Mode = TRUE;

	#if defined(PIOS_COM_DEBUG)
    	if (!API_Mode)
    		DEBUG_PRINTF("TRANSPARENT mode\r\n");
    	else
    		DEBUG_PRINTF("API mode\r\n");
	#endif

    // *************
    // read the 434/868/915 jumper options

    if (!GPIO_IN(_868MHz_PIN) && !GPIO_IN(_915MHz_PIN)) saved_settings.frequency_band = FREQBAND_434MHz;    // 434MHz band
    else
    if (!GPIO_IN(_868MHz_PIN) &&  GPIO_IN(_915MHz_PIN)) saved_settings.frequency_band = FREQBAND_868MHz;    // 868MHz band
    else
    if ( GPIO_IN(_868MHz_PIN) && !GPIO_IN(_915MHz_PIN)) saved_settings.frequency_band = FREQBAND_915MHz;    // 915MHz band

    // set some defaults if they are not set
    switch (saved_settings.frequency_band)
    {
        case FREQBAND_434MHz:

            if (saved_settings.min_frequency_Hz == 0xffffffff)
            {
                saved_settings.frequency_Hz = 434000000;
                saved_settings.min_frequency_Hz = saved_settings.frequency_Hz - 2000000;
                saved_settings.max_frequency_Hz = saved_settings.frequency_Hz + 2000000;
            }
            if (saved_settings.max_rf_bandwidth == 0xffffffff)
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
            if (saved_settings.max_tx_power == 0xff)
            {
                saved_settings.max_tx_power = RFM22_tx_pwr_txpow_0;        // +1dBm ... 1.25mW
    //          saved_settings.max_tx_power = RFM22_tx_pwr_txpow_1;        // +2dBm ... 1.6mW
    //          saved_settings.max_tx_power = RFM22_tx_pwr_txpow_2;        // +5dBm ... 3.16mW
    //          saved_settings.max_tx_power = RFM22_tx_pwr_txpow_3;        // +8dBm ... 6.3mW
    //          saved_settings.max_tx_power = RFM22_tx_pwr_txpow_4;        // +11dBm .. 12.6mW
    //          saved_settings.max_tx_power = RFM22_tx_pwr_txpow_5;        // +14dBm .. 25mW
    //          saved_settings.max_tx_power = RFM22_tx_pwr_txpow_6;        // +17dBm .. 50mW
    //          saved_settings.max_tx_power = RFM22_tx_pwr_txpow_7;        // +20dBm .. 100mW
            }
            break;

        case FREQBAND_868MHz:
            if (saved_settings.min_frequency_Hz == 0xffffffff)
            {
                saved_settings.frequency_Hz = 868000000;
                saved_settings.min_frequency_Hz = saved_settings.frequency_Hz - 10000000;
                saved_settings.max_frequency_Hz = saved_settings.frequency_Hz + 10000000;
            }
            if (saved_settings.max_rf_bandwidth == 0xffffffff)
                saved_settings.max_rf_bandwidth = 128000;
            if (saved_settings.max_tx_power == 0xff)
                saved_settings.max_tx_power = RFM22_tx_pwr_txpow_0;    // +1dBm ... 1.25mW
            break;

        case FREQBAND_915MHz:
            if (saved_settings.min_frequency_Hz == 0xffffffff)
            {
                saved_settings.frequency_Hz = 915000000;
                saved_settings.min_frequency_Hz = saved_settings.frequency_Hz - 13000000;
                saved_settings.max_frequency_Hz = saved_settings.frequency_Hz + 13000000;
            }
            if (saved_settings.max_rf_bandwidth == 0xffffffff)
                saved_settings.max_rf_bandwidth = 128000;
            if (saved_settings.max_tx_power == 0xff)
                saved_settings.max_tx_power = RFM22_tx_pwr_txpow_0;    // +1dBm ... 1.25mW
            break;

        default:
            break;
    }

	#if defined(PIOS_COM_DEBUG)
    	switch (saved_settings.frequency_band)
    	{
    		case FREQBAND_UNKNOWN: DEBUG_PRINTF("UNKNOWN band\r\n"); break;
    		case FREQBAND_434MHz:  DEBUG_PRINTF("434MHz band\r\n"); break;
    		case FREQBAND_868MHz:  DEBUG_PRINTF("868MHz band\r\n"); break;
    		case FREQBAND_915MHz:  DEBUG_PRINTF("915MHz band\r\n"); break;
    	}
	#endif

    // *************

    processReset();		// Determine what caused the reset/reboot

    init_RF_module();	// initialise the RF module

    // *************

	#if defined(PIOS_COM_DEBUG)
    	DEBUG_PRINTF("\r\n");
    	DEBUG_PRINTF("RF datarate: %dbps\r\n", ph_getDatarate());
    	DEBUG_PRINTF("RF frequency: %dHz\r\n", rfm22_getNominalCarrierFrequency());
    	DEBUG_PRINTF("RF TX power: %d\r\n", ph_getTxPower());

    	DEBUG_PRINTF("\r\nUnit mode: ");
    	switch (saved_settings.mode)
    	{
    		case MODE_NORMAL:					DEBUG_PRINTF("NORMAL\r\n"); break;
    		case MODE_STREAM_TX:				DEBUG_PRINTF("STREAM-TX\r\n"); break;
    		case MODE_STREAM_RX:				DEBUG_PRINTF("STREAM-RX\r\n"); break;
    		case MODE_PPM_TX:					DEBUG_PRINTF("PPM-TX\r\n"); break;
    		case MODE_PPM_RX:					DEBUG_PRINTF("PPM-RX\r\n"); break;
    		case MODE_SCAN_SPECTRUM:			DEBUG_PRINTF("SCAN-SPECTRUM\r\n"); break;
    		case MODE_TX_BLANK_CARRIER_TEST:	DEBUG_PRINTF("TX-BLANK-CARRIER\r\n"); break;
    		case MODE_TX_SPECTRUM_TEST:			DEBUG_PRINTF("TX_SPECTRUM\r\n"); break;
    		default:							DEBUG_PRINTF("UNKNOWN [%u]\r\n", saved_settings.mode); break;
    	}
	#endif

   	switch (saved_settings.mode)
   	{
   		case MODE_NORMAL:
   			ph_init(serial_number_crc32);            // initialise the packet handler
   			break;

   		case MODE_STREAM_TX:
   		case MODE_STREAM_RX:
   			stream_init(serial_number_crc32);		// initialise the continuous packet stream module
   			break;

   		case MODE_PPM_TX:
   		case MODE_PPM_RX:
   			ppm_init(serial_number_crc32);			// initialise the ppm module
   			break;

   		case MODE_SCAN_SPECTRUM:
   		case MODE_TX_BLANK_CARRIER_TEST:
   		case MODE_TX_SPECTRUM_TEST:
   			break;
   	}

    // *************
    // Main executive loop

    saved_settings_save();

    booting = FALSE;

    for (;;)
    {
        random32 = updateCRC32(random32, PIOS_DELAY_GetuS() >> 8);
        random32 = updateCRC32(random32, PIOS_DELAY_GetuS());

        if (second_tick)
        {
            second_tick = FALSE;

            // *************************
            // display the up-time .. HH:MM:SS

//          #if defined(PIOS_COM_DEBUG)
//				uint32_t _uptime_ms = uptime_ms;
//				uint32_t _uptime_sec = _uptime_ms / 1000;
//				DEBUG_PRINTF("Uptime: %02d:%02d:%02d.%03d\r\n", _uptime_sec / 3600, (_uptime_sec / 60) % 60, _uptime_sec % 60, _uptime_ms % 1000);
//	    	#endif

            // *************************
            // process the Temperature

//	    	if (temp_adc >= 0)
//	    	{
//				int32_t degress_C = temp_adc;
//
//				#if defined(PIOS_COM_DEBUG)
//					DEBUG_PRINTF("TEMP: %d %d\r\n", temp_adc, degress_C);
//				#endif
//	    	}

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
        }

        rfm22_process();				// rf hardware layer processing

		uint8_t mode = saved_settings.mode;
//				modeTxBlankCarrierTest,	// blank carrier Tx mode (for calibrating the carrier frequency say)
//				modeTxSpectrumTest		// pseudo random Tx data mode (for checking the Tx carrier spectrum)

		if (mode == MODE_NORMAL)
	        ph_process();				// packet handler processing

		if (mode == MODE_STREAM_TX || mode == MODE_STREAM_RX)
			stream_process();			// continuous data stream processing

		if (mode == MODE_PPM_TX || mode == MODE_PPM_RX)
			ppm_process();				// ppm processing

        if (!API_Mode)
        	trans_process();			// tranparent local communication processing (serial port and usb port)
        else
//        	api_process();				// API local communication processing (serial port and usb port)
        	apiconfig_process();		// API local communication processing (serial port and usb port)




















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
