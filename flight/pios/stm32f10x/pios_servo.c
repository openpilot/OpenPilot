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

#include "pios.h"

#ifdef PIOS_INCLUDE_SERVO

#include "pios_servo_priv.h"
#include "pios_tim_priv.h"

/* Private Function Prototypes */

static const struct pios_servo_cfg *servo_cfg;

// determine if the related timer will work in synchronous (or OneShot/OneShot125) One Pulse mode.
// static uint8_t pios_servo_bank_mode[PIOS_SERVO_BANKS] = { 0 };

// timer associated to each bank
static TIM_TypeDef *pios_servo_bank_timer[PIOS_SERVO_BANKS] = { 0 };

// index of bank used for each pin
static uint8_t *pios_servo_pin_bank;

#define PIOS_SERVO_TIMER_CLOCK 1000000

/**
 * Initialise Servos
 */
int32_t PIOS_Servo_Init(const struct pios_servo_cfg *cfg)
{
    uint32_t tim_id;

    if (PIOS_TIM_InitChannels(&tim_id, cfg->channels, cfg->num_channels, NULL, 0)) {
        return -1;
    }

    /* Store away the requested configuration */
    servo_cfg = cfg;
    pios_servo_pin_bank = pios_malloc(sizeof(uint8_t) * cfg->num_channels);

    uint8_t bank = 0;
    /* Configure the channels to be in output compare mode */
    for (uint8_t i = 0; i < cfg->num_channels; i++) {
        const struct pios_tim_channel *chan = &servo_cfg->channels[i];
        bool new = true;
        /* See if any previous channels use that same timer */
        for (uint8_t j = 0; (j < i) && new; j++) {
            new &= chan->timer != servo_cfg->channels[j].timer;
        }

        if (new) {
            PIOS_Assert(bank < PIOS_SERVO_BANKS);
            for (uint8_t j = i; j < servo_cfg->num_channels; j++) {
                if (servo_cfg->channels[j].timer == chan->timer) {
                    pios_servo_pin_bank[j] = bank;
                }
            }
            pios_servo_bank_timer[bank] = chan->timer;

            PIOS_Assert(bank < PIOS_SERVO_BANKS);

            for (uint8_t j = i; j < servo_cfg->num_channels; j++) {
                if (servo_cfg->channels[j].timer == chan->timer) {
                    pios_servo_pin_bank[j] = bank;
                }
            }
            bank++;
        }

        /* Set up for output compare function */
        switch (chan->timer_chan) {
        case TIM_Channel_1:
            TIM_OC1Init(chan->timer, &cfg->tim_oc_init);
            TIM_OC1PreloadConfig(chan->timer, TIM_OCPreload_Enable);
            break;
        case TIM_Channel_2:
            TIM_OC2Init(chan->timer, &cfg->tim_oc_init);
            TIM_OC2PreloadConfig(chan->timer, TIM_OCPreload_Enable);
            break;
        case TIM_Channel_3:
            TIM_OC3Init(chan->timer, &cfg->tim_oc_init);
            TIM_OC3PreloadConfig(chan->timer, TIM_OCPreload_Enable);
            break;
        case TIM_Channel_4:
            TIM_OC4Init(chan->timer, &cfg->tim_oc_init);
            TIM_OC4PreloadConfig(chan->timer, TIM_OCPreload_Enable);
            break;
        }

        TIM_ARRPreloadConfig(chan->timer, ENABLE);
        TIM_CtrlPWMOutputs(chan->timer, ENABLE);
        TIM_Cmd(chan->timer, ENABLE);
    }

    return 0;
}

/**
 * Set the servo update rate (Max 500Hz)
 * \param[in] array of rates in Hz
 * \param[in] array of timer clocks in Hz
 * \param[in] maximum number of banks
 */
void PIOS_Servo_SetHz(const uint16_t *speeds, const uint32_t *clock, uint8_t banks)
{
    PIOS_Assert(banks <= PIOS_SERVO_BANKS);
    if (!servo_cfg) {
        return;
    }

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = servo_cfg->tim_base_init;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;

    for (uint8_t i = 0; i < banks && i < PIOS_SERVO_BANKS; i++) {
        const TIM_TypeDef *timer = pios_servo_bank_timer[i];
        if (timer) {
            uint32_t new_clock = PIOS_SERVO_TIMER_CLOCK;
            if (clock[i]) {
                new_clock = clock[i];
            }
            TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / new_clock) - 1;
            TIM_TimeBaseStructure.TIM_Period    = ((new_clock / speeds[i]) - 1);

            TIM_TimeBaseInit((TIM_TypeDef *)timer, &TIM_TimeBaseStructure);
        }
    }
}

/**
 * Set servo position
 * \param[in] Servo Servo number (0-7)
 * \param[in] Position Servo position in microseconds
 */
void PIOS_Servo_Set(uint8_t servo, uint16_t position)
{
    /* Make sure servo exists */
    if (!servo_cfg || servo >= servo_cfg->num_channels) {
        return;
    }

    /* Update the position */
    const struct pios_tim_channel *chan = &servo_cfg->channels[servo];
    switch (chan->timer_chan) {
    case TIM_Channel_1:
        TIM_SetCompare1(chan->timer, position);
        break;
    case TIM_Channel_2:
        TIM_SetCompare2(chan->timer, position);
        break;
    case TIM_Channel_3:
        TIM_SetCompare3(chan->timer, position);
        break;
    case TIM_Channel_4:
        TIM_SetCompare4(chan->timer, position);
        break;
    }
}

void PIOS_Servo_Update()
{
    /*
       for (uint8_t i = 0; (i < PIOS_SERVO_BANKS); i++) {
        const TIM_TypeDef *timer = pios_servo_bank_timer[i];
        if (timer) {
            TIM_Cmd((TIM_TypeDef *)timer, ENABLE);
        }
       }
     */
}

void PIOS_Servo_SetBankMode(__attribute__((unused)) uint8_t bank, __attribute__((unused)) uint8_t mode) {}

uint8_t PIOS_Servo_GetPinBank(uint8_t pin)
{
    if (pin < servo_cfg->num_channels) {
        return pios_servo_pin_bank[pin];
    } else {
        return 0;
    }
}

#endif /* PIOS_INCLUDE_SERVO */
