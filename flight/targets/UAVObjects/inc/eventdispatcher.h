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

// Public types
/**
 * Event dispatcher statistics
 */
typedef struct {
	uint32_t lastErrorID;
	uint32_t eventErrors;
} EventStats;

// Public functions
int32_t EventDispatcherInitialize();
void EventGetStats(EventStats* statsOut);
void EventClearStats();
int32_t EventCallbackDispatch(UAVObjEvent* ev, UAVObjEventCallback cb);
int32_t EventPeriodicCallbackCreate(UAVObjEvent* ev, UAVObjEventCallback cb, uint16_t periodMs);
int32_t EventPeriodicCallbackUpdate(UAVObjEvent* ev, UAVObjEventCallback cb, uint16_t periodMs);
int32_t EventPeriodicQueueCreate(UAVObjEvent* ev, xQueueHandle queue, uint16_t periodMs);
int32_t EventPeriodicQueueUpdate(UAVObjEvent* ev, xQueueHandle queue, uint16_t periodMs);

#endif // EVENTDISPATCHER_H
