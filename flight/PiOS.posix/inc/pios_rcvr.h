/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_RCVR RCVR layer functions
 * @brief Hardware communication layer
 * @{
 *
 * @file       pios_rcvr.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RCVR layer functions header
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

#ifndef PIOS_RCVR_H
#define PIOS_RCVR_H

struct pios_rcvr_driver {
	void    (*init)(uint32_t id);
	int32_t (*read)(uint32_t id, uint8_t channel);
};

/* Public Functions */
extern int32_t PIOS_RCVR_Read(uint32_t rcvr_id, uint8_t channel);

/*! Define error codes for PIOS_RCVR_Get */
enum PIOS_RCVR_errors {
	/*! Indicates that a failsafe condition or missing receiver detected for that channel */
	PIOS_RCVR_TIMEOUT = 0,
	/*! Channel is invalid for this driver (usually out of range supported) */
	PIOS_RCVR_INVALID = -1,
	/*! Indicates that the driver for this channel has not been initialized */
	PIOS_RCVR_NODRIVER = -2
};

#endif /* PIOS_RCVR_H */

/**
  * @}
  * @}
  */
