#include "gtest/gtest.h"

#include <stdio.h> /* printf */
#include <stdlib.h> /* abort */
#include <string.h> /* memset */

extern "C" {
#define xTaskGetTickCount() 1
#define portTICK_RATE_MS 1

#include "lednotification.c"

void PIOS_WS2811_setColorRGB(__attribute__((unused)) Color_t c, __attribute__((unused)) uint8_t led, __attribute__((unused)) bool update){

}
void PIOS_WS2811_Update(){

}
}

class LedNotificationTest : public testing::Test {};

TEST_F(LedNotificationTest, TestQueueOrder1) {
    NotifierLedStatus_t status;

    for (uint8_t i = 0; i < MAX_BACKGROUND_NOTIFICATIONS; i++) {
        status.queued_priorities[i] = NOTIFY_PRIORITY_BACKGROUND;
    }


    ExtLedNotification_t notification0;
    notification0.priority = NOTIFY_PRIORITY_LOW;
    push_queued_sequence(&notification0, &status);
    ExtLedNotification_t notification1;
    notification1.priority = NOTIFY_PRIORITY_CRITICAL;
    push_queued_sequence(&notification1, &status);
    ExtLedNotification_t notification2;
    notification2.priority = NOTIFY_PRIORITY_LOW;
    push_queued_sequence(&notification2, &status);
    ExtLedNotification_t notification3;
    notification3.priority = NOTIFY_PRIORITY_CRITICAL;
    push_queued_sequence(&notification3, &status);

    EXPECT_EQ(NOTIFY_PRIORITY_LOW, status.queued_priorities[0]);
    EXPECT_EQ(NOTIFY_PRIORITY_LOW, status.queued_priorities[1]);
    EXPECT_EQ(NOTIFY_PRIORITY_CRITICAL, status.queued_priorities[2]);
    EXPECT_EQ(NOTIFY_PRIORITY_CRITICAL, status.queued_priorities[3]);
    EXPECT_EQ(NOTIFY_PRIORITY_BACKGROUND, status.queued_priorities[4]);

}

TEST_F(LedNotificationTest, TestQueueOrder2) {
    NotifierLedStatus_t status;

// Fails because insert_point and first_point will both be -1. This will also cause an array-out-of bounds at:
// 146            status->queued_priorities[insert_point] = new_notification->priority;
// 147            status->queued_sequences[insert_point]  = new_notification->sequence;
// 148            updated_sequence = insert_point;

    for (uint8_t i = 0; i < MAX_BACKGROUND_NOTIFICATIONS; i++) {
        status.queued_priorities[i] = NOTIFY_PRIORITY_LOW;
    }

    ExtLedNotification_t notification;
    notification.priority = NOTIFY_PRIORITY_REGULAR;
    push_queued_sequence(&notification, &status);

    EXPECT_EQ(NOTIFY_PRIORITY_REGULAR, status.queued_priorities[4]);
    EXPECT_EQ(NOTIFY_PRIORITY_LOW, status.queued_priorities[3]);
    EXPECT_EQ(NOTIFY_PRIORITY_LOW, status.queued_priorities[2]);
    EXPECT_EQ(NOTIFY_PRIORITY_LOW, status.queued_priorities[1]);
    EXPECT_EQ(NOTIFY_PRIORITY_LOW, status.queued_priorities[0]);
}

TEST_F(LedNotificationTest, TestQueueOrder3) {
    NotifierLedStatus_t status;

    // Fails because queued_priorities[0] _LOW and not _REGULAR. I _think_ this is a bug.
    for (uint8_t i = 0; i < MAX_BACKGROUND_NOTIFICATIONS; i++) {
        status.queued_priorities[i] = NOTIFY_PRIORITY_REGULAR;
    }

    ExtLedNotification_t notification;
    notification.priority = NOTIFY_PRIORITY_LOW;
    push_queued_sequence(&notification, &status);

    for (uint8_t i = 0; i < MAX_BACKGROUND_NOTIFICATIONS; i++) {
        EXPECT_EQ(NOTIFY_PRIORITY_REGULAR, status.queued_priorities[i]);
    }
}
