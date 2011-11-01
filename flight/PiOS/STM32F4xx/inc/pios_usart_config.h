/*
 * pios_usart_config.h
 *
 *  Created on: May 8, 2011
 *      Author: Michael Smioth
 */

#ifndef PIOS_USART_CONFIG_H_
#define PIOS_USART_CONFIG_H_

/**
 * Generic USART configuration structure for an STM32F2xx port.
 */
#define USART_CONFIG(_usart, _baudrate, _rx_gpio, _rx_pin, _tx_gpio, _tx_pin) \
{                                                                       \
    .regs  = _usart,                                                    \
    .remap = GPIO_AF_ ## _usart,                                        \
    .init = {                                                           \
        .USART_BaudRate            = _baudrate,                         \
        .USART_WordLength          = USART_WordLength_8b,               \
        .USART_Parity              = USART_Parity_No,                   \
        .USART_StopBits            = USART_StopBits_1,                  \
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None,	\
        .USART_Mode                = USART_Mode_Rx | USART_Mode_Tx,     \
    },                                                                  \
    .irq = {                                                            \
        .init    = {                                                    \
            .NVIC_IRQChannel                   = _usart ## _IRQn,       \
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,	\
            .NVIC_IRQChannelSubPriority        = 0,                     \
            .NVIC_IRQChannelCmd                = ENABLE,                \
        },                                                              \
    },                                                                  \
    .rx   = {                                                           \
        .gpio = _rx_gpio,                                               \
        .init = {                                                       \
            .GPIO_Pin   = _rx_pin,                                      \
            .GPIO_Mode  = GPIO_Mode_AF,                                 \
            .GPIO_Speed = GPIO_Speed_50MHz,                             \
            .GPIO_OType = GPIO_OType_PP,                                \
            .GPIO_PuPd  = GPIO_PuPd_UP,                                 \
        },                                                              \
    },                                                                  \
    .tx   = {                                                           \
        .gpio = _tx_gpio,                                               \
        .init = {                                                       \
            .GPIO_Pin   = _tx_pin,                                      \
            .GPIO_Mode  = GPIO_Mode_AF,                                 \
            .GPIO_Speed = GPIO_Speed_50MHz,                             \
            .GPIO_OType = GPIO_OType_PP,                                \
            .GPIO_PuPd  = GPIO_PuPd_UP,                                 \
        },                                                              \
    },                                                                  \
}

#endif /* PIOS_USART_CONFIG_H_ */
