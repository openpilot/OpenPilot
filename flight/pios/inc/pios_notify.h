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
#include <optypes.h>
typedef enum {
    NOTIFY_NONE = 0,
    NOTIFY_OK,
    NOTIFY_NOK,
    NOTIFY_DRAW_ATTENTION
} pios_notify_notification;

typedef enum {
    NOTIFY_PRIORITY_CRITICAL   = 2,
    NOTIFY_PRIORITY_REGULAR    = 1,
    NOTIFY_PRIORITY_LOW = 0,
    NOTIFY_PRIORITY_BACKGROUND = -1,
} pios_notify_priority;


// A single led step, Color, time On/Off, repeat count
#define NOTIFY_SEQUENCE_MAX_STEPS 5

typedef struct {
    uint16_t time_off;
    uint16_t time_on;
    Color_t  color;
    uint8_t  repeats;
} LedStep_t;

typedef struct {
    int8_t    repeats; // -1 for infinite repetitions
    LedStep_t steps[NOTIFY_SEQUENCE_MAX_STEPS];
} LedSequence_t;

typedef struct {
    LedSequence_t sequence;
    pios_notify_priority priority;
} ExtLedNotification_t;

#define NOTIFY_IS_NULL_STEP(x) (!x || (!x->time_off && !x->time_on && !x->repeats))

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

/*
 * Play a sequence on the default external led. Sequences with priority higher than NOTIFY_PRIORITY_LOW
 * are repeated only once if repeat = -1
 * @param sequence Sequence to be played
 * @param priority Priority of the sequence being played
 */
void PIOS_NOTIFICATION_Default_Ext_Led_Play(const LedSequence_t *sequence, pios_notify_priority priority);

/*
 * Play a sequence on an external rgb led. Sequences with priority higher than NOTIFY_PRIORITY_LOW
 * are repeated only once if repeat = -1
 * @param sequence Sequence to be played
 * @param ledNumber Led number
 * @param priority Priority of the sequence being played
 *
 */
// void PIOS_NOTIFICATION_Ext_Led_Play(const LedSequence_t sequence, uint8_t ledNumber, pios_notify_priority priority);

ExtLedNotification_t *PIOS_NOTIFY_GetNewExtLedSequence(bool clear);

#endif /* PIOS_NOTIFY_H_ */
