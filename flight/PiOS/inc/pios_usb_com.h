/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SYS System Functions
 * @brief PIOS USB communication code
 * @{
 *
 * @file       pios_usb_com.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.
 * @brief      USB_COM functions header.
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

#ifndef PIOS_USB_COM_H
#define PIOS_USB_COM_H


/////////////////////////////////////////////////////////////////////////////
// Global definitions
/////////////////////////////////////////////////////////////////////////////

// number of USB_COM interfaces (0..1)
#ifndef PIOS_USB_COM_NUM
#define PIOS_USB_COM_NUM 1
#endif


// size of IN/OUT pipe
#ifndef PIOS_USB_COM_DATA_IN_SIZE
#define PIOS_USB_COM_DATA_IN_SIZE            64
#endif

#ifndef PIOS_USB_COM_DATA_OUT_SIZE
#define PIOS_USB_COM_DATA_OUT_SIZE           64
#endif


#ifndef PIOS_USB_COM_INT_IN_SIZE
#define PIOS_USB_COM_INT_IN_SIZE             64
#endif

/* Public Functions */
extern s32 PIOS_USB_COM_Init(u32 mode);
extern s32 PIOS_USB_COM_CheckAvailable(void);
extern s32 PIOS_USB_COM_ChangeConnectionState(u8 connected);
extern void PIOS_USB_COM_EP4_IN_Callback(void);
extern void PIOS_USB_COM_EP3_OUT_Callback(void);
extern void PIOS_USB_COM_CB_StatusIn(void);
extern s32 PIOS_USB_COM_CB_Data_Setup(u8 RequestNo);
extern s32 PIOS_USB_COM_CB_NoData_Setup(u8 RequestNo);
extern s32 PIOS_USB_COM_RxBufferFree(u8 usb_com);
extern s32 PIOS_USB_COM_RxBufferUsed(u8 usb_com);
extern s32 PIOS_USB_COM_RxBufferGet(u8 usb_com);
extern s32 PIOS_USB_COM_TxBufferFree(u8 usb_com);
extern s32 PIOS_USB_COM_TxBufferUsed(u8 usb_com);
extern s32 PIOS_USB_COM_TxBufferPut_NonBlocking(u8 usb_com, u8 b);
extern s32 PIOS_USB_COM_TxBufferPut(u8 usb_com, u8 b);
extern s32 PIOS_USB_COM_TxBufferPutMore_NonBlocking(u8 usb_com, u8 *buffer, u16 len);
extern s32 PIOS_USB_COM_TxBufferPutMore(u8 usb_com, u8 *buffer, u16 len);

#endif /* PIOS_USB_COM_H */

/**
  * @}
  * @}
  */