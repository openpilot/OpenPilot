/**
 ******************************************************************************
 *
 * @file       pios_notify.c
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

#include "pios_notify.h"

static volatile pios_notify_notification currentNotification = NOTIFY_NONE;
static volatile pios_notify_priority currentPriority;
static volatile ExtLedNotification_t extLedNotification;
static volatile bool newNotification;

void PIOS_NOTIFY_StartNotification(pios_notify_notification notification, pios_notify_priority priority)
{
    if (currentNotification == NOTIFY_NONE || currentPriority < priority) {
        currentPriority     = priority;
        currentNotification = notification;
    }
}

pios_notify_notification PIOS_NOTIFY_GetActiveNotification(bool clear)
{
    pios_notify_notification ret = currentNotification;

    if (clear && ret != NOTIFY_NONE) {
        currentNotification = NOTIFY_NONE;
    }
    return ret;
}


/*
 * Play a sequence on the default external led. Sequences with priority higher than NOTIFY_PRIORITY_LOW
 * are repeated only once if repeat = -1
 * @param sequence Sequence to be played
 * @param priority Priority of the sequence being played
 */
void PIOS_NOTIFICATION_Default_Ext_Led_Play(const LedSequence_t *sequence, pios_notify_priority priority)
{
    // alert and alarms are repeated if condition persists. bacground notification instead are set once, so try to prevent loosing any update
    if (newNotification && priority != NOTIFY_PRIORITY_BACKGROUND) {
        // prevent overwriting higher priority or background notifications
        if (extLedNotification.priority == NOTIFY_PRIORITY_BACKGROUND || extLedNotification.priority > priority) {
            return;
        }
    }
    extLedNotification.priority = priority;
    extLedNotification.sequence = *sequence;
    newNotification = true;
}


ExtLedNotification_t *PIOS_NOTIFY_GetNewExtLedSequence(bool clear)
{
    if (!newNotification) {
        return 0;
    }
    if (clear) {
        newNotification = false;
    }
    return (ExtLedNotification_t *)&extLedNotification;
}
