/**
 ******************************************************************************
 *
 * @file       notification.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      notification library.
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
#include "inc/notification.h"
#include <openpilot.h>
#include <pios_struct_helper.h>
#include <systemalarms.h>
#include <flightstatus.h>
#include <pios_notify.h>
#include <stdbool.h>

#ifdef PIOS_INCLUDE_WS2811
#include <pios_ws2811.h>
#endif
// Private defines
// Maximum number of notifications enqueued when a higher priority notification is added
#define MAX_BACKGROUND_NOTIFICATIONS 3
#define MAX_HANDLED_LED              1

#define BACKGROUND_SEQUENCE          -1
#define RESET_STEP                   -1
#define GET_CURRENT_MILLIS           (xTaskGetTickCount() * portTICK_RATE_MS)
// Private data types definition

// this is the status for a single LED notification led set
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
    uint8_t  led_set; // target led set
} NotifierLedStatus_t;

#ifdef PIOS_INCLUDE_WS2811
static bool led_status_initialized = false;
#endif

#ifdef PIOS_LED_ALARM
#define ALARM_LED_ON()  PIOS_LED_On(PIOS_LED_ALARM)
#define ALARM_LED_OFF() PIOS_LED_Off(PIOS_LED_ALARM)
#else
#define ALARM_LED_ON()
#define ALARM_LED_OFF()
#endif

#ifdef PIOS_LED_HEARTBEAT
#define HEARTBEAT_LED_ON()  PIOS_LED_On(PIOS_LED_HEARTBEAT)
#define HEARTBEAT_LED_OFF() PIOS_LED_Off(PIOS_LED_HEARTBEAT)
#else
#define HEARTBEAT_LED_ON()
#define HEARTBEAT_LED_OFF()
#endif

#define BLINK_R_ALARM_PATTERN(x) \
    (x == SYSTEMALARMS_ALARM_OK ? 0 : \
     x == SYSTEMALARMS_ALARM_WARNING ? 0b0000000001000000 : \
     x == SYSTEMALARMS_ALARM_ERROR ? 0b0000001000100000 : \
     x == SYSTEMALARMS_ALARM_CRITICAL ? 0b0111111111111110 : 0)
#define BLINK_B_ALARM_PATTERN(x) \
    (x == SYSTEMALARMS_ALARM_OK ? 0 : \
     x == SYSTEMALARMS_ALARM_WARNING ? 0 : \
     x == SYSTEMALARMS_ALARM_ERROR ? 0 : \
     x == SYSTEMALARMS_ALARM_CRITICAL ? 0 : 0)


#define BLINK_B_FM_ARMED_PATTERN(x) \
    (x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED1 ? 0b0000000000000001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED2 ? 0b0000000000100001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED3 ? 0b0000010000100001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED4 ? 0b0000000000000001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED5 ? 0b0000000000100001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED6 ? 0b0000010000100001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ? 0b0000010000100001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE ? 0b0001000100010001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_LAND ? 0b0001000100010001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER ? 0b0000010000100001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POI ? 0b0000010000100001 : 0b0000000000000001)

#define BLINK_R_FM_ARMED_PATTERN(x) \
    (x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED1 ? 0b0000000000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED2 ? 0b0000000000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED3 ? 0b0000000000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED4 ? 0b0000000000000001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED5 ? 0b0000000000000001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED6 ? 0b0000000000000001 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ? 0b0000010000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE ? 0b0001000100000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_LAND ? 0b0001000100000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER ? 0b0000010000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POI ? 0b0000010000000000 : 0b0000010000000000)

#define BLINK_B_FM_DISARMED_PATTERN(x) \
    (x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED1 ? 0b0000000000000011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED2 ? 0b0000000001100011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED3 ? 0b0000110001100011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED4 ? 0b0000000000000011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED5 ? 0b0000000001100011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED6 ? 0b0000110001100011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ? 0b0000110001100011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE ? 0b0011001100110011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_LAND ? 0b0011001100110011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER ? 0b0000110001100011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POI ? 0b0000110001100011 : 0b0000000000000011)

#define BLINK_R_FM_DISARMED_PATTERN(x) \
    (x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED1 ? 0b0000000000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED2 ? 0b0000000000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED3 ? 0b0000000000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED4 ? 0b0000000000000011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED5 ? 0b0000000000000011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_STABILIZED6 ? 0b0000000000000011 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD ? 0b0000110000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE ? 0b0011001100000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_LAND ? 0b0011001100000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER ? 0b0000110000000000 : \
     x == FLIGHTSTATUS_FLIGHTMODE_POI ? 0b0000110000000000 : 0b0000110000000000)

#define BLINK_B_HEARTBEAT_PATTERN 0b0001111111111111
#define BLINK_R_HEARTBEAT_PATTERN 0

#define BLINK_B_NOTIFY_PATTERN(x) \
    (x == NOTIFY_NONE ? 0 : \
     x == NOTIFY_OK ? 0b0000100100111111 : \
     x == NOTIFY_NOK ? 0b0000000000111111 : \
     x == NOTIFY_DRAW_ATTENTION ? 0b0101010101010101 : 0b0101010101010101)

#define BLINK_R_NOTIFY_PATTERN(x) \
    (x == NOTIFY_NONE ? 0 : \
     x == NOTIFY_OK ? 0b0000000000001111 : \
     x == NOTIFY_NOK ? 0b0011000011001111 : \
     x == NOTIFY_DRAW_ATTENTION ? 0b1010101010101010 : 0b1010101010101010)

// led notification handling
static volatile SystemAlarmsAlarmOptions currentAlarmLevel = SYSTEMALARMS_ALARM_OK;
static volatile FlightStatusData currentFlightStatus;
static volatile bool started = false;
static volatile pios_notify_notification nextNotification = NOTIFY_NONE;

#ifdef PIOS_LED_ALARM
static bool handleAlarms(uint16_t *r_pattern, uint16_t *b_pattern);
#endif // PIOS_LED_ALARM
static bool handleNotifications(pios_notify_notification runningNotification, uint16_t *r_pattern, uint16_t *b_pattern);
static void handleFlightMode(uint16_t *r_pattern, uint16_t *b_pattern);
static void handleHeartbeat(uint16_t *r_pattern, uint16_t *b_pattern);

static void HandleExtLeds();

void NotificationUpdateStatus()
{
    started = true;
    // get values to be used for led handling
    FlightStatusGet((FlightStatusData *)&currentFlightStatus);
    currentAlarmLevel = AlarmsGetHighestSeverity();
    if (nextNotification == NOTIFY_NONE) {
        nextNotification = PIOS_NOTIFY_GetActiveNotification(true);
    }
}

void NotificationOnboardLedsRun()
{
    static uint32_t lastRunTimestamp;
    static uint16_t r_pattern;
    static uint16_t b_pattern;
    static uint8_t cycleCount; // count the number of cycles
    static uint8_t lastFlightMode   = -1;
    static bool forceShowFlightMode = false;
    static pios_notify_notification runningNotification = NOTIFY_NONE;
    static enum {
        STATUS_NOTIFY,
        STATUS_ALARM,
        STATUS_FLIGHTMODE, // flightMode/HeartBeat
        STATUS_LENGHT
    } status;

    HandleExtLeds();
    const uint32_t current_timestamp = GET_CURRENT_MILLIS;
    if (!started || (current_timestamp - lastRunTimestamp) < LED_BLINK_PERIOD_MS) {
        return;
    }

    lastRunTimestamp = current_timestamp;
    // the led will show various status information, subdivided in three phases
    // - Notification
    // - Alarm
    // - Flight status
    // they are shown using the above priority
    // a phase last exactly 8 cycles (so bit 1<<4 is used to determine if a phase end

    cycleCount++;
    // Notifications are "modal" to other statuses so they takes precedence
    if (status != STATUS_NOTIFY && nextNotification != NOTIFY_NONE) {
        // read next notification to show
        runningNotification = nextNotification;
        nextNotification    = NOTIFY_NONE;
        // Force a notification status
        status     = STATUS_NOTIFY;
        cycleCount = 0; // instantly start a notify cycle
    } else {
        if (lastFlightMode != currentFlightStatus.FlightMode) {
            status         = STATUS_FLIGHTMODE;
            lastFlightMode = currentFlightStatus.FlightMode;
            cycleCount     = 0; // instantly start a flightMode cycle
            forceShowFlightMode = true;
        }
    }

    // check if a phase has just finished
    if (cycleCount & 0x10) {
        HEARTBEAT_LED_OFF();
        ALARM_LED_OFF();
        cycleCount = 0x0;
        forceShowFlightMode = false;
        // Notification has been just shown, cleanup
        if (status == STATUS_NOTIFY) {
            runningNotification = NOTIFY_NONE;
        }
        status = (status + 1) % STATUS_LENGHT;
    }

    if (status == STATUS_NOTIFY) {
        if (!cycleCount && !handleNotifications(runningNotification, &r_pattern, &b_pattern)) {
            // no notifications, advance
            status++;
        }
    }

    // Handles Alarm display
    if (status == STATUS_ALARM) {
#ifdef PIOS_LED_ALARM
        if (!cycleCount && !handleAlarms(&r_pattern, &b_pattern)) {
            // no alarms, advance
            status++;
        }
#else
        // no alarms leds, advance
        status++;
#endif // PIOS_LED_ALARM
    }

    // **** Handles flightmode display
    if (status == STATUS_FLIGHTMODE && !cycleCount) {
        if (forceShowFlightMode || currentFlightStatus.Armed != FLIGHTSTATUS_ARMED_DISARMED) {
            handleFlightMode(&r_pattern, &b_pattern);
        } else {
            handleHeartbeat(&r_pattern, &b_pattern);
        }
    }

    // led output
    if (b_pattern & 0x1) {
        HEARTBEAT_LED_ON();
    } else {
        HEARTBEAT_LED_OFF();
    }
    if (r_pattern & 0x1) {
        ALARM_LED_ON();
    } else {
        ALARM_LED_OFF();
    }
    r_pattern >>= 1;
    b_pattern >>= 1;
}

#if defined(PIOS_LED_ALARM)
static bool handleAlarms(uint16_t *r_pattern, uint16_t *b_pattern)
{
    if (currentAlarmLevel == SYSTEMALARMS_ALARM_OK) {
        return false;
    }
    *b_pattern = BLINK_B_ALARM_PATTERN(currentAlarmLevel);
    *r_pattern = BLINK_R_ALARM_PATTERN(currentAlarmLevel);
    return true;
}
#endif /* PIOS_LED_ALARM */


static bool handleNotifications(pios_notify_notification runningNotification, uint16_t *r_pattern, uint16_t *b_pattern)
{
    if (runningNotification == NOTIFY_NONE) {
        return false;
    }
    *b_pattern = BLINK_B_NOTIFY_PATTERN(runningNotification);
    *r_pattern = BLINK_R_NOTIFY_PATTERN(runningNotification);
    return true;
}

static void handleFlightMode(uint16_t *r_pattern, uint16_t *b_pattern)
{
    // Flash the heartbeat LED
    uint8_t flightmode = currentFlightStatus.FlightMode;

    if (currentFlightStatus.Armed == FLIGHTSTATUS_ARMED_DISARMED) {
        *b_pattern = BLINK_B_FM_DISARMED_PATTERN(flightmode);
        *r_pattern = BLINK_R_FM_DISARMED_PATTERN(flightmode);
    } else {
        *b_pattern = BLINK_B_FM_ARMED_PATTERN(flightmode);
        *r_pattern = BLINK_R_FM_ARMED_PATTERN(flightmode);
    }
}

static void handleHeartbeat(uint16_t *r_pattern, uint16_t *b_pattern)
{
    // Flash the heartbeat LED
    *b_pattern = BLINK_B_HEARTBEAT_PATTERN;
    *r_pattern = BLINK_R_HEARTBEAT_PATTERN;
}

#ifdef PIOS_INCLUDE_WS2811

NotifierLedStatus_t led_status[MAX_HANDLED_LED];

static void InitExtLed()
{
    memset(led_status, 0, sizeof(NotifierLedStatus_t) * MAX_HANDLED_LED);
    const uint32_t now = GET_CURRENT_MILLIS;
    for (uint8_t l = 0; l < MAX_HANDLED_LED; l++) {
        led_status[l].led_set = l;
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
        int8_t insert_point = -1;
        int8_t first_free   = -1;
        for (int8_t i = MAX_BACKGROUND_NOTIFICATIONS; i > -1; i--) {
            const int8_t priority_i = status->queued_priorities[i];
            if (priority_i == NOTIFY_PRIORITY_BACKGROUND) {
                first_free   = i;
                insert_point = i;
                continue;
            }
            if (priority_i > new_notification->priority) {
                insert_point = i;
            }
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
    uint32_t currentTime = GET_CURRENT_MILLIS;

    if (!status->running || currentTime < status->next_run_time) {
        return;
    }
    status->next_run_time = currentTime;
    uint8_t step = status->next_sequence_step;

    LedSequence_t *activeSequence = status->active_sequence_num == BACKGROUND_SEQUENCE ?
                                    &status->background_sequence : &status->queued_sequences[status->active_sequence_num];
    if (status->step_phase_on) {
        PIOS_WS2811_setColorRGB(activeSequence->steps[step].color, status->led_set, true);
    } else {
        PIOS_WS2811_setColorRGB(Color_Off, status->led_set, true);
    }
    advance_sequence(status);
}

void HandleExtLeds()
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
#else /* ifdef PIOS_INCLUDE_WS2811 */
void HandleExtLeds() {}
#endif /* ifdef PIOS_INCLUDE_WS2811 */
