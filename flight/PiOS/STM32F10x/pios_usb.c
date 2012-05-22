/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USB USB Setup Functions
 * @brief PIOS USB device implementation
 * @{
 *
 * @file       pios_usb.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      USB device functions (STM32 dependent code)
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

/* Project Includes */
#include "pios.h"
#include "usb_lib.h"
#include "pios_usb_board_data.h"
#include "stm32f10x.h"

#include "pios_usb.h"
#include "pios_usb_priv.h"

#if defined(PIOS_INCLUDE_USB_HID)

/* Rx/Tx status */
static bool transfer_possible = false;

enum pios_usb_dev_magic {
	PIOS_USB_DEV_MAGIC = 0x17365904,
};

struct pios_usb_dev {
	enum pios_usb_dev_magic     magic;
	const struct pios_usb_cfg * cfg;
};

/**
 * @brief Validate the usb device structure
 * @returns 0 if valid device or -1 otherwise
 */
static int32_t PIOS_USB_validate(struct pios_usb_dev * usb_dev)
{
	if(usb_dev == NULL)
		return -1;

	if (usb_dev->magic != PIOS_USB_DEV_MAGIC)
		return -1;

	return 0;
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_usb_dev * PIOS_USB_alloc(void)
{
	struct pios_usb_dev * usb_dev;

	usb_dev = (struct pios_usb_dev *)pvPortMalloc(sizeof(*usb_dev));
	if (!usb_dev) return(NULL);

	usb_dev->magic = PIOS_USB_DEV_MAGIC;
	return(usb_dev);
}
#else
static struct pios_usb_dev pios_usb_devs[PIOS_USB_MAX_DEVS];
static uint8_t pios_usb_num_devs;
static struct pios_usb_dev * PIOS_USB_alloc(void)
{
	struct pios_usb_dev * usb_dev;

	if (pios_usb_num_devs >= PIOS_USB_MAX_DEVS) {
		return (NULL);
	}

	usb_dev = &pios_usb_devs[pios_usb_num_devs++];
	usb_dev->magic = PIOS_USB_DEV_MAGIC;

	return (usb_dev);
}
#endif


/**
 * Initialises USB COM layer
 * \return < 0 if initialisation failed
 * \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
 */
static uint32_t pios_usb_com_id;
int32_t PIOS_USB_Init(uint32_t * usb_id, const struct pios_usb_cfg * cfg)
{
	PIOS_Assert(usb_id);
	PIOS_Assert(cfg);

	struct pios_usb_dev * usb_dev;

	usb_dev = (struct pios_usb_dev *) PIOS_USB_alloc();
	if (!usb_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	usb_dev->cfg = cfg;

	PIOS_USB_Reenumerate();

	/*
	 * This is a horrible hack to make this available to
	 * the interrupt callbacks.  This should go away ASAP.
	 */
	pios_usb_com_id = (uint32_t) usb_dev;

	/* Enable the USB Interrupts */
	NVIC_Init(&usb_dev->cfg->irq.init);

	/* Select USBCLK source */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
	/* Enable the USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

	USB_Init();
	USB_SIL_Init();

	*usb_id = (uint32_t) usb_dev;

	return 0;		/* No error */

out_fail:
	return(-1);
}

/**
 * This function is called by the USB driver on cable connection/disconnection
 * \param[in] connected connection status (1 if connected)
 * \return < 0 on errors
 * \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
 */
int32_t PIOS_USB_ChangeConnectionState(bool Connected)
{
	// In all cases: re-initialise USB HID driver
	if (Connected) {
		transfer_possible = true;

		//TODO: Check SetEPRxValid(ENDP1);

#if defined(USB_LED_ON)
		USB_LED_ON;	// turn the USB led on
#endif
	} else {
		// Cable disconnected: disable transfers
		transfer_possible = false;

#if defined(USB_LED_OFF)
		USB_LED_OFF;	// turn the USB led off
#endif
	}

	return 0;
}

int32_t PIOS_USB_Reenumerate()
{
	/* Force USB reset and power-down (this will also release the USB pins for direct GPIO control) */
	_SetCNTR(CNTR_FRES | CNTR_PDWN);

	/* Using a "dirty" method to force a re-enumeration: */
	/* Force DPM (Pin PA12) low for ca. 10 mS before USB Tranceiver will be enabled */
	/* This overrules the external Pull-Up at PA12, and at least Windows & MacOS will enumerate again */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	PIOS_DELAY_WaitmS(50);

	/* Release power-down, still hold reset */
	_SetCNTR(CNTR_PDWN);
	PIOS_DELAY_WaituS(5);

	/* CNTR_FRES = 0 */
	_SetCNTR(0);

	/* Clear pending interrupts */
	_SetISTR(0);

	/* Configure USB clock */
	/* USBCLK = PLLCLK / 1.5 */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
	/* Enable USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

	return 0;
}

bool PIOS_USB_CableConnected(uint8_t id)
{
	struct pios_usb_dev * usb_dev = (struct pios_usb_dev *) pios_usb_com_id;

	if (PIOS_USB_validate(usb_dev) != 0)
		return false;

	return usb_dev->cfg->vsense.gpio->IDR & usb_dev->cfg->vsense.init.GPIO_Pin;
}

/**
 * This function returns the connection status of the USB HID interface
 * \return 1: interface available
 * \return 0: interface not available
 * \note Applications shouldn't call this function directly, instead please use \ref PIOS_COM layer functions
 */
bool PIOS_USB_CheckAvailable(uint8_t id)
{
	struct pios_usb_dev * usb_dev = (struct pios_usb_dev *) pios_usb_com_id;

	if (PIOS_USB_validate(usb_dev) != 0)
		return false;

	return PIOS_USB_CableConnected(id) && transfer_possible;
}

#endif

/**
 * @}
 * @}
 */
