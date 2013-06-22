/**
 ******************************************************************************
 *
 * @file       msd.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      MSD functions header.
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
#ifndef _MSD_H
#define _MSD_H

/////////////////////////////////////////////////////////////////////////////
// Global definitions
/////////////////////////////////////////////////////////////////////////////

/* defines how many bytes are sent per packet */
#define MSD_BULK_MAX_PACKET_SIZE  0x40

/* max. number of supported logical units */
#define MSD_NUM_LUN 1

/* defines how many endpoints are used by the device */
#define MSD_EP_NUM 3

/* buffer table base address */

#define MSD_BTABLE_ADDRESS      (0x00)

/* EP0  */
/* rx/tx buffer base address */
#define MSD_ENDP0_RXADDR        (0x18)
#define MSD_ENDP0_TXADDR        (0x58)

/* EP1  */
/* tx buffer base address */
#define MSD_ENDP1_TXADDR        (0x98)

/* EP2  */
/* Rx buffer base address */
#define MSD_ENDP2_RXADDR        (0xD8)



/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////

extern s32 MSD_Init(u32 mode);
extern s32 MSD_Periodic_mS(void);

extern s32 MSD_CheckAvailable(void);

extern s32 MSD_LUN_AvailableSet(u8 lun, u8 available);
extern s32 MSD_LUN_AvailableGet(u8 lun);

/////////////////////////////////////////////////////////////////////////////
// Export global variables
/////////////////////////////////////////////////////////////////////////////


#endif /* _MSD_H */
