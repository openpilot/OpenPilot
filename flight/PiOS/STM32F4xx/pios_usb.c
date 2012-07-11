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
#include "usb_core.h"
#include "pios_usb_board_data.h"
#include "pios_usb.h"
#include "pios_usb_priv.h"

#if defined(PIOS_INCLUDE_USB)

/* Rx/Tx status */
static uint8_t transfer_possible = 0;

enum pios_usb_dev_magic {
	PIOS_USB_DEV_MAGIC = 0x17365904,
};

struct pios_usb_dev {
	enum pios_usb_dev_magic     magic;
	const struct pios_usb_cfg * cfg;
};

/**
 * @brief Validate the usb device structure
 * @returns true if valid device or false otherwise
 */
static bool PIOS_USB_validate(struct pios_usb_dev * usb_dev)
{
	return (usb_dev && (usb_dev->magic == PIOS_USB_DEV_MAGIC));
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
 * Bind configuration to USB BSP layer
 * \return < 0 if initialisation failed
 */
static uint32_t pios_usb_id;
int32_t PIOS_USB_Init(uint32_t * usb_id, const struct pios_usb_cfg * cfg)
{
	PIOS_Assert(usb_id);
	PIOS_Assert(cfg);

	struct pios_usb_dev * usb_dev;

	usb_dev = (struct pios_usb_dev *) PIOS_USB_alloc();
	if (!usb_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	usb_dev->cfg = cfg;

	/*
	 * This is a horrible hack to make this available to
	 * the interrupt callbacks.  This should go away ASAP.
	 */
	pios_usb_id = (uint32_t) usb_dev;

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
int32_t PIOS_USB_ChangeConnectionState(bool connected)
{
	// In all cases: re-initialise USB HID driver
	if (connected) {
		transfer_possible = 1;

		//TODO: Check SetEPRxValid(ENDP1);

#if defined(USB_LED_ON)
		USB_LED_ON;	// turn the USB led on
#endif
	} else {
		// Cable disconnected: disable transfers
		transfer_possible = 0;

#if defined(USB_LED_OFF)
		USB_LED_OFF;	// turn the USB led off
#endif
	}

	return 0;
}

/**
 * This function returns the connection status of the USB interface
 * \return 1: interface available
 * \return 0: interface not available
 */
uint32_t usb_found;
bool PIOS_USB_CheckAvailable(uint8_t id)
{
	struct pios_usb_dev * usb_dev = (struct pios_usb_dev *) pios_usb_id;

	if(!PIOS_USB_validate(usb_dev))
		return false;

	usb_found = (usb_dev->cfg->vsense.gpio->IDR & usb_dev->cfg->vsense.init.GPIO_Pin);
	return usb_found;
	return usb_found != 0 && transfer_possible ? 1 : 0;
}

/*
 *
 * Provide STM32 USB OTG BSP layer API
 *
 */

#include "usb_bsp.h"

void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
	struct pios_usb_dev * usb_dev = (struct pios_usb_dev *) pios_usb_id;

	bool valid = PIOS_USB_validate(usb_dev);
	PIOS_Assert(valid);

#define FORCE_DISABLE_USB_IRQ 1
#if FORCE_DISABLE_USB_IRQ
	/* Make sure we disable the USB interrupt since it may be left on by bootloader */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure = usb_dev->cfg->irq.init;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

	/* Configure USB D-/D+ (DM/DP) pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_OTG1_FS);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_OTG1_FS);

	/* Configure VBUS sense pin */
	GPIO_Init(usb_dev->cfg->vsense.gpio, &usb_dev->cfg->vsense.init);

	/* Enable USB OTG Clock */
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE);
}

void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
	struct pios_usb_dev * usb_dev = (struct pios_usb_dev *) pios_usb_id;

	bool valid = PIOS_USB_validate(usb_dev);
	PIOS_Assert(valid);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_Init(&usb_dev->cfg->irq.init);
}

#ifdef USE_HOST_MODE
void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev, uint8_t state)
{

}

void USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev)
{

}
#endif	/* USE_HOST_MODE */

void USB_OTG_BSP_TimeInit ( void )
{

}

void USB_OTG_BSP_uDelay (const uint32_t usec)
{
	uint32_t count = 0;
	const uint32_t utime = (120 * usec / 7);
	do {
		if (++count > utime) {
			return ;
		}
	}
	while (1); 
}

void USB_OTG_BSP_mDelay (const uint32_t msec)
{
	USB_OTG_BSP_uDelay(msec * 1000);
}

void USB_OTG_BSP_TimerIRQ (void)
{

}

#endif	/* PIOS_INCLUDE_USB */

/**
 * @}
 * @}
 */
