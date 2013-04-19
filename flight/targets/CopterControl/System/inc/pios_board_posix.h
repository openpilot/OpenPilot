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


#ifndef PIOS_BOARD_H
#define PIOS_BOARD_H




//------------------------
// PIOS_LED
//------------------------
//#define PIOS_LED_LED1_GPIO_PORT			GPIOC
//#define PIOS_LED_LED1_GPIO_PIN			GPIO_Pin_12
//#define PIOS_LED_LED1_GPIO_CLK			RCC_APB2Periph_GPIOC
//#define PIOS_LED_LED2_GPIO_PORT			GPIOC
//#define PIOS_LED_LED2_GPIO_PIN			GPIO_Pin_13
//#define PIOS_LED_LED2_GPIO_CLK			RCC_APB2Periph_GPIOC
#define PIOS_LED_NUM				2
//#define PIOS_LED_PORTS				{ PIOS_LED_LED1_GPIO_PORT, PIOS_LED_LED2_GPIO_PORT }
//#define PIOS_LED_PINS				{ PIOS_LED_LED1_GPIO_PIN, PIOS_LED_LED2_GPIO_PIN }
//#define PIOS_LED_CLKS				{ PIOS_LED_LED1_GPIO_CLK, PIOS_LED_LED2_GPIO_CLK }


//-------------------------
// COM
//
// See also pios_board_posix.c
//-------------------------
//#define PIOS_USART_TX_BUFFER_SIZE		256
#define PIOS_COM_BUFFER_SIZE 1024
#define PIOS_UDP_RX_BUFFER_SIZE		PIOS_COM_BUFFER_SIZE

#define PIOS_COM_TELEM_RF                       0
#define PIOS_COM_GPS                            1
#define PIOS_COM_TELEM_USB                      2

#ifdef PIOS_ENABLE_AUX_UART
#define PIOS_COM_AUX                            3
#define PIOS_COM_DEBUG                          PIOS_COM_AUX
#endif

/**
 * glue macros for file IO
 * STM32 uses DOSFS for file IO
 */
#define PIOS_FOPEN_READ(filename,file)	(file=fopen((char*)filename,"r"))==NULL

#define PIOS_FOPEN_WRITE(filename,file)	(file=fopen((char*)filename,"w"))==NULL

#define PIOS_FREAD(file,bufferadr,length,resultadr)	(*resultadr=fread((uint8_t*)bufferadr,1,length,*file)) != length

#define PIOS_FWRITE(file,bufferadr,length,resultadr)	*resultadr=fwrite((uint8_t*)bufferadr,1,length,*file)



#define PIOS_FCLOSE(file)		fclose(file)

#define PIOS_FUNLINK(file)		unlink((char*)filename)

#endif /* PIOS_BOARD_H */
