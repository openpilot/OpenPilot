/**
 ******************************************************************************
 *
 * @file       uavobjecttemplate.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Template file for all objects. This file will not compile, it is used
 * 			   by the parser as a template to generate all other objects. All $(x) fields
 * 			   will be replaced by the parser with the actual object information.
 * 			   Each object has an meta object associated with it. The meta object
 *             contains information such as the telemetry and logging properties.
 *
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

#ifndef $(NAMEUC)_H
#define $(NAMEUC)_H

#include <stdint.h>
#include "uavobject.h"
#include "FreeRTOS.h"
#include "queue.h"

// Object data
typedef struct {
	$(DATAFIELDS)
} __attribute__((packed)) $(NAME)Data;

// Generic interface functions
int32_t $(NAME)Initialize();
UAVObject* $(NAME)Get();
void $(NAME)GetData(TestObjectData* dataOut);
void $(NAME)SetData(const TestObjectData* dataIn);

#endif // $(NAME)_H
