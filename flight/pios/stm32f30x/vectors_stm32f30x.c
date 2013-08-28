/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 *
 * @file       vector_stm32f4xx.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      C based vectors for F4
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

/** interrupt handler */
typedef const void	(vector)(void);

/** default interrupt handler */
static void
default_io_handler(void)
{
	for (;;) ;
}

/** prototypes an interrupt handler */
#define HANDLER(_name)	extern vector _name __attribute__((weak, alias("default_io_handler")))

HANDLER(WWDG_IRQHandler);                 // Window WatchDog
HANDLER(PVD_IRQHandler);                  // PVD through EXTI Line detection
HANDLER(TAMP_STAMP_IRQHandler);           // Tamper and TimeStamps through the EXTI line
HANDLER(RTC_WKUP_IRQHandler);             // RTC Wakeup through the EXTI line
HANDLER(FLASH_IRQHandler);                // FLASH
HANDLER(RCC_IRQHandler);                  // RCC
HANDLER(EXTI0_IRQHandler);                // EXTI Line0
HANDLER(EXTI1_IRQHandler);                // EXTI Line1
HANDLER(EXTI2_TS_IRQn);             // EXTI Line2 and Touch Sense Interrupt
HANDLER(EXTI3_IRQHandler);                // EXTI Line3
HANDLER(EXTI4_IRQHandler);                // EXTI Line4
HANDLER(DMA1_Channel1_IRQHandler);        // DMA1 Channel 1
HANDLER(DMA1_Channel2_IRQHandler);        // DMA1 Channel 2
HANDLER(DMA1_Channel3_IRQHandler);        // DMA1 Channel 3
HANDLER(DMA1_Channel4_IRQHandler);        // DMA1 Channel 4
HANDLER(DMA1_Channel5_IRQHandler);        // DMA1 Channel 5
HANDLER(DMA1_Channel6_IRQHandler);        // DMA1 Channel 6
HANDLER(DMA1_Channel7_IRQHandler);        // DMA1 Channel 7
HANDLER(ADC1_2_IRQHandler);               // ADC1 and ADC2
HANDLER(USB_HP_CAN1_TX_IRQHandler);       // USB Device High Priority or CAN1 TX
HANDLER(USB_LP_CAN1_RX0_IRQHandler);      // USB Device Low Priority or CAN1 RX0
HANDLER(CAN1_RX1_IRQHandler);             // CAN1 RX1
HANDLER(CAN1_SCE_IRQHandler);             // CAN1 SCE
HANDLER(EXTI9_5_IRQHandler);              // External Line[9:5]s
HANDLER(TIM1_BRK_TIM15_IRQHandler);       // TIM1 Break and TIM15
HANDLER(TIM1_UP_TIM16_IRQHandler);        // TIM1 Update and TIM16
HANDLER(TIM1_TRG_COM_TIM17_IRQHandler);   // TIM1 Trigger and Commutation and TIM17
HANDLER(TIM1_CC_IRQHandler);              // TIM1 Capture Compare
HANDLER(TIM2_IRQHandler);                 // TIM2
HANDLER(TIM3_IRQHandler);                 // TIM3
HANDLER(TIM4_IRQHandler);                 // TIM4
HANDLER(I2C1_EV_IRQHandler);              // I2C1 Event
HANDLER(I2C1_ER_IRQHandler);              // I2C1 Error
HANDLER(I2C2_EV_IRQHandler);              // I2C2 Event
HANDLER(I2C2_ER_IRQHandler);              // I2C2 Error
HANDLER(SPI1_IRQHandler);                 // SPI1
HANDLER(SPI2_IRQHandler);                 // SPI2
HANDLER(USART1_IRQHandler);               // USART1
HANDLER(USART2_IRQHandler);               // USART2
HANDLER(USART3_IRQHandler);               // USART3
HANDLER(EXTI15_10_IRQHandler);            // External Line[15:10]s
HANDLER(RTC_Alarm_IRQHandler);            // RTC Alarm (A and B) through EXTI Line
HANDLER(USB_WKUP_IRQHandler);             // USB FS Wakeup through EXTI line
HANDLER(TIM8_BRK_IRQHandler);             // TIM8 Break
HANDLER(TIM8_UP_IRQHandler);              // TIM8 Update
HANDLER(TIM8_TRG_COM_IRQHandler);         // TIM8 Trigger and Commutation
HANDLER(TIM8_CC_IRQHandler);              // TIM8 Capture Compare
HANDLER(ADC3_IRQHandler);                 // ADC3
HANDLER(SPI3_IRQHandler);                 // SPI3
HANDLER(UART4_IRQHandler);                // UART4
HANDLER(UART5_IRQHandler);                // UART5
HANDLER(TIM6_DAC_IRQHandler);             // TIM6 and DAC1&2 underrun errors
HANDLER(TIM7_IRQHandler);                 // TIM7
HANDLER(DMA2_Channel1_IRQHandler);        // DMA2 Channel 1
HANDLER(DMA2_Channel2_IRQHandler);        // DMA2 Channel 2
HANDLER(DMA2_Channel3_IRQHandler);        // DMA2 Channel 3
HANDLER(DMA2_Channel4_IRQHandler);        // DMA2 Channel 4
HANDLER(DMA2_Channel5_IRQHandler);        // DMA2 Channel 5
HANDLER(ADC4_IRQHandler);                 // ADC4
HANDLER(COMP1_2_3_IRQHandler);            // COMP1, COMP2 and COMP3
HANDLER(COMP4_5_6_IRQHandler);            // COMP4, COMP5 and COMP6
HANDLER(COMP7_IRQHandler);                // COMP7
HANDLER(USB_HP_IRQHandler);               // USB High Priority remap
HANDLER(USB_LP_IRQHandler);               // USB Low Priority remap
HANDLER(USB_WKUP_RMP_IRQHandler);         // USB Wakup remap
HANDLER(FPU_IRQHandler);                  // FPU

/** stm32f30x interrupt vector table */
vector *io_vectors[] __attribute__((section(".io_vectors"))) = {
	WWDG_IRQHandler,                 // Window WatchDog
	PVD_IRQHandler,                  // PVD through EXTI Line detection
	TAMP_STAMP_IRQHandler,           // Tamper and TimeStamps through the EXTI line
	RTC_WKUP_IRQHandler,             // RTC Wakeup through the EXTI line
	FLASH_IRQHandler,                // FLASH
	RCC_IRQHandler,                  // RCC
	EXTI0_IRQHandler,                // EXTI Line0
	EXTI1_IRQHandler,                // EXTI Line1
	EXTI2_TS_IRQn,             // EXTI Line2 and Touch Sense Interrupt
	EXTI3_IRQHandler,                // EXTI Line3
	EXTI4_IRQHandler,                // EXTI Line4
	DMA1_Channel1_IRQHandler,        // DMA1 Channel 1
	DMA1_Channel2_IRQHandler,        // DMA1 Channel 2
	DMA1_Channel3_IRQHandler,        // DMA1 Channel 3
	DMA1_Channel4_IRQHandler,        // DMA1 Channel 4
	DMA1_Channel5_IRQHandler,        // DMA1 Channel 5
	DMA1_Channel6_IRQHandler,        // DMA1 Channel 6
	DMA1_Channel7_IRQHandler,        // DMA1 Channel 7
	ADC1_2_IRQHandler,               // ADC1 and ADC2
	USB_HP_CAN1_TX_IRQHandler,       // USB Device High Priority or CAN1 TX
	USB_LP_CAN1_RX0_IRQHandler,      // USB Device Low Priority or CAN1 RX0
	CAN1_RX1_IRQHandler,             // CAN1 RX1
	CAN1_SCE_IRQHandler,             // CAN1 SCE
	EXTI9_5_IRQHandler,              // External Line[9:5]s
	TIM1_BRK_TIM15_IRQHandler,       // TIM1 Break and TIM15
	TIM1_UP_TIM16_IRQHandler,        // TIM1 Update and TIM16
	TIM1_TRG_COM_TIM17_IRQHandler,   // TIM1 Trigger and Commutation and TIM17
	TIM1_CC_IRQHandler,              // TIM1 Capture Compare
	TIM2_IRQHandler,                 // TIM2
	TIM3_IRQHandler,                 // TIM3
	TIM4_IRQHandler,                 // TIM4
	I2C1_EV_IRQHandler,              // I2C1 Event
	I2C1_ER_IRQHandler,              // I2C1 Error
	I2C2_EV_IRQHandler,              // I2C2 Event
	I2C2_ER_IRQHandler,              // I2C2 Error
	SPI1_IRQHandler,                 // SPI1
	SPI2_IRQHandler,                 // SPI2
	USART1_IRQHandler,               // USART1
	USART2_IRQHandler,               // USART2
	USART3_IRQHandler,               // USART3
	EXTI15_10_IRQHandler,            // External Line[15:10]s
	RTC_Alarm_IRQHandler,            // RTC Alarm (A and B) through EXTI Line
	USB_WKUP_IRQHandler,             // USB FS Wakeup through EXTI line
	TIM8_BRK_IRQHandler,             // TIM8 Break
	TIM8_UP_IRQHandler,              // TIM8 Update
	TIM8_TRG_COM_IRQHandler,         // TIM8 Trigger and Commutation
	TIM8_CC_IRQHandler,              // TIM8 Capture Compare
	ADC3_IRQHandler,                 // ADC3
	SPI3_IRQHandler,                 // SPI3
	UART4_IRQHandler,                // UART4
	UART5_IRQHandler,                // UART5
	TIM6_DAC_IRQHandler,             // TIM6 and DAC1&2 underrun errors
	TIM7_IRQHandler,                 // TIM7
	DMA2_Channel1_IRQHandler,        // DMA2 Channel 1
	DMA2_Channel2_IRQHandler,        // DMA2 Channel 2
	DMA2_Channel3_IRQHandler,        // DMA2 Channel 3
	DMA2_Channel4_IRQHandler,        // DMA2 Channel 4
	DMA2_Channel5_IRQHandler,        // DMA2 Channel 5
	ADC4_IRQHandler,                 // ADC4
	COMP1_2_3_IRQHandler,            // COMP1, COMP2 and COMP3
	COMP4_5_6_IRQHandler,            // COMP4, COMP5 and COMP6
	COMP7_IRQHandler,                // COMP7
	USB_HP_IRQHandler,               // USB High Priority remap
	USB_LP_IRQHandler,               // USB Low Priority remap
	USB_WKUP_RMP_IRQHandler,         // USB Wakup remap
	FPU_IRQHandler,                  // FPU
};

/**
 * @}
 */
