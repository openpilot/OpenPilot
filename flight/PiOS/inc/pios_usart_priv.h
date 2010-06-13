/**
 ******************************************************************************
 *
 * @file       pios_usart_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
 * @brief      USART private definitions.
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

#ifndef PIOS_USART_PRIV_H
#define PIOS_USART_PRIV_H

#include <pios.h>
#include <pios_stm32.h>

struct pios_usart_cfg {
  USART_TypeDef               * regs;
  uint32_t                      remap; /* GPIO_Remap_* */
  USART_InitTypeDef             init;
  struct stm32_gpio             rx;
  struct stm32_gpio             tx;
  struct stm32_irq              irq;
};

struct pios_usart_buffer {
  uint8_t   buf[PIOS_USART_RX_BUFFER_SIZE];
  uint16_t  head;
  uint16_t  tail;
  uint16_t  size;
};

struct pios_usart_dev {
  const struct pios_usart_cfg * const cfg;
  struct pios_usart_buffer      rx;
  struct pios_usart_buffer      tx;
};

extern struct pios_usart_dev pios_usart_devs[];
extern uint8_t             pios_usart_num_devices;

#endif /* PIOS_USART_PRIV_H */
