/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_PWM PWM Input Functions
 * @brief       Code to measure with PWM input
 * @{
 *
 * @file       pios_pwm.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PWM Input functions (STM32 dependent)
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

#include "pios.h"

#ifdef PIOS_INCLUDE_PWM

#include <pthread.h>

static pthread_mutex_t pwm_mutex = PTHREAD_MUTEX_INITIALIZER;


/* Provide a RCVR driver */
static int32_t PIOS_PWM_Get(uint32_t rcvr_id, uint8_t channel);

const struct pios_rcvr_driver pios_pwm_rcvr_driver = {
    .read = PIOS_PWM_Get,
};

enum pios_pwm_dev_magic {
    PIOS_PWM_DEV_MAGIC = 0xab30293c,
};

struct pios_pwm_dev {
    enum pios_pwm_dev_magic   magic;
    uint32_t CaptureValue[PIOS_PWM_NUM_INPUTS];
};

static bool PIOS_PWM_validate(struct pios_pwm_dev *pwm_dev)
{
    return pwm_dev->magic == PIOS_PWM_DEV_MAGIC;
}

static struct pios_pwm_dev pios_pwm_devs[PIOS_PWM_MAX_DEVS];
static uint8_t pios_pwm_num_devs = 0;
static struct pios_pwm_dev *PIOS_PWM_alloc(void)
{
    struct pios_pwm_dev *pwm_dev;

    if (pios_pwm_num_devs >= PIOS_PWM_MAX_DEVS) {
        return NULL;
    }

    pwm_dev = &pios_pwm_devs[pios_pwm_num_devs++];
    pwm_dev->magic = PIOS_PWM_DEV_MAGIC;

    return pwm_dev;
}

/**
 * Initialises all the pins
 */
int32_t PIOS_PWM_Init(uint32_t *pwm_id)
{
    PIOS_DEBUG_Assert(pwm_id);

    struct pios_pwm_dev *pwm_dev;

    pwm_dev = (struct pios_pwm_dev *)PIOS_PWM_alloc();
    if (!pwm_dev) {
        goto out_fail;
    }

    /* Do not need synchronization yet */
    for (uint8_t i = 0; i < PIOS_PWM_NUM_INPUTS; i++) {
        pwm_dev->CaptureValue[i] = PIOS_RCVR_TIMEOUT;
    }

    *pwm_id = (uint32_t)pwm_dev;

    return 0;

out_fail:
    return -1;
}

/**
 * Get the value of an input channel
 * \param[in] channel Number of the channel desired (zero based)
 * \output PIOS_RCVR_INVALID channel not available
 * \output PIOS_RCVR_TIMEOUT failsafe condition or missing receiver
 * \output >=0 channel value
 */
static int32_t PIOS_PWM_Get(uint32_t rcvr_id, uint8_t channel)
{
    struct pios_pwm_dev *pwm_dev = (struct pios_pwm_dev *)rcvr_id;
    int32_t ret;

    if (!PIOS_PWM_validate(pwm_dev)) {
        /* Invalid device specified */
        return PIOS_RCVR_INVALID;
    }

    if (channel >= PIOS_PWM_NUM_INPUTS) {
        /* Channel out of range */
        return PIOS_RCVR_INVALID;
    }

    pthread_mutex_lock(&pwm_mutex);
    ret = pwm_dev->CaptureValue[channel];
    pthread_mutex_unlock(&pwm_mutex);

    return ret;
}

//-------------------------------------------------------------------------------
#if 0
   Logitech Extreme 3D Pro Joystick

          Configuration data
0x38 0x66 0x0B 0x00 0x00 0x00 0x81 0x00
0x39 0x66 0x0B 0x00 0x00 0x00 0x81 0x01
0x39 0x66 0x0B 0x00 0x00 0x00 0x81 0x02
0x39 0x66 0x0B 0x00 0x00 0x00 0x81 0x03
0x39 0x66 0x0B 0x00 0x00 0x00 0x81 0x04
0x3A 0x66 0x0B 0x00 0x00 0x00 0x81 0x05
0x3A 0x66 0x0B 0x00 0x00 0x00 0x81 0x06
0x3A 0x66 0x0B 0x00 0x00 0x00 0x81 0x07
0x3A 0x66 0x0B 0x00 0x00 0x00 0x81 0x08
0x3A 0x66 0x0B 0x00 0x00 0x00 0x81 0x09
0x3B 0x66 0x0B 0x00 0x00 0x00 0x81 0x0A
0x3B 0x66 0x0B 0x00 0x00 0x00 0x81 0x0B
0x3B 0x66 0x0B 0x00 0x00 0x00 0x82 0x00
0x3B 0x66 0x0B 0x00 0x00 0x00 0x82 0x01
0x3C 0x66 0x0B 0x00 0x00 0x00 0x82 0x02
0x3C 0x66 0x0B 0x00 0xFF 0x7F 0x82 0x03
0x3C 0x66 0x0B 0x00 0x00 0x00 0x82 0x04
0x3C 0x66 0x0B 0x00 0x00 0x00 0x82 0x05 



data[0] data[1] data[2] data[3] --> some kind of counter ???

  LSB     MSB
data[4] data[5] --> Button / Axis value

data[6] --> Button / Axis flag.
    0x01 = Button
    0x02 = Axis
    Byte 8 of this flag indicate configuration data (e.g. first readout / initial values)

data[7] --> Button Axis identyfication
    Buttons: 0x00 = Fire Button 1
             0x01 = Button 2 --> Under Thumb
             0x02 = Button 3 --> Under Thumb
             0x03 = Button 4 --> Under Thumb
             0x04 = Button 5 --> Under Thumb
             0x05 = Button 6 --> Under Thumb
             0x06 = Button 7
             0x07 = Button 8
             0x08 = Button 9
             0x09 = Button 10
             0x0A = Button 11
             0x0B = Button 12

       Axis: 0x00 = Left / Right (X) Axis
             0x01 = Push / Pull  (Y) Axis
             0x02 = Rotate Axis
             0x03 = Throttle Axis
             0x04 = Thumb Joystick, Left / Right (X) Axis
             0x05 = Thumb Joystick, Push / Pull  (Y) Axis
#endif

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>

#define USB_JOYSTICK_FILE       "/dev/input/js0"
#define USB_JOYSTICK_MSG_LEN    8
#define USB_JOYSTICK_BUTTON     0x01
#define USB_JOYSTICK_AXIS       0x02

#define STACK_SIZE_BYTES     2048
#define TASK_PRIORITY        (tskIDLE_PRIORITY + 4) // device driver

static xTaskHandle taskHandle;
int fd;

static void usbJoyTask(void *parameters);

int32_t UsbJoyStart()
{
    /* can't open joystick file
     * no point to have task */
    if (fd < 0) {
        return 0;
    }

    xTaskCreate(usbJoyTask, "Extreme_3D_Pro_Joy", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &taskHandle);

    //PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_???, taskHandle);
#ifdef PIOS_INCLUDE_WDG
//    PIOS_WDG_RegisterFlag(PIOS_WDG_???);
#endif

    return 0;
}

/**
 * @brief Logitech Extreme 3D Pro Joystick initialization
 * @return 0
 */
int32_t UsbJoyInitialize()
{
    fd = -1;
    fd = open(USB_JOYSTICK_FILE, O_RDONLY);
    if (fd < 0) {
        // ???
        //AlarmsSet(SYSTEMALARMS_ALARM_RECEIVER, SYSTEMALARMS_ALARM_CRITICAL);
        return 0;
    }

    // switch to non-blocking I/0
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

    // ???
    //AlarmsClear(SYSTEMALARMS_ALARM_RECEIVER);
    return 0;
}

MODULE_INITCALL(UsbJoyInitialize, UsbJoyStart);

/**
 * @brief Main USB Joystick driver task
 *
 * Logitech Extreme 3D Pro Joystick driver.
 */
static void usbJoyTask(__attribute__((unused)) void *parameters)
{
    //struct pollfd pfds;
    uint8_t data[8];
    int32_t capture_value;
    int ret;

    while (1) {
        /* smallest amount of data that we can read from this device is 8 bytes (USB_JOYSTICK_MSG_LEN)
         * if you try read less than this then you will get EINVAL error
         * as we already read smallest portion of data, there is no possibility for partial read
         * i.e. we can't read less than 8 bytes (joystick message) */
        ret = read(fd, data, USB_JOYSTICK_MSG_LEN);

        if (0 == ret) {
            // End of file
            break;
        }

        if (-1 == ret) {
            if (EAGAIN != errno) {
                // unidentified error
                // should report error here and keep going
            }

            //pfds.fd = fd;
            //pfds.events = POLLIN;
            //ret = poll(&pfds, 1, -1);
            //if (-1 == ret) {
                //cout << "Error in poll: " << strerror(errno) << '.' << endl;
            //}

            // poll cause problems because FreeRTOS port
            // but it's proper solution for Linux
            vTaskDelay(20);
            continue;
        }

        capture_value = (int16_t)((data[5] << 8) | data[4]);
/*  OpenPilot PWM mapping
    PWM[0] = 0x03 Throttle Axis              | Throttle
    PWM[1] = 0x02 Rotate Axis                | Yaw
    PWM[2] = 0x01 Push / Pull  (Y) Axis      | Pitch
    PWM[3] = 0x00 Left / Right (X) Axis      | Roll
    PWM[4] = 0x07 Button 8
    PWM[5] = 0x06 Button 7
    PWM[6] = 0x08 Button 9
    PWM[7] = 0x0A = Button 11 */

        if (data[6] & USB_JOYSTICK_AXIS) {
            /*
             * Jostick raw data [-32767, 32767]
             * we are converting to our PWM range [0, 2000]
             *
             *          new_range * (raw_val - raw_min)
             * f(x) = ----------------------------------- + new_min
             *                   raw_range
             * new_range = 2000
             * raw_range = 65534
             * new_min = 0
             * raw_min = -32767
             */
            double tmp;

            tmp = (2000.0 * (capture_value + 32767)) / 65534.0;
            capture_value = floor(tmp + 0.5);

            pthread_mutex_lock(&pwm_mutex);
            switch (data[7])
            {
                // USB_JOYSTICK is statically mapped to first (and only) PWM device
                case 0x00:
                    pios_pwm_devs[0].CaptureValue[3] = capture_value;
                    break;
                case 0x01:
                    pios_pwm_devs[0].CaptureValue[2] = capture_value;
                    break;
                case 0x02:
                    pios_pwm_devs[0].CaptureValue[1] = capture_value;
                    break;
                case 0x03:
                    pios_pwm_devs[0].CaptureValue[0] = capture_value;
                    break;
            }
            pthread_mutex_unlock(&pwm_mutex);
        }

        if (data[6] & USB_JOYSTICK_BUTTON) {
            pthread_mutex_lock(&pwm_mutex);
            switch (data[7])
            {
                // USB_JOYSTICK is statically mapped to first (and only) PWM device
                case 0x07:
                    pios_pwm_devs[0].CaptureValue[4] = (capture_value == 1) ? 2000 : 0;
                    break;
                case 0x06:
                    pios_pwm_devs[0].CaptureValue[5] = (capture_value == 1) ? 2000 : 0;
                    break;
                case 0x08:
                    pios_pwm_devs[0].CaptureValue[6] = (capture_value == 1) ? 2000 : 0;
                    break;
                case 0x0A:
                    pios_pwm_devs[0].CaptureValue[7] = (capture_value == 1) ? 2000 : 0;
                    break;
            }
            pthread_mutex_unlock(&pwm_mutex);
        }
    }
}

#endif /* PIOS_INCLUDE_PWM */

/**
 * @}
 * @}
 */
