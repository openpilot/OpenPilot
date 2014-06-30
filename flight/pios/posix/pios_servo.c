/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_SERVO RC Servo Functions
 * @brief Code to do set RC servo output
 * @{
 *
 * @file       pios_servo.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RC Servo routines (STM32 dependent)
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_SERVO)
#include "pios_i2c.h"

#define PIOS_PCA9685_SUBADR1 0x02
#define PIOS_PCA9685_SUBADR2 0x03
#define PIOS_PCA9685_SUBADR3 0x04
#define PIOS_PCA9685_MODE1 0x00
#define PIOS_PCA9685_PRESCALE 0xFE
#define PIOS_PCA9685_LED0_ON_L 0x06
#define PIOS_PCA9685_LED0_ON_H 0x07
#define PIOS_PCA9685_LED0_OFF_L 0x08
#define PIOS_PCA9685_LED0_OFF_H 0x09
#define PIOS_PCA9685_ALLLED_ON_L 0xFA
#define PIOS_PCA9685_ALLLED_ON_H 0xFB
#define PIOS_PCA9685_ALLLED_OFF_L 0xFC
#define PIOS_PCA9685_ALLLED_OFF_H 0xFD

#define PIOS_PCA9685_I2C_ADDR 0x40

/* Private Function Prototypes */

/* Local Variables */
static volatile uint16_t ServoPosition[PIOS_SERVO_NUM_TIMERS];
static uint16_t ServoFreq;

/**
 * Initialise Servos
 */
void PIOS_Servo_Init(void)
{
	struct pios_i2c_txn i2c_transaction[1];
	uint8_t i2c_buf[2];

	i2c_transaction[0].info = NULL;
	i2c_transaction[0].addr = PIOS_PCA9685_I2C_ADDR;
	i2c_transaction[0].rw = PIOS_I2C_TXN_WRITE;
	i2c_transaction[0].len = 2;
	i2c_transaction[0].buf = i2c_buf;

	i2c_buf[0] = PIOS_PCA9685_MODE1;
	i2c_buf[1] = 0x00;
	PIOS_I2C_Transfer(1, i2c_transaction, 1); // reset chip;
}

/**
 * Set the servo update rate (Max 500Hz)
 * \param[in] array of rates in Hz
 * \param[in] maximum number of banks
 */
void PIOS_Servo_SetHz(const uint16_t *banks, uint8_t num_banks)
{
	struct pios_i2c_txn i2c_transaction[2];
	double prescaleval;
	uint8_t prescale;
	uint8_t i2c_buf[2];
	uint8_t oldmode;
	uint8_t newmode;

	// PCA9685 only supports one freq per all channels
	// freq equation:
	// round(osc_clock / (4096 * freq)) -1
	// osc_clock = 25MHz
	if (banks != NULL) {
		ServoFreq = banks[0];

		prescaleval = 25000000.0; // 25MHz
    	prescaleval /= (4096.0 * ServoFreq);
	    prescaleval -= 1.0;
		prescale = floor(prescaleval + 0.5);

		i2c_transaction[0].info = NULL;
		i2c_transaction[0].addr = PIOS_PCA9685_I2C_ADDR;
		i2c_transaction[0].rw = PIOS_I2C_TXN_WRITE;
		i2c_transaction[0].len = 1;
		i2c_transaction[0].buf = i2c_buf;

		i2c_transaction[1].info = NULL;
		i2c_transaction[1].addr = PIOS_PCA9685_I2C_ADDR;
		i2c_transaction[1].rw = PIOS_I2C_TXN_READ;
		i2c_transaction[1].len = 1;
		i2c_transaction[1].buf = &oldmode;
	
		i2c_buf[0] = PIOS_PCA9685_MODE1;
		PIOS_I2C_Transfer(1, i2c_transaction, 2);
		newmode = (oldmode & 0x7F) | 0x10; // sleep
		i2c_buf[0] = PIOS_PCA9685_MODE1;
		i2c_buf[1] = newmode;
		i2c_transaction[0].len = 2;
		PIOS_I2C_Transfer(1, i2c_transaction, 1); // go to sleep

		i2c_buf[0] = PIOS_PCA9685_PRESCALE;
		i2c_buf[1] = prescale;
		PIOS_I2C_Transfer(1, i2c_transaction, 1);

		i2c_buf[0] = PIOS_PCA9685_MODE1;
		i2c_buf[1] = oldmode;
		PIOS_I2C_Transfer(1, i2c_transaction, 1);
		PIOS_DELAY_WaituS(5);

		i2c_buf[0] = PIOS_PCA9685_MODE1;
		i2c_buf[1] = oldmode | 0x80;
		PIOS_I2C_Transfer(1, i2c_transaction, 1);
	}
}

/**
 * Set servo position
 * \param[in] Servo Servo number (0-7)
 * \param[in] Position Servo position in microseconds
 */
// miliseconds or microseconds thats the question
void PIOS_Servo_Set(uint8_t Servo, uint16_t Position)
{
	struct pios_i2c_txn i2c_transaction[4];
	uint8_t i2c_buf[8];
	double pulse_duration;

#ifndef PIOS_ENABLE_DEBUG_PINS
    /* Make sure servo exists */
    if (Servo < PIOS_SERVO_NUM_OUTPUTS) {
        /* Update the position */
        ServoPosition[Servo] = Position;
    }
#endif // PIOS_ENABLE_DEBUG_PINS

	pulse_duration = 1000000.0; // us in second;
	pulse_duration /= (ServoFreq * 1.09); // there is around 9% error on frequency;
	pulse_duration /= 4096; // 12 bits of resolution;
	pulse_duration = Position / pulse_duration;
	Position = pulse_duration;

	i2c_buf[0] = PIOS_PCA9685_LED0_ON_L + (4 * Servo);
	i2c_buf[1] = 0x00;
	i2c_buf[2] = PIOS_PCA9685_LED0_ON_H + (4 * Servo);
	i2c_buf[3] = 0x00;
	i2c_buf[4] = PIOS_PCA9685_LED0_OFF_L + (4 * Servo);
	i2c_buf[5] = Position & 0xFF;
	i2c_buf[6] = PIOS_PCA9685_LED0_OFF_H + (4 * Servo);
	i2c_buf[7] = Position >> 8;

	i2c_transaction[0].info = NULL;
	i2c_transaction[0].addr = PIOS_PCA9685_I2C_ADDR;
	i2c_transaction[0].rw = PIOS_I2C_TXN_WRITE;
	i2c_transaction[0].len = 2;
	i2c_transaction[0].buf = &i2c_buf[0];
	i2c_transaction[1].info = NULL;
	i2c_transaction[1].addr = PIOS_PCA9685_I2C_ADDR;
	i2c_transaction[1].rw = PIOS_I2C_TXN_WRITE;
	i2c_transaction[1].len = 2;
	i2c_transaction[1].buf = &i2c_buf[2];
	i2c_transaction[2].info = NULL;
	i2c_transaction[2].addr = PIOS_PCA9685_I2C_ADDR;
	i2c_transaction[2].rw = PIOS_I2C_TXN_WRITE;
	i2c_transaction[2].len = 2;
	i2c_transaction[2].buf = &i2c_buf[4];
	i2c_transaction[3].info = NULL;
	i2c_transaction[3].addr = PIOS_PCA9685_I2C_ADDR;
	i2c_transaction[3].rw = PIOS_I2C_TXN_WRITE;
	i2c_transaction[3].len = 2;
	i2c_transaction[3].buf = &i2c_buf[6];

	PIOS_I2C_Transfer(1, i2c_transaction, 4);
}

#endif /* if defined(PIOS_INCLUDE_SERVO) */
