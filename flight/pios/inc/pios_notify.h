/**
 ******************************************************************************
 *
 * @file       pios_notify.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Handles user notifications.
 *             --
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
#ifndef PIOS_NOTIFY_H_
#define PIOS_NOTIFY_H_
#include <stdbool.h>

typedef enum {
    NOTIFY_NONE = 0,
    NOTIFY_OK,
    NOTIFY_NOK,
    NOTIFY_DRAW_ATTENTION
} pios_notify_notification;

typedef enum {
    NOTIFY_PRIORITY_CRITICAL = 2,
    NOTIFY_PRIORITY_REGULAR  = 1,
    NOTIFY_PRIORITY_LOW = 0,
} pios_notify_priority;

/**
 * start a new notification. If a notification is active it will be overwritten if its priority is lower than the new one.
 * The new will be discarded otherwise
 * @param notification kind of notification
 * @param priority priority of the new notification
 */
void PIOS_NOTIFY_StartNotification(pios_notify_notification notification, pios_notify_priority priority);

/**
 * retrieve any active notification
 * @param clear
 * @return
 */
pios_notify_notification PIOS_NOTIFY_GetActiveNotification(bool clear);

#endif /* PIOS_NOTIFY_H_ */
