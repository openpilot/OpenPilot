/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_USBHOOK USB glue code
 * @brief Glue between PiOS and STM32 libs
 * @{
 *
 * @file       pios_usbhook.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Glue between PiOS and STM32 libs
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

#include "pios.h"
#include "pios_usb.h"		/* PIOS_USB_* */
#include "pios_usbhook.h"
#include "pios_usb_defs.h"	/* struct usb_* */
#include "pios_usb_cdc_priv.h"	/* PIOS_USB_CDC_* */
#include "pios_usb_board_data.h" /* PIOS_USB_BOARD_* */


/* STM32 USB Library Definitions */
#include "usb_core.h"		/* USBD_Class_cb_TypeDef */
#include "usbd_core.h"		/* USBD_Init USBD_OK*/
#include "usbd_ioreq.h"		/* USBD_CtlPrepareRx, USBD_CtlSendData */
#include "usbd_req.h"		/* USBD_CtlError */
#include "usb_dcd_int.h"	/* USBD_OTG_ISR_Handler */

/*
 * External API
 */
static struct pios_usbhook_descriptor Device_Descriptor;

void PIOS_USBHOOK_RegisterDevice(const uint8_t * desc, uint16_t length)
{
	Device_Descriptor.descriptor = desc;
	Device_Descriptor.length     = length;
}

static struct pios_usbhook_descriptor String_Descriptor[4];

void PIOS_USBHOOK_RegisterString(enum usb_string_desc string_id, const uint8_t * desc, uint16_t desc_size)
{
	if (string_id < NELEMENTS(String_Descriptor)) {
		String_Descriptor[string_id].descriptor = desc;
		String_Descriptor[string_id].length     = desc_size;
	}
}

static struct pios_usbhook_descriptor Config_Descriptor;

void PIOS_USBHOOK_RegisterConfig(uint8_t config_id, const uint8_t * desc, uint16_t desc_size)
{
	Config_Descriptor.descriptor = desc;
	Config_Descriptor.length     = desc_size;
}

static USB_OTG_CORE_HANDLE pios_usb_otg_core_handle;
static USBD_Class_cb_TypeDef class_callbacks;
static USBD_DEVICE device_callbacks;
static USBD_Usr_cb_TypeDef user_callbacks;

void PIOS_USBHOOK_Activate(void)
{
	USBD_Init(&pios_usb_otg_core_handle,
		USB_OTG_FS_CORE_ID,
		&device_callbacks,
		&class_callbacks,
		&user_callbacks);
}

void PIOS_USBHOOK_Deactivate(void)
{
	DCD_DevDisconnect(&pios_usb_otg_core_handle);
	USBD_DeInit(&pios_usb_otg_core_handle);
	USB_OTG_StopDevice(&pios_usb_otg_core_handle);
}

void OTG_FS_IRQHandler(void)
{
	if(!USBD_OTG_ISR_Handler(&pios_usb_otg_core_handle)) {
		/* spurious interrupt, disable IRQ */
	  
	}
}

struct usb_if_entry {
  struct pios_usb_ifops *ifops;
  uint32_t context;
};
static struct usb_if_entry usb_if_table[3];
void PIOS_USBHOOK_RegisterIfOps(uint8_t ifnum, struct pios_usb_ifops * ifops, uint32_t context)
{
	PIOS_Assert(ifnum < NELEMENTS(usb_if_table));
	PIOS_Assert(ifops);

	usb_if_table[ifnum].ifops   = ifops;
	usb_if_table[ifnum].context = context;
}

struct usb_ep_entry {
	pios_usbhook_epcb cb;
	uint32_t context;
	uint16_t max_len;
};
static struct usb_ep_entry usb_epin_table[6];
void PIOS_USBHOOK_RegisterEpInCallback(uint8_t epnum, uint16_t max_len, pios_usbhook_epcb cb, uint32_t context)
{
	PIOS_Assert(epnum < NELEMENTS(usb_epin_table));
	PIOS_Assert(cb);

	usb_epin_table[epnum].cb      = cb;
	usb_epin_table[epnum].context = context;
	usb_epin_table[epnum].max_len = max_len;

	DCD_EP_Open(&pios_usb_otg_core_handle,
		epnum | 0x80,
		max_len,
		USB_OTG_EP_INT);
	/*
	 * FIXME do not hardcode endpoint type
	 */
}

extern void PIOS_USBHOOK_DeRegisterEpInCallback(uint8_t epnum)
{
	PIOS_Assert(epnum < NELEMENTS(usb_epin_table));

	usb_epin_table[epnum].cb = NULL;

	DCD_EP_Close(&pios_usb_otg_core_handle, epnum | 0x80);
}

static struct usb_ep_entry usb_epout_table[6];
void PIOS_USBHOOK_RegisterEpOutCallback(uint8_t epnum, uint16_t max_len, pios_usbhook_epcb cb, uint32_t context)
{
  PIOS_Assert(epnum < NELEMENTS(usb_epout_table));
  PIOS_Assert(cb);

  usb_epout_table[epnum].cb      = cb;
  usb_epout_table[epnum].context = context;
  usb_epout_table[epnum].max_len = max_len;

  DCD_EP_Open(&pios_usb_otg_core_handle,
	      epnum,
	      max_len,
	      USB_OTG_EP_INT);
	/*
	 * FIXME do not hardcode endpoint type
	 */
}

extern void PIOS_USBHOOK_DeRegisterEpOutCallback(uint8_t epnum)
{
	PIOS_Assert(epnum < NELEMENTS(usb_epout_table));

	usb_epout_table[epnum].cb = NULL;

	DCD_EP_Close(&pios_usb_otg_core_handle, epnum);
}

void PIOS_USBHOOK_CtrlTx(const uint8_t *buf, uint16_t len)
{
	USBD_CtlSendData(&pios_usb_otg_core_handle, buf, len);
}

void PIOS_USBHOOK_CtrlRx(uint8_t *buf, uint16_t len)
{
	USBD_CtlPrepareRx(&pios_usb_otg_core_handle, buf, len);
}

void PIOS_USBHOOK_EndpointTx(uint8_t epnum, const uint8_t *buf, uint16_t len)
{
	if (pios_usb_otg_core_handle.dev.device_status == USB_OTG_CONFIGURED) {
		DCD_EP_Tx(&pios_usb_otg_core_handle, epnum, buf, len);
	}
}

void PIOS_USBHOOK_EndpointRx(uint8_t epnum, uint8_t *buf, uint16_t len)
{
	DCD_EP_PrepareRx(&pios_usb_otg_core_handle, epnum, buf, len);
}

/*
 * Device level hooks into STM USB library
 */

static const uint8_t * PIOS_USBHOOK_DEV_GetDeviceDescriptor(uint8_t speed, uint16_t *length)
{
	*length = Device_Descriptor.length;
	return Device_Descriptor.descriptor;
}

static const uint8_t * PIOS_USBHOOK_DEV_GetLangIDStrDescriptor(uint8_t speed, uint16_t *length)
{
	*length = String_Descriptor[USB_STRING_DESC_LANG].length;
	return String_Descriptor[USB_STRING_DESC_LANG].descriptor;
}

static const uint8_t * PIOS_USBHOOK_DEV_GetManufacturerStrDescriptor(uint8_t speed, uint16_t *length)
{
	*length = String_Descriptor[USB_STRING_DESC_VENDOR].length;
	return String_Descriptor[USB_STRING_DESC_VENDOR].descriptor;
}

static const uint8_t * PIOS_USBHOOK_DEV_GetProductStrDescriptor(uint8_t speed, uint16_t *length)
{
	*length = String_Descriptor[USB_STRING_DESC_PRODUCT].length;
	return String_Descriptor[USB_STRING_DESC_PRODUCT].descriptor;
}

static const uint8_t * PIOS_USBHOOK_DEV_GetSerialStrDescriptor(uint8_t speed, uint16_t *length)
{
	*length = String_Descriptor[USB_STRING_DESC_SERIAL].length;
	return String_Descriptor[USB_STRING_DESC_SERIAL].descriptor;
}

static const uint8_t * PIOS_USBHOOK_DEV_GetConfigurationStrDescriptor(uint8_t speed, uint16_t *length)
{
	return NULL;
}

static const uint8_t * PIOS_USBHOOK_DEV_GetInterfaceStrDescriptor(uint8_t speed, uint16_t *length)
{
	return NULL;
}

static USBD_DEVICE device_callbacks = {
	.GetDeviceDescriptor           = PIOS_USBHOOK_DEV_GetDeviceDescriptor,
	.GetLangIDStrDescriptor        = PIOS_USBHOOK_DEV_GetLangIDStrDescriptor,
	.GetManufacturerStrDescriptor  = PIOS_USBHOOK_DEV_GetManufacturerStrDescriptor,
	.GetProductStrDescriptor       = PIOS_USBHOOK_DEV_GetProductStrDescriptor,
	.GetSerialStrDescriptor        = PIOS_USBHOOK_DEV_GetSerialStrDescriptor,
	.GetConfigurationStrDescriptor = PIOS_USBHOOK_DEV_GetConfigurationStrDescriptor,
	.GetInterfaceStrDescriptor     = PIOS_USBHOOK_DEV_GetInterfaceStrDescriptor,
};

static void PIOS_USBHOOK_USR_Init(void)
{
	PIOS_USB_ChangeConnectionState(false);

#if 1
	/* Force a physical disconnect/reconnect */
	DCD_DevDisconnect(&pios_usb_otg_core_handle);
	DCD_DevConnect(&pios_usb_otg_core_handle);
#endif
}

static void PIOS_USBHOOK_USR_DeviceReset(uint8_t speed)
{
	PIOS_USB_ChangeConnectionState(false);
}

static void PIOS_USBHOOK_USR_DeviceConfigured(void)
{
	PIOS_USB_ChangeConnectionState(true);
}

static void PIOS_USBHOOK_USR_DeviceSuspended(void)
{
	/* Unhandled */
}

static void PIOS_USBHOOK_USR_DeviceResumed(void)
{
	/* Unhandled */
}

static void PIOS_USBHOOK_USR_DeviceConnected(void)
{
	/* NOP */
}

static void PIOS_USBHOOK_USR_DeviceDisconnected(void)
{
	PIOS_USB_ChangeConnectionState(false);
}

static USBD_Usr_cb_TypeDef user_callbacks = {
	.Init               = PIOS_USBHOOK_USR_Init,
	.DeviceReset        = PIOS_USBHOOK_USR_DeviceReset,
	.DeviceConfigured   = PIOS_USBHOOK_USR_DeviceConfigured,
	.DeviceSuspended    = PIOS_USBHOOK_USR_DeviceSuspended,
	.DeviceResumed      = PIOS_USBHOOK_USR_DeviceResumed,
	.DeviceConnected    = PIOS_USBHOOK_USR_DeviceConnected,
	.DeviceDisconnected = PIOS_USBHOOK_USR_DeviceDisconnected,
};

static uint8_t PIOS_USBHOOK_CLASS_Init(void *pdev, uint8_t cfgidx)
{
  /* Call all of the registered init callbacks */
  for (uint8_t i = 0; i < NELEMENTS(usb_if_table); i++) {
    struct usb_if_entry * usb_if = &(usb_if_table[i]);
    if (usb_if->ifops && usb_if->ifops->init) {
      usb_if->ifops->init(usb_if->context);
    }
  }
  return USBD_OK;
}

static uint8_t PIOS_USBHOOK_CLASS_DeInit(void *pdev, uint8_t cfgidx)
{
  /* Call all of the registered deinit callbacks */
  for (uint8_t i = 0; i < NELEMENTS(usb_if_table); i++) {
    struct usb_if_entry * usb_if = &(usb_if_table[i]);
    if (usb_if->ifops && usb_if->ifops->deinit) {
      usb_if->ifops->deinit(usb_if->context);
    }
  }
  return USBD_OK;
}

static struct usb_setup_request usb_ep0_active_req;
static uint8_t PIOS_USBHOOK_CLASS_Setup(void *pdev, USB_SETUP_REQ *req)
{
	switch (req->bmRequest & (USB_REQ_TYPE_MASK | USB_REQ_RECIPIENT_MASK)) {
	case (USB_REQ_TYPE_STANDARD | USB_REQ_RECIPIENT_INTERFACE):
	case (USB_REQ_TYPE_CLASS    | USB_REQ_RECIPIENT_INTERFACE):
	{
		uint8_t ifnum = LOBYTE(req->wIndex);
		if ((ifnum < NELEMENTS(usb_if_table)) &&
		    (usb_if_table[ifnum].ifops && usb_if_table[ifnum].ifops->setup)) {
			usb_if_table[ifnum].ifops->setup(usb_if_table[ifnum].context, 
							 (struct usb_setup_request *)req);
			if (req->bmRequest & 0x80 && req->wLength > 0) {
				/* Request is a host-to-device data setup packet, keep track of the request details for the EP0_RxReady call */
				usb_ep0_active_req.bmRequestType = req->bmRequest;
				usb_ep0_active_req.bRequest      = req->bRequest;
				usb_ep0_active_req.wValue        = req->wValue;
				usb_ep0_active_req.wIndex        = req->wIndex;
				usb_ep0_active_req.wLength       = req->wLength;
			}
		} else {
			/* No Setup handler or Setup handler failed */
			USBD_CtlError (&pios_usb_otg_core_handle, req);
		}
		break;
	}
	default:
		/* Unhandled Setup */
		USBD_CtlError (&pios_usb_otg_core_handle, req);
 		break;
	}

	return USBD_OK;
}

static uint8_t PIOS_USBHOOK_CLASS_EP0_TxSent(void *pdev)
{
	return USBD_OK;
}

static uint8_t PIOS_USBHOOK_CLASS_EP0_RxReady(void *pdev)
{
	uint8_t ifnum = LOBYTE(usb_ep0_active_req.wIndex);

	if ((ifnum < NELEMENTS(usb_if_table)) &&
	    (usb_if_table[ifnum].ifops && usb_if_table[ifnum].ifops->ctrl_data_out)) {
		usb_if_table[ifnum].ifops->ctrl_data_out(usb_if_table[ifnum].context, 
							&usb_ep0_active_req);
	}

	return USBD_OK;
}

static uint8_t PIOS_USBHOOK_CLASS_DataIn(void *pdev, uint8_t epnum)
{
	/* Make sure the previous transfer has completed before starting a new one */
	DCD_EP_Flush(pdev, epnum);	/* NOT SURE IF THIS IS REQUIRED */

	/* Remove the direction bit so we can use this as an index */
	uint8_t epnum_idx = epnum & 0x7F;

	if ((epnum_idx < NELEMENTS(usb_epin_table)) && usb_epin_table[epnum_idx].cb) {
		struct usb_ep_entry *ep = &(usb_epin_table[epnum_idx]);
		if (!ep->cb(ep->context, epnum_idx, ep->max_len)) {
			/* NOTE: use real endpoint number including direction bit */
			DCD_SetEPStatus(pdev, epnum, USB_OTG_EP_TX_NAK);
		}
	}

	return USBD_OK;
}

static uint8_t PIOS_USBHOOK_CLASS_DataOut(void *pdev, uint8_t epnum)
{
	/* Remove the direction bit so we can use this as an index */
	uint8_t epnum_idx = epnum & 0x7F;

	if ((epnum_idx < NELEMENTS(usb_epout_table)) && usb_epout_table[epnum_idx].cb) {
		struct usb_ep_entry *ep = &(usb_epout_table[epnum_idx]);
		if (!ep->cb(ep->context, epnum_idx, ep->max_len)) {
			/* NOTE: use real endpoint number including direction bit */
			DCD_SetEPStatus(pdev, epnum, USB_OTG_EP_RX_NAK);
		}
	}

	return USBD_OK;
}

static uint8_t PIOS_USBHOOK_CLASS_SOF(void *pdev)
{
	return USBD_OK;
}

static uint8_t PIOS_USBHOOK_CLASS_IsoINIncomplete(void *pdev)
{
	return USBD_OK;
}

static uint8_t PIOS_USBHOOK_CLASS_IsoOUTIncomplete(void *pdev)
{
	return USBD_OK;
}

static const uint8_t * PIOS_USBHOOK_CLASS_GetConfigDescriptor(uint8_t speed, uint16_t *length)
{
	*length = Config_Descriptor.length;
	return Config_Descriptor.descriptor;
}

#ifdef USB_OTG_HS_CORE
static const uint8_t * PIOS_USBHOOK_CLASS_GetOtherConfigDescriptor(uint8_t speed, uint16_t *length)
{
	return PIOS_USBHOOK_CLASS_GetConfigDescriptor(speed, length);
}
#endif	/* USB_OTG_HS_CORE */

#ifdef USB_SUPPORT_USER_STRING_DESC
static const uint8_t * PIOS_USBHOOK_CLASS_GetUsrStrDescriptor(uint8_t speed, uint8_t index, uint16_t *length)
{
	return NULL;
}
#endif	/* USB_SUPPORT_USER_STRING_DESC */

static USBD_Class_cb_TypeDef class_callbacks = {
	.Init                     = PIOS_USBHOOK_CLASS_Init,
	.DeInit                   = PIOS_USBHOOK_CLASS_DeInit,
	.Setup                    = PIOS_USBHOOK_CLASS_Setup,
	.EP0_TxSent               = PIOS_USBHOOK_CLASS_EP0_TxSent,
	.EP0_RxReady              = PIOS_USBHOOK_CLASS_EP0_RxReady,
	.DataIn                   = PIOS_USBHOOK_CLASS_DataIn,
	.DataOut                  = PIOS_USBHOOK_CLASS_DataOut,
	.SOF                      = PIOS_USBHOOK_CLASS_SOF,
	.IsoINIncomplete          = PIOS_USBHOOK_CLASS_IsoINIncomplete,
	.IsoOUTIncomplete         = PIOS_USBHOOK_CLASS_IsoOUTIncomplete,
	.GetConfigDescriptor      = PIOS_USBHOOK_CLASS_GetConfigDescriptor,
#ifdef USB_OTG_HS_CORE
	.GetOtherConfigDescriptor = PIOS_USBHOOK_CLASS_GetOtherConfigDescriptor,
#endif	/* USB_OTG_HS_CORE */
#ifdef USB_SUPPORT_USER_STRING_DESC
	.GetUsrStrDescriptor      = PIOS_USBHOOK_CLASS_GetUsrStrDescriptor,
#endif	/* USB_SUPPORT_USER_STRING_DESC */
};

