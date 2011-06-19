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
 * 	       Parts by Thorsten Klose (tk@midibox.org)
 * @brief      COM layer functions header
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

extern uint32_t pios_rcvr_channel_to_id_map[];

struct pios_rcvr_driver {
  void    (*init)(uint32_t id);
  int32_t (*read)(uint32_t id);
};

/* Public Functions */
extern int32_t PIOS_RCVR_Read(uint32_t rcvr_id);

#endif /* PIOS_RCVR_H */

/**
  * @}
  * @}
  */
