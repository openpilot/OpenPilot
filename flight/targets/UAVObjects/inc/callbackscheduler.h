/**
 ******************************************************************************
 *
 * @file       callbackscheduler.h
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

#ifndef CALLBACKSCHEDULER_H
#define CALLBACKSCHEDULER_H

// Public types
typedef enum{CALLBACK_PRIORITY_CRITICAL=0,CALLBACK_PRIORITY_REGULAR=1,CALLBACK_PRIORITY_LOW=2} DelayedCallbackPriority;

typedef void (*DelayedCallback)(void);

struct DelayedCallbackInfoStruct;
typedef struct DelayedCallbackInfoStruct DelayedCallbackInfo;

// Public functions
//
int32_t CallbackSchedulerInitialize();
int32_t CallbackSchedulerStart();
int32_t DelayedCallbackDispatch(DelayedCallbackInfo *cbinfo);
int32_t DelayedCallbackDispatchFromISR(DelayedCallbackInfo *cbinfo, long *pxHigherPriorityTaskWoken);
DelayedCallbackInfo* DelayedCallbackCreate(DelayedCallback cb, DelayedCallbackPriority priority, long taskPriority, uint32_t stacksize);

#endif // CALLBACKSCHEDULER_H
