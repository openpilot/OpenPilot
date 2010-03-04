/**
 ******************************************************************************
 *
 * @file       eventdispatcher.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Include files of the uavobjectlist library
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

#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H

#include <stdint.h>
#include "uavobjectmanager.h"

// Public types

// Public functions
int32_t EventDispatcherInitialize();
int32_t EventDispatch(UAVObjEvent* ev, UAVObjEventCallback cb);
int32_t EventPeriodicCreate(UAVObjEvent* ev, UAVObjEventCallback cb, int32_t periodMs);
int32_t EventPeriodicUpdate(UAVObjEvent* ev, UAVObjEventCallback cb, int32_t periodMs);
int32_t EventPeriodicDelete(UAVObjEvent* ev, UAVObjEventCallback cb);

#endif // EVENTDISPATCHER_H
