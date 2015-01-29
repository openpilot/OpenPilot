/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_SERVO RC Servo Functions
 * @{
 *
 * @file       pios_servo.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RC Servo functions header.
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

#ifndef PIOS_SERVO_H
#define PIOS_SERVO_H

/* Global types */
enum pios_servo_bank_mode {
    PIOS_SERVO_BANK_MODE_PWM = 0,
    PIOS_SERVO_BANK_MODE_SINGLE_PULSE = 1
};
/* Public Functions */
extern void PIOS_Servo_SetHz(const uint16_t *speeds, const uint32_t *clock, uint8_t banks);
extern void PIOS_Servo_Set(uint8_t Servo, uint16_t Position);
extern void PIOS_Servo_Update();
extern void PIOS_Servo_SetBankMode(uint8_t bank, uint8_t mode);
extern uint8_t PIOS_Servo_GetPinBank(uint8_t pin);

#endif /* PIOS_SERVO_H */

/**
 * @}
 * @}
 */
