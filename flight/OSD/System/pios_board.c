/**
 ******************************************************************************
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board specific static initializers for hardware for the OPOSD board.
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
#include <fifo_buffer.h>
#include <openpilot.h>
#include <uavobjectsinit.h>
#include <hwsettings.h>
#include <manualcontrolsettings.h>
#include <gcsreceiver.h>

#include "board_hw_defs.c"


#define TxBufferSize3   33


/* Private macro -------------------------------------------------------------*/
#define countof(a)   (sizeof(a) / sizeof(*(a)))

/* Private variables ---------------------------------------------------------*/
//uint8_t TxBuffer2[TxBufferSize2];
uint8_t TxBuffer3[TxBufferSize3];
//uint8_t RxBuffer2[TxBufferSize2];
uint8_t RxBuffer3[TxBufferSize3];
//uint8_t UART1_REVDATA[380];


#if defined(PIOS_INCLUDE_ADC)
/*
* ADC system
*/

#include <pios_adc_priv.h>

void PIOS_ADC_handler(void)
{
	PIOS_ADC_DMA_Handler();
}

void DMA2_Stream4_IRQHandler() __attribute__ ((alias("PIOS_ADC_handler")));

const struct pios_adc_cfg pios_adc_cfg = {
	.dma = {
			.irq = {
					.flags = (DMA_FLAG_TCIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_HTIF4),
					.init = {
							.NVIC_IRQChannel = DMA2_Stream4_IRQn
					},
			},
			/* XXX there is secret knowledge here regarding the use of ADC1 by the pios_adc code */
			.rx = {
					.channel = DMA2_Stream4, // stream0 may be used by SPI1
					.init = {
							.DMA_Channel = DMA_Channel_0,
							.DMA_PeripheralBaseAddr = (uint32_t) & ADC1->DR
					},
			}
	},
	.half_flag = DMA_IT_HTIF4,
	.full_flag = DMA_IT_TCIF4,
};

struct pios_adc_dev pios_adc_devs[] = {
	{
		.cfg = &pios_adc_cfg,
		.callback_function = NULL,
		.data_queue = NULL
	},
};

uint8_t pios_adc_num_devices = NELEMENTS(pios_adc_devs);

#endif

/* Private define ------------------------------------------------------------*/
#define DAC_DHR12R2_ADDRESS    0x40007414
#define DAC_DHR8R1_ADDRESS     0x40007410


/* Private variables ---------------------------------------------------------*/
DAC_InitTypeDef  DAC_InitStructure;

const uint16_t Sine12bit[32] = {
                      2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095, 4056,
                      3939, 3750, 3495, 3185, 2831, 2447, 2047, 1647, 1263, 909,
                      599, 344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647};


const uint8_t Escalator8bit[6] = {0x0, 0x33, 0x66, 0x99, 0xCC, 0xFF};


/**
  * @brief  TIM6 Configuration
  * @note   TIM6 configuration is based on CPU @168MHz and APB1 @42MHz
  * @note   TIM6 Update event occurs each 37.5MHz/256 = 16.406 KHz
  * @param  None
  * @retval None
  */
void TIM6_Config(void)
{
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
  /* TIM6 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

  /* Time base configuration */
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = 27;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

  /* TIM6 TRGO selection */
  TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);

  /* TIM6 enable counter */
  TIM_Cmd(TIM6, ENABLE);
}


/**
  * @brief  DAC  Channel2 SineWave Configuration
  * @param  None
  * @retval None
  */
void DAC_Ch2_SineWaveConfig(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  /* DAC channel2 Configuration */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_2, &DAC_InitStructure);

  /* DMA1_Stream5 channel7 configuration **************************************/
  DMA_DeInit(DMA1_Stream6);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(uint32_t)&DAC->DHR12R2;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&Sine12bit;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = 32;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream6, &DMA_InitStructure);

  /* Enable DMA1_Stream5 */
  DMA_Cmd(DMA1_Stream6, ENABLE);

  /* Enable DAC Channel2 */
  DAC_Cmd(DAC_Channel_2, ENABLE);

  /* Enable DMA for DAC Channel2 */
  DAC_DMACmd(DAC_Channel_2, ENABLE);
}

void DAC_Ch1_SineWaveConfig(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  /* DAC channel2 Configuration */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);

  /* DMA1_Stream5 channel7 configuration **************************************/
  DMA_DeInit(DMA1_Stream5);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(uint32_t)&DAC->DHR12R1;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&Sine12bit;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = 32;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream5, &DMA_InitStructure);

  /* Enable DMA1_Stream5 */
  DMA_Cmd(DMA1_Stream5, ENABLE);

  /* Enable DAC Channel2 */
  DAC_Cmd(DAC_Channel_1, ENABLE);

  /* Enable DMA for DAC Channel2 */
  DAC_DMACmd(DAC_Channel_1, ENABLE);
}




static void Clock(uint32_t spektrum_id);

void initUSARTs(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_UART4);
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_UART4);
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	USART_InitStructure.USART_BaudRate = 57600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(UART4, &USART_InitStructure);
	USART_DMACmd(UART4, USART_DMAReq_Rx, ENABLE);
	DMA_Cmd(DMA1_Stream2, ENABLE);
	USART_Cmd(UART4, ENABLE);
}

#define DMA_Channel_USART4_RX    DMA1_Stream2
#define DMA_Channel_USART4_TX    DMA1_Stream4
#define DMA_FLAG_USART3_TC_RX    DMA1_FLAG_TC3
#define DMA_FLAG_USART3_TC_TX    DMA1_FLAG_TC2
#define USART3_DR_Base  0x40004804

void init_USART_dma()
{
	DMA_InitTypeDef				DMA_InitStructure;

	/*RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

   DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);
   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
   DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStructure.DMA_BufferSize = size;
   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
   DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
   DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
   DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)buff[0];
   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;

   DMA_DeInit(DMA_Channel_USART3_RX);
   DMA_Init(DMA_Channel_USART3_RX, &DMA_InitStructure);
   //DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);

   DMA_Cmd(DMA_Channel_USART3_RX, ENABLE);
   USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);*/

	/*DMA Channel2 USART3 TX*/
   DMA_DeInit(DMA1_Stream4);
   DMA_InitStructure.DMA_Channel = DMA_Channel_4;
   DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
   DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)TxBuffer3;
   DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;  /*read from  ram*/
   DMA_InitStructure.DMA_BufferSize = TxBufferSize3;	 /*if content is 0,stop TX*/
   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
   DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
   //DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
   DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
   //DMA_Init(DMA1_Channel2, &DMA_InitStructure);
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

   /*DMA Channel3 USART3 RX*/
   DMA_DeInit(DMA1_Stream2);
   DMA_InitStructure.DMA_Channel = DMA_Channel_4;
   DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
   DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)RxBuffer3;
   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
   DMA_InitStructure.DMA_BufferSize = sizeof(RxBuffer3);
   DMA_Init(DMA1_Stream2, &DMA_InitStructure);

}

#define PIOS_COM_TELEM_RF_RX_BUF_LEN 512
#define PIOS_COM_TELEM_RF_TX_BUF_LEN 512

#define PIOS_COM_GPS_RX_BUF_LEN 32

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 65
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 65

#define PIOS_COM_BRIDGE_RX_BUF_LEN 65
#define PIOS_COM_BRIDGE_TX_BUF_LEN 12

uint32_t pios_com_aux_id;
uint32_t pios_com_gps_id;
uint32_t pios_com_telem_usb_id;
uint32_t pios_com_telem_rf_id;
uint32_t pios_com_cotelem_id;


void PIOS_Board_Init(void) {

	// Delay system
	PIOS_DELAY_Init();

	PIOS_LED_Init(&pios_led_cfg);

	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();


	/* Initialize the alarms library */
	AlarmsInitialize();

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

	/* IAP System Setup */
	PIOS_IAP_Init();
	uint16_t boot_count = PIOS_IAP_ReadBootCount();
	if (boot_count < 3) {
		PIOS_IAP_WriteBootCount(++boot_count);
		AlarmsClear(SYSTEMALARMS_ALARM_BOOTFAULT);
	} else {
		/* Too many failed boot attempts, force hwsettings to defaults */
		HwSettingsSetDefaults(HwSettingsHandle(), 0);
		AlarmsSet(SYSTEMALARMS_ALARM_BOOTFAULT, SYSTEMALARMS_ALARM_CRITICAL);
	}

#if defined(PIOS_INCLUDE_RTC)
	/* Initialize the real-time clock and its associated tick */
	PIOS_RTC_Init(&pios_rtc_main_cfg);
	if (!PIOS_RTC_RegisterTickCallback(Clock, 0)) {
		PIOS_DEBUG_Assert(0);
	}
#endif

#if defined(PIOS_INCLUDE_USB)
	/* Initialize board specific USB data */
	PIOS_USB_BOARD_DATA_Init();

	/* Flags to determine if various USB interfaces are advertised */
	bool usb_hid_present = false;
	bool usb_cdc_present = false;

	uint8_t hwsettings_usb_devicetype;
	HwSettingsUSB_DeviceTypeGet(&hwsettings_usb_devicetype);

	switch (hwsettings_usb_devicetype) {
	case HWSETTINGS_USB_DEVICETYPE_HIDONLY:
		if (PIOS_USB_DESC_HID_ONLY_Init()) {
			PIOS_Assert(0);
		}
		usb_hid_present = true;
		break;
	case HWSETTINGS_USB_DEVICETYPE_HIDVCP:
		if (PIOS_USB_DESC_HID_CDC_Init()) {
			PIOS_Assert(0);
		}
		usb_hid_present = true;
		usb_cdc_present = true;
		break;
	case HWSETTINGS_USB_DEVICETYPE_VCPONLY:
		break;
	default:
		PIOS_Assert(0);
	}

	uint32_t pios_usb_id;
	PIOS_USB_Init(&pios_usb_id, &pios_usb_main_cfg);

#if defined(PIOS_INCLUDE_USB_CDC)

	uint8_t hwsettings_usb_vcpport;
	/* Configure the USB VCP port */
	HwSettingsUSB_VCPPortGet(&hwsettings_usb_vcpport);

	if (!usb_cdc_present) {
		/* Force VCP port function to disabled if we haven't advertised VCP in our USB descriptor */
		hwsettings_usb_vcpport = HWSETTINGS_USB_VCPPORT_DISABLED;
	}

	switch (hwsettings_usb_vcpport) {
	case HWSETTINGS_USB_VCPPORT_DISABLED:
		break;
	case HWSETTINGS_USB_VCPPORT_USBTELEMETRY:
#if defined(PIOS_INCLUDE_COM)
		{
			uint32_t pios_usb_cdc_id;
			if (PIOS_USB_CDC_Init(&pios_usb_cdc_id, &pios_usb_cdc_cfg, pios_usb_id)) {
				PIOS_Assert(0);
			}
			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_RX_BUF_LEN);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_telem_usb_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
						rx_buffer, PIOS_COM_TELEM_USB_RX_BUF_LEN,
						tx_buffer, PIOS_COM_TELEM_USB_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_COM */
		break;
	case HWSETTINGS_USB_VCPPORT_COMBRIDGE:
#if defined(PIOS_INCLUDE_COM)
		{
			uint32_t pios_usb_cdc_id;
			if (PIOS_USB_CDC_Init(&pios_usb_cdc_id, &pios_usb_cdc_cfg, pios_usb_id)) {
				PIOS_Assert(0);
			}
			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_BRIDGE_RX_BUF_LEN);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_BRIDGE_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_vcp_id, &pios_usb_cdc_com_driver, pios_usb_cdc_id,
						rx_buffer, PIOS_COM_BRIDGE_RX_BUF_LEN,
						tx_buffer, PIOS_COM_BRIDGE_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_COM */
		break;
	}
#endif	/* PIOS_INCLUDE_USB_CDC */

#if defined(PIOS_INCLUDE_USB_HID)
	/* Configure the usb HID port */
	uint8_t hwsettings_usb_hidport;
	HwSettingsUSB_HIDPortGet(&hwsettings_usb_hidport);

	if (!usb_hid_present) {
		/* Force HID port function to disabled if we haven't advertised HID in our USB descriptor */
		hwsettings_usb_hidport = HWSETTINGS_USB_HIDPORT_DISABLED;
	}

	switch (hwsettings_usb_hidport) {
	case HWSETTINGS_USB_HIDPORT_DISABLED:
		break;
	case HWSETTINGS_USB_HIDPORT_USBTELEMETRY:
#if defined(PIOS_INCLUDE_COM)
		{
			uint32_t pios_usb_hid_id;
			if (PIOS_USB_HID_Init(&pios_usb_hid_id, &pios_usb_hid_cfg, pios_usb_id)) {
				PIOS_Assert(0);
			}
			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_RX_BUF_LEN);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_telem_usb_id, &pios_usb_hid_com_driver, pios_usb_hid_id,
						rx_buffer, PIOS_COM_TELEM_USB_RX_BUF_LEN,
						tx_buffer, PIOS_COM_TELEM_USB_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_COM */
		break;
	}

#endif	/* PIOS_INCLUDE_USB_HID */

	if (usb_hid_present || usb_cdc_present) {
		PIOS_USBHOOK_Activate();
	}
#endif	/* PIOS_INCLUDE_USB */

#if defined(PIOS_INCLUDE_COM)
#if defined(PIOS_INCLUDE_GPS)

	uint32_t pios_usart_gps_id;
	if (PIOS_USART_Init(&pios_usart_gps_id, &pios_usart_gps_cfg)) {
		PIOS_Assert(0);
	}

	uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_GPS_RX_BUF_LEN);
	PIOS_Assert(rx_buffer);
	if (PIOS_COM_Init(&pios_com_gps_id, &pios_usart_com_driver, pios_usart_gps_id,
					  rx_buffer, PIOS_COM_GPS_RX_BUF_LEN,
					  NULL, 0)) {
		PIOS_Assert(0);
	}

#endif	/* PIOS_INCLUDE_GPS */

#if defined(PIOS_INCLUDE_COM_AUX)
	{
		uint32_t pios_usart_aux_id;

		if (PIOS_USART_Init(&pios_usart_aux_id, &pios_usart_aux_cfg)) {
			PIOS_DEBUG_Assert(0);
		}

		const uint32_t BUF_SIZE = 512;
		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(BUF_SIZE);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(BUF_SIZE);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);

		if (PIOS_COM_Init(&pios_com_aux_id, &pios_usart_com_driver, pios_usart_aux_id,
						  rx_buffer, BUF_SIZE,
						  tx_buffer, BUF_SIZE)) {
			PIOS_DEBUG_Assert(0);
		}
	}
#else
	pios_com_aux_id = 0;
#endif  /* PIOS_INCLUDE_COM_AUX */

#if defined(PIOS_INCLUDE_COM_TELEM)
	{ /* Eventually add switch for this port function */
		uint32_t pios_usart_telem_rf_id;
		if (PIOS_USART_Init(&pios_usart_telem_rf_id, &pios_usart_telem_main_cfg)) {
			PIOS_Assert(0);
		}

		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_telem_rf_id, &pios_usart_com_driver, pios_usart_telem_rf_id,
						  rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
						  tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}
#else
	pios_com_telem_rf_id = 0;
#endif	/* PIOS_INCLUDE_COM_TELEM */

#if defined(PIOS_INCLUDE_COM_COTELEM)
	{ /* Eventually add switch for this port function */
		uint32_t pios_usart_cotelem_id;
		if (PIOS_USART_Init(&pios_usart_cotelem_id, &pios_usart_cotelem_main_cfg)) {
			PIOS_Assert(0);
		}

		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_cotelem_id, &pios_usart_com_driver, pios_usart_cotelem_id,
						  rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
						  tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}
#else
	pios_com_cotelem_id = 0;
#endif	/* PIOS_INCLUDE_COM_TELEM */
#endif	/* PIOS_INCLUDE_COM */

/*
	uint32_t pios_usart_hkosd_id;
	if (PIOS_USART_Init(&pios_usart_hkosd_id, &pios_usart_serial_cfg)) {
		PIOS_Assert(0);
	}
	uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_HKOSD_RX_BUF_LEN);
	PIOS_Assert(rx_buffer);

	uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_HKOSD_TX_BUF_LEN);
	PIOS_Assert(tx_buffer);
	if (PIOS_COM_Init(&pios_com_hkosd_id, &pios_usart_com_driver, pios_usart_hkosd_id,
				rx_buffer, PIOS_COM_HKOSD_RX_BUF_LEN,
				tx_buffer, PIOS_COM_HKOSD_TX_BUF_LEN)) {
		PIOS_Assert(0);
	}*/

/*
	uint32_t pios_usart_serial_id;
	if (PIOS_USART_Init(&pios_usart_serial_id, &pios_usart_serial_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
	uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
	uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
	PIOS_Assert(rx_buffer);
	PIOS_Assert(tx_buffer);

	if (PIOS_COM_Init(&pios_com_serial_id, &pios_usart_com_driver, pios_usart_serial_id,
			  rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
			  tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
		PIOS_Assert(0);
	}*/

#if 1
/* Preconfiguration before using DAC----------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;

/* DAC Periph clock enable */
 RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

 /* DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) configuration */
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
 GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
 GPIO_Init(GPIOA, &GPIO_InitStructure);

 /* TIM6 Configuration ------------------------------------------------------*/
 TIM6_Config();

 DAC_DeInit();
 DAC_Ch1_SineWaveConfig();
 //DAC_Ch2_SineWaveConfig();
#endif
	// ADC system
	PIOS_ADC_Init();

	// SPI link to master
	/*if (PIOS_SPI_Init(&pios_spi_port_id, &pios_spi_port_cfg)) {
		PIOS_DEBUG_Assert(0);
	}*/

	/*init_USART_dma();
	initUSARTs();
	extern t_fifo_buffer rx;
	fifoBuf_init(&rx,RxBuffer3,sizeof(RxBuffer3));*/

	//PIOS_Video_Init(&pios_video_cfg);

	//uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_HKOSD_RX_BUF_LEN);

	//uint8_t test[16];


}

uint16_t supv_timer=0;

static void Clock(uint32_t spektrum_id) {
	/* 125hz */
	++supv_timer;
	if(supv_timer >= 625) {
		supv_timer = 0;
		time.sec++;
	}
	if (time.sec >= 60) {
		time.sec = 0;
		time.min++;
	}
	if (time.min >= 60) {
		time.min = 0;
		time.hour++;
	}
	if (time.hour >= 99) {
		time.hour = 0;
	}
}
