/**
 ******************************************************************************
 *
 * @file       op_logging.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      OpenPilot Logging Functions header.
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


#ifndef OP_LOGGING_H
#define OP_LOGGING_H

/* Defines */
#define OP_LOGGING_TASK_PRI	( tskIDLE_PRIORITY + 4 )

/* Type Definitions */
typedef enum {FLIGHT_LOG, RC_LOG} LogTypeTypeDef;
typedef struct {
	LogTypeTypeDef Type;
	char *Message;
} LogTypeDef;


/* Function Prototypes */
extern void OP_Logging_Init(void);


#endif /* OP_LOGGING_H */
