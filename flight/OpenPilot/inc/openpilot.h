/**
 ******************************************************************************
 *
 * @file       openpilot.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.
 * @brief      Main OpenPilot header.
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


#ifndef OPENPILOT_H
#define OPENPILOT_H


/* PIOS Includes */
#include "pios.h"

/* OpenPilot Includes */
/* We use this to include all OpenPilot head files */

/* FreeRTOS Includes */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

/* Function Prototypes */
extern void OpenPilotInit(void);
extern void vApplicationIdleHook(void);

/* Hooks */
extern void ADCNotifyChange(uint32_t pin, uint32_t pin_value);

#endif /* OPENPILOT_H */
