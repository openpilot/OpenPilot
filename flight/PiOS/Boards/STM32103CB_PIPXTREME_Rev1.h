 /**
 ******************************************************************************
 *
 * @file       pios_board.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board hardware for the OpenPilot Version 1.1 hardware.
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

#ifndef STM32103CB_PIPXTREME_H_
#define STM32103CB_PIPXTREME_H_

#define ADD_ONE_ADC

//------------------------
// Timers and Channels Used
//------------------------
/*
Timer | Channel 1 | Channel 2 | Channel 3 | Channel 4
------+-----------+-----------+-----------+----------
TIM1  |  Servo 4  |           |           |
TIM2  |  RC In 5  |  RC In 6  |  Servo 6  |
TIM3  |  Servo 5  |  RC In 2  |  RC In 3  |  RC In 4
TIM4  |  RC In 1  |  Servo 3  |  Servo 2  |  Servo 1
------+-----------+-----------+-----------+----------
*/

//------------------------
// DMA Channels Used
//------------------------
/* Channel 1  - 				*/
/* Channel 2  - 				*/
/* Channel 3  - 				*/
/* Channel 4  - 				*/
/* Channel 5  - 				*/
/* Channel 6  - 				*/
/* Channel 7  - 				*/
/* Channel 8  - 				*/
/* Channel 9  - 				*/
/* Channel 10 - 				*/
/* Channel 11 - 				*/
/* Channel 12 - 				*/


//------------------------
// BOOTLOADER_SETTINGS
//------------------------
#define BOARD_READABLE	TRUE
#define BOARD_WRITABLE	TRUE
#define MAX_DEL_RETRYS	3


//------------------------
// WATCHDOG_SETTINGS
//------------------------
#define PIOS_WATCHDOG_TIMEOUT    250
#define PIOS_WDG_REGISTER        BKP_DR4
#define PIOS_WDG_ACTUATOR        0x0001
#define PIOS_WDG_STABILIZATION   0x0002
#define PIOS_WDG_ATTITUDE        0x0004
#define PIOS_WDG_MANUAL          0x0008

//------------------------
// TELEMETRY
//------------------------
#define TELEM_QUEUE_SIZE         20

//------------------------
// PIOS_LED
//------------------------
#define PIOS_LED_USB	0
#define PIOS_LED_LINK	1
#define PIOS_LED_RX	2
#define PIOS_LED_TX	3

#define PIOS_LED_HEARTBEAT PIOS_LED_USB
#define PIOS_LED_ALARM PIOS_LED_TX

#define USB_LED_ON					PIOS_LED_On(PIOS_LED_USB)
#define USB_LED_OFF					PIOS_LED_Off(PIOS_LED_USB)
#define USB_LED_TOGGLE					PIOS_LED_Toggle(PIOS_LED_USB)

#define LINK_LED_ON					PIOS_LED_On(PIOS_LED_LINK)
#define LINK_LED_OFF					PIOS_LED_Off(PIOS_LED_LINK)
#define LINK_LED_TOGGLE					PIOS_LED_Toggle(PIOS_LED_LINK)

#define RX_LED_ON					PIOS_LED_On(PIOS_LED_RX)
#define RX_LED_OFF					PIOS_LED_Off(PIOS_LED_RX)
#define RX_LED_TOGGLE					PIOS_LED_Toggle(PIOS_LED_RX)

#define TX_LED_ON					PIOS_LED_On(PIOS_LED_TX)
#define TX_LED_OFF					PIOS_LED_Off(PIOS_LED_TX)
#define TX_LED_TOGGLE					PIOS_LED_Toggle(PIOS_LED_TX)

//-------------------------
// System Settings
//-------------------------
#define PIOS_MASTER_CLOCK			72000000
#define PIOS_PERIPHERAL_CLOCK			(PIOS_MASTER_CLOCK / 2)

//-------------------------
// Interrupt Priorities
//-------------------------
#define PIOS_IRQ_PRIO_LOW			12		// lower than RTOS
#define PIOS_IRQ_PRIO_MID			8		// higher than RTOS
#define PIOS_IRQ_PRIO_HIGH			5		// for SPI, ADC, I2C etc...
#define PIOS_IRQ_PRIO_HIGHEST			4 		// for USART etc...

//------------------------
// PIOS_I2C
// See also pios_board.c
//------------------------
#define PIOS_I2C_MAX_DEVS			1
extern uint32_t pios_i2c_flexi_adapter_id;
#define PIOS_I2C_MAIN_ADAPTER			(pios_i2c_flexi_adapter_id)

//-------------------------
// SPI
//
// See also pios_board.c
//-------------------------
#define PIOS_SPI_MAX_DEVS			2

//-------------------------
// PIOS_USART
//-------------------------
#define PIOS_USART_MAX_DEVS			3

//-------------------------
// PIOS_COM
//
// See also pios_board.c
//-------------------------
#define PIOS_COM_MAX_DEVS			5

extern uint32_t pios_com_telem_usb_id;
extern uint32_t pios_com_vcp_usb_id;
extern uint32_t pios_com_usart1_id;
extern uint32_t pios_com_usart3_id;
extern uint32_t pios_com_rfm22b_id;
#define PIOS_COM_TELEM_SERIAL           (pios_com_usart1_id)
#define PIOS_COM_FLEXI                  (pios_com_usart3_id)
#define PIOS_COM_TELEM_USB              (pios_com_telem_usb_id)
#define PIOS_COM_VCP_USB                (pios_com_vcp_usb_id)
#define PIOS_COM_RFM22B_RF              (pios_com_rfm22b_id)
/*
#define PIOS_COM_DEBUG									PIOS_COM_TELEM_SERIAL
#define PIOS_COM_RADIO_TEMP							PIOS_COM_FLEXI
#define PIOS_COM_BRIDGE_COM							PIOS_COM_TELEM_USB
*/
#define PIOS_COM_DEBUG									PIOS_COM_TELEM_SERIAL
#define PIOS_COM_RADIO_TEMP							PIOS_COM_FLEXI
#define PIOS_COM_BRIDGE_COM							PIOS_COM_TELEM_SERIAL
#define PIOS_COM_BRIDGE_RADIO						PIOS_COM_RFM22B_RF

//------------------------
// PIOS_RCVR
// See also pios_board.c
//------------------------
#define PIOS_RCVR_MAX_DEVS                      3
#define PIOS_RCVR_MAX_CHANNELS			12

//-------------------------
// Receiver PPM input
//-------------------------
#define PIOS_PPM_MAX_DEVS			1
#define PIOS_PPM_NUM_INPUTS                     12

//-------------------------
// Receiver PWM input
//-------------------------
#define PIOS_PWM_MAX_DEVS			1
#define PIOS_PWM_NUM_INPUTS                     1

//-------------------------
// Servo outputs
//-------------------------
#define PIOS_SERVO_UPDATE_HZ                    50
#define PIOS_SERVOS_INITIAL_POSITION            0 /* dont want to start motors, have no pulse till settings loaded */

//--------------------------
// Timer controller settings
//--------------------------
#define PIOS_TIM_MAX_DEVS			3

//-------------------------
// GPIO
//-------------------------
#define PIOS_GPIO_PORTS				{ }
#define PIOS_GPIO_PINS				{ }
#define PIOS_GPIO_CLKS				{ }
#define PIOS_GPIO_NUM				0

//-------------------------
// USB
//-------------------------
#define PIOS_USB_HID_MAX_DEVS                   1
#define PIOS_USB_ENABLED                        1
#define PIOS_USB_HID_MAX_DEVS                   1
#define PIOS_USB_MAX_DEVS                       1
#define PIOS_USB_DETECT_GPIO_PORT               GPIOC
#define PIOS_USB_DETECT_GPIO_PIN                GPIO_Pin_15
#define PIOS_USB_DETECT_EXTI_LINE               EXTI_Line15

#endif /* STM32103CB_PIPXTREME_H_ */
