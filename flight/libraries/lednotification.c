/**
 ******************************************************************************
 *
 * @file       lednotification.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      led notification library.
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
#include "inc/lednotification.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <FreeRTOS.h>
#include <pios.h>
#include <pios_notify.h>
#include <pios_ws2811.h>

// Private defines

// Maximum number of notifications enqueued when a higher priority notification is added
#define MAX_BACKGROUND_NOTIFICATIONS 5
#define MAX_HANDLED_LED              1

#define BACKGROUND_SEQUENCE          -1
#define RESET_STEP                   -1
#define GET_CURRENT_MILLIS           (xTaskGetTickCount() * portTICK_RATE_MS)

// Private data types definition
// this is the status for a single notification led set
typedef struct {
    int8_t   queued_priorities[MAX_BACKGROUND_NOTIFICATIONS];
    LedSequence_t queued_sequences[MAX_BACKGROUND_NOTIFICATIONS];
    LedSequence_t background_sequence;
    uint32_t next_run_time;
    uint32_t sequence_starting_time;

    int8_t   active_sequence_num; // active queued sequence or BACKGROUND_SEQUENCE
    bool     running; // is this led running?
    bool     step_phase_on; // true = step on phase, false = step off phase
    uint8_t  next_sequence_step; // (step number to be executed) << 1 || (0x00 = on phase, 0x01 = off phase)
    uint8_t  next_step_rep; // next repetition number for next step (valid if step.repeats >1)
    uint8_t  next_sequence_rep; // next sequence repetition counter (valid if sequence.repeats > 1)
    uint8_t  led_set_start; // first target led for this set
    uint8_t  led_set_end; // last target led for this set
} NotifierLedStatus_t;

static bool led_status_initialized = false;

NotifierLedStatus_t led_status[MAX_HANDLED_LED];

static void InitExtLed()
{
    memset(led_status, 0, sizeof(NotifierLedStatus_t) * MAX_HANDLED_LED);
    const uint32_t now = GET_CURRENT_MILLIS;
    for (uint8_t l = 0; l < MAX_HANDLED_LED; l++) {
        led_status[l].led_set_start = 0;
        led_status[l].led_set_end   = PIOS_WS2811_NUMLEDS - 1;
        led_status[l].next_run_time = now + 500; // start within half a second
        for (uint8_t i = 0; i < MAX_BACKGROUND_NOTIFICATIONS; i++) {
            led_status[l].queued_priorities[i] = NOTIFY_PRIORITY_BACKGROUND;
        }
    }
}

/**
 * restart current sequence
 */
static void restart_sequence(NotifierLedStatus_t *status, bool immediate)
{
    status->next_sequence_step = 0;
    status->next_step_rep = 0;
    status->step_phase_on = true;
    status->running = true;
    if (immediate) {
        uint32_t currentTime = GET_CURRENT_MILLIS;
        status->next_run_time = currentTime;
    }
    status->sequence_starting_time = status->next_run_time;
}

/**
 * modify background sequence or enqueue a new sequence to play
 */
static void push_queued_sequence(ExtLedNotification_t *new_notification, NotifierLedStatus_t *status)
{
    int8_t updated_sequence = BACKGROUND_SEQUENCE;

    if (new_notification->priority == NOTIFY_PRIORITY_BACKGROUND) {
        status->background_sequence = new_notification->sequence;
    } else {
        // a notification with priority higher than background.
        // try to enqueue it
        int8_t insert_point = MAX_BACKGROUND_NOTIFICATIONS - 1;
        int8_t first_free   = -1;
        for (int8_t i = MAX_BACKGROUND_NOTIFICATIONS - 1; i >= 0; i--) {
            const int8_t priority_i = status->queued_priorities[i];
            if (priority_i == NOTIFY_PRIORITY_BACKGROUND) {
                first_free   = i;
                insert_point = i;
                continue;
            }
            if (priority_i > new_notification->priority) {
                insert_point = ((i > 0) || (first_free > -1)) ? i : -1; // check whether priority is no bigger than lowest queued priority
            }
        }

        // no space left on queue for this new notification, ignore.
        if (insert_point < 0) {
            return;
        }
        if (insert_point != first_free) {
            // there is a free slot, move everything up one place
            if (first_free != -1) {
                for (uint8_t i = MAX_BACKGROUND_NOTIFICATIONS - 1; i > insert_point; i--) {
                    status->queued_priorities[i] = status->queued_priorities[i - 1];
                    status->queued_sequences[i]  = status->queued_sequences[i - 1];
                }
                if (status->active_sequence_num >= insert_point) {
                    status->active_sequence_num++;
                }
            } else {
                // no free space, discard lowest priority notification and move everything down
                for (uint8_t i = 0; i < insert_point; i++) {
                    status->queued_priorities[i] = status->queued_priorities[i + 1];
                    status->queued_sequences[i]  = status->queued_sequences[i + 1];
                }
                if (status->active_sequence_num <= insert_point) {
                    status->active_sequence_num--;
                }
            }
        }

        status->queued_priorities[insert_point] = new_notification->priority;
        status->queued_sequences[insert_point]  = new_notification->sequence;
        updated_sequence = insert_point;
    }

    if (status->active_sequence_num < updated_sequence) {
        status->active_sequence_num = updated_sequence;
        restart_sequence(status, true);
    }
    if (updated_sequence == BACKGROUND_SEQUENCE) {
        restart_sequence(status, false);
    }
}

static bool pop_queued_sequence(NotifierLedStatus_t *status)
{
    if (status->active_sequence_num != BACKGROUND_SEQUENCE) {
        // start the lower priority item
        status->queued_priorities[status->active_sequence_num] = NOTIFY_PRIORITY_BACKGROUND;
        status->active_sequence_num--;
        return true;
    }
    // background sequence was completed
    return false;
}

/**
 * advance current sequence pointers for next step
 */
static void advance_sequence(NotifierLedStatus_t *status)
{
    LedSequence_t *activeSequence =
        status->active_sequence_num == BACKGROUND_SEQUENCE ?
        &status->background_sequence : &status->queued_sequences[status->active_sequence_num];

    uint8_t step = status->next_sequence_step;
    LedStep_t *currentStep = &activeSequence->steps[step];

    // Next step will be the OFF phase, so just update the time and
    if (status->step_phase_on) {
        // next will be the off phase
        status->next_run_time += currentStep->time_on;
        status->step_phase_on  = false;
        // check if off phase should be skipped
        if (currentStep->time_off != 0) {
            return;
        }
    }

    // next step is ON phase. check whether to repeat current step or move to next one
    status->next_run_time += currentStep->time_off;
    status->step_phase_on  = true;

    if (status->next_step_rep + 1 < currentStep->repeats) {
        // setup next repetition
        status->next_step_rep++;
        return;
    }

    // move to next step
    LedStep_t *nextStep = (step + 1 < NOTIFY_SEQUENCE_MAX_STEPS) ? &activeSequence->steps[step + 1] : 0;

    // next step is null, check whether sequence must be repeated or it must move to lower priority queued or background sequences
    if (NOTIFY_IS_NULL_STEP(nextStep)) {
        if (activeSequence->repeats == -1 || status->next_sequence_rep + 1 < activeSequence->repeats) {
            status->next_sequence_rep++;
            // restart the sequence
            restart_sequence(status, false);
            return;
        }
        if (status->active_sequence_num != BACKGROUND_SEQUENCE) {
            // no repeat, pop enqueued or background sequences
            pop_queued_sequence(status);
            restart_sequence(status, false);
            status->next_sequence_rep = 0;
        } else {
            status->running = false;
        }
    } else {
        status->next_step_rep = 0;
        status->next_sequence_step++;
    }
}

/**
 * run a led set
 */
static void run_led(NotifierLedStatus_t *status)
{
    const uint32_t currentTime = GET_CURRENT_MILLIS;

    if (!status->running || currentTime < status->next_run_time) {
        return;
    }
    status->next_run_time = currentTime;
    uint8_t step = status->next_sequence_step;

    LedSequence_t *activeSequence = status->active_sequence_num == BACKGROUND_SEQUENCE ?
                                    &status->background_sequence : &status->queued_sequences[status->active_sequence_num];
    const Color_t color = status->step_phase_on ? activeSequence->steps[step].color : Color_Off;

    for (uint8_t i = status->led_set_start; i <= status->led_set_end; i++) {
        PIOS_WS2811_setColorRGB(color, i, false);
    }
    PIOS_WS2811_Update();
    advance_sequence(status);
}

void LedNotificationExtLedsRun()
{
    // handle incoming sequences
    if (!led_status_initialized) {
        InitExtLed();
        led_status_initialized = true;
    }
    static ExtLedNotification_t *newNotification;
    newNotification = PIOS_NOTIFY_GetNewExtLedSequence(true);
    if (newNotification) {
        push_queued_sequence(newNotification, &led_status[0]);
    }

    // Run Leds
    for (uint8_t i = 0; i < MAX_HANDLED_LED; i++) {
        run_led(&led_status[i]);
    }
}
