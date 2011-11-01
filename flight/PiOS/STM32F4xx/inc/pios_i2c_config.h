/*
 * pios_i2c_config.h
 *
 *  Created on: May 8, 2011
 *      Author: Michael Smith
 */

#ifndef PIOS_I2C_CONFIG_H_
#define PIOS_I2C_CONFIG_H_

/**
 * Generic I2C configuration for the STM32F4xx
 */
#define I2C_CONFIG(_i2c, _scl_gpio, _scl_pin, _sda_gpio, _sda_pin) 		\
{                                                                       \
    .regs = _i2c,                                                       \
    .remap = GPIO_AF_ ## _i2c,                                          \
    .init = {                                                           \
        .I2C_ClockSpeed          = 400000,  /* bits/s */                \
        .I2C_Mode                = I2C_Mode_I2C,                        \
        .I2C_DutyCycle           = I2C_DutyCycle_2,                     \
        .I2C_OwnAddress1         = 0,                                   \
        .I2C_Ack                 = I2C_Ack_Enable,                      \
        .I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit,        \
    },                                                                  \
    .transfer_timeout_ms = 50,                                          \
    .scl = {                                                            \
        .gpio = _scl_gpio,                                              \
        .init = {                                                       \
            .GPIO_Pin   = _scl_pin,                                     \
            .GPIO_Mode  = GPIO_Mode_AF,                                 \
            .GPIO_Speed = GPIO_Speed_50MHz,                             \
            .GPIO_OType = GPIO_OType_OD,                                \
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,                             \
        },                                                              \
    },                                                                  \
    .sda = {                                                            \
        .gpio = _sda_gpio,                                              \
        .init = {                                                       \
            .GPIO_Pin   = _sda_pin,                                     \
            .GPIO_Mode  = GPIO_Mode_AF,                                 \
            .GPIO_Speed = GPIO_Speed_50MHz,                             \
            .GPIO_OType = GPIO_OType_OD,                                \
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,                             \
        },                                                              \
    },                                                                  \
    .event = {                                                          \
        .flags   = 0,       /* FIXME: check this */                     \
        .init = {                                                       \
            .NVIC_IRQChannel                   = _i2c ## _EV_IRQn,      \
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST, \
            .NVIC_IRQChannelSubPriority        = 0,                     \
            .NVIC_IRQChannelCmd                = ENABLE,                \
        },                                                              \
    },                                                                  \
    .error = {                                                          \
        .flags   = 0,       /* FIXME: check this */                     \
        .init = {                                                       \
            .NVIC_IRQChannel                   = _i2c ## _ER_IRQn,      \
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST, \
            .NVIC_IRQChannelSubPriority        = 0,                     \
            .NVIC_IRQChannelCmd                = ENABLE,                \
        },                                                              \
    },                                                                  \
}

#endif /* PIOS_I2C_CONFIG_H_ */
