/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_I2C I2C Functions
 * @brief STM32 Hardware dependent I2C functionality
 * @{
 *
 * @file       pios_i2c.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      I2C Enable/Disable routines
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



#if defined(PIOS_INCLUDE_I2C)

#if defined(PIOS_INCLUDE_FREERTOS)
#define USE_FREERTOS
#endif

#include <pios_i2c_priv.h>

enum i2c_adapter_event {
  I2C_EVENT_START,
  I2C_EVENT_STARTED_MORE_TXN_READ,
  I2C_EVENT_STARTED_MORE_TXN_WRITE,
  I2C_EVENT_STARTED_LAST_TXN_READ,
  I2C_EVENT_STARTED_LAST_TXN_WRITE,
  I2C_EVENT_ADDR_SENT_LEN_EQ_0,
  I2C_EVENT_ADDR_SENT_LEN_EQ_1,
  I2C_EVENT_ADDR_SENT_LEN_EQ_2,
  I2C_EVENT_ADDR_SENT_LEN_GT_2,
  I2C_EVENT_TRANSFER_DONE_LEN_EQ_0,
  I2C_EVENT_TRANSFER_DONE_LEN_EQ_1,
  I2C_EVENT_TRANSFER_DONE_LEN_EQ_2,
  I2C_EVENT_TRANSFER_DONE_LEN_GT_2,
  I2C_EVENT_AUTO,		/* FIXME: remove this */

  I2C_EVENT_NUM_EVENTS	/* Must be last */
};

static void go_faulted (struct pios_i2c_adapter * i2c_adapter);
static void go_stopping (struct pios_i2c_adapter * i2c_adapter);
static void go_stopped (struct pios_i2c_adapter * i2c_adapter);
static void go_starting (struct pios_i2c_adapter * i2c_adapter);
static void go_r_any_txn_addr (struct pios_i2c_adapter * i2c_adapter);
static void go_r_more_txn_pre_one (struct pios_i2c_adapter * i2c_adapter);
static void go_r_any_txn_pre_first (struct pios_i2c_adapter * i2c_adapter);
static void go_r_any_txn_pre_middle (struct pios_i2c_adapter * i2c_adapter);
static void go_r_more_txn_pre_last (struct pios_i2c_adapter * i2c_adapter);
static void go_r_any_txn_post_last (struct pios_i2c_adapter * i2c_adapter);

static void go_r_any_txn_addr (struct pios_i2c_adapter * i2c_adapter);
static void go_r_last_txn_pre_one (struct pios_i2c_adapter * i2c_adapter);
static void go_r_any_txn_pre_first (struct pios_i2c_adapter * i2c_adapter);
static void go_r_any_txn_pre_middle (struct pios_i2c_adapter * i2c_adapter);
static void go_r_last_txn_pre_last (struct pios_i2c_adapter * i2c_adapter);
static void go_r_any_txn_post_last (struct pios_i2c_adapter * i2c_adapter);

static void go_w_any_txn_addr (struct pios_i2c_adapter * i2c_adapter);
static void go_w_any_txn_middle (struct pios_i2c_adapter * i2c_adapter);
static void go_w_more_txn_last (struct pios_i2c_adapter * i2c_adapter);

static void go_w_any_txn_addr (struct pios_i2c_adapter * i2c_adapter);
static void go_w_any_txn_middle (struct pios_i2c_adapter * i2c_adapter);
static void go_w_last_txn_last (struct pios_i2c_adapter * i2c_adapter);

struct i2c_adapter_transition {
  void (*entry_fn)(struct pios_i2c_adapter * i2c_adapter);
  enum i2c_adapter_state next_state[I2C_EVENT_NUM_EVENTS];
};

static void i2c_adapter_inject_event(struct pios_i2c_adapter * i2c_adapter, enum i2c_adapter_event event);
static void i2c_adapter_fsm_init(struct pios_i2c_adapter * i2c_adapter);

const static struct i2c_adapter_transition i2c_adapter_transitions[I2C_STATE_NUM_STATES] = {
  [I2C_STATE_FAULTED] = {
    .entry_fn = go_faulted,
  },

  [I2C_STATE_STOPPED] = {
    .entry_fn = go_stopped,
    .next_state = {
      [I2C_EVENT_START]                  = I2C_STATE_STARTING,
    },
  },

  [I2C_STATE_STOPPING] = {
    .entry_fn = go_stopping,
    .next_state = {
      [I2C_EVENT_AUTO]                   = I2C_STATE_STOPPED,
    },
  },

  [I2C_STATE_STARTING] = {
    .entry_fn = go_starting,
    .next_state = {
      [I2C_EVENT_STARTED_MORE_TXN_READ]  = I2C_STATE_R_MORE_TXN_ADDR,
      [I2C_EVENT_STARTED_MORE_TXN_WRITE] = I2C_STATE_W_MORE_TXN_ADDR,
      [I2C_EVENT_STARTED_LAST_TXN_READ]  = I2C_STATE_R_LAST_TXN_ADDR,
      [I2C_EVENT_STARTED_LAST_TXN_WRITE] = I2C_STATE_W_LAST_TXN_ADDR,
    },
  },

  /*
   * Read with restart
   */

  [I2C_STATE_R_MORE_TXN_ADDR] = {
    .entry_fn = go_r_any_txn_addr,
    .next_state = {
      [I2C_EVENT_ADDR_SENT_LEN_EQ_1]     = I2C_STATE_R_MORE_TXN_PRE_ONE,
      [I2C_EVENT_ADDR_SENT_LEN_EQ_2]     = I2C_STATE_R_MORE_TXN_PRE_LAST,
      [I2C_EVENT_ADDR_SENT_LEN_GT_2]     = I2C_STATE_R_MORE_TXN_PRE_FIRST,
    },
  },

  [I2C_STATE_R_MORE_TXN_PRE_ONE] = {
    .entry_fn = go_r_more_txn_pre_one,
    .next_state = {
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_1] = I2C_STATE_R_MORE_TXN_POST_LAST,
    },
  },

  [I2C_STATE_R_MORE_TXN_PRE_FIRST] = {
    .entry_fn = go_r_any_txn_pre_first,
    .next_state = {
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_2] = I2C_STATE_R_MORE_TXN_PRE_LAST,
      [I2C_EVENT_TRANSFER_DONE_LEN_GT_2] = I2C_STATE_R_MORE_TXN_PRE_MIDDLE,
    },
  },

  [I2C_STATE_R_MORE_TXN_PRE_MIDDLE] = {
    .entry_fn = go_r_any_txn_pre_middle,
    .next_state = {
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_2] = I2C_STATE_R_MORE_TXN_PRE_LAST,
      [I2C_EVENT_TRANSFER_DONE_LEN_GT_2] = I2C_STATE_R_MORE_TXN_PRE_MIDDLE,
    },
  },

  [I2C_STATE_R_MORE_TXN_PRE_LAST] = {
    .entry_fn = go_r_more_txn_pre_last,
    .next_state = {
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_1] = I2C_STATE_R_MORE_TXN_POST_LAST,
    },
  },

  [I2C_STATE_R_MORE_TXN_POST_LAST] = {
    .entry_fn = go_r_any_txn_post_last,
    .next_state = {
      [I2C_EVENT_AUTO]                   = I2C_STATE_STARTING,
    },
  },

  /*
   * Read
   */

  [I2C_STATE_R_LAST_TXN_ADDR] = {
    .entry_fn = go_r_any_txn_addr,
    .next_state = {
      [I2C_EVENT_ADDR_SENT_LEN_EQ_1]     = I2C_STATE_R_LAST_TXN_PRE_ONE,
      [I2C_EVENT_ADDR_SENT_LEN_EQ_2]     = I2C_STATE_R_LAST_TXN_PRE_FIRST,
      [I2C_EVENT_ADDR_SENT_LEN_GT_2]     = I2C_STATE_R_LAST_TXN_PRE_FIRST,
    },
  },

  [I2C_STATE_R_LAST_TXN_PRE_ONE] = {
    .entry_fn = go_r_last_txn_pre_one,
    .next_state = {
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_1] = I2C_STATE_R_LAST_TXN_POST_LAST,
    },
  },

  [I2C_STATE_R_LAST_TXN_PRE_FIRST] = {
    .entry_fn = go_r_any_txn_pre_first,
    .next_state = {
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_2] = I2C_STATE_R_LAST_TXN_PRE_LAST,
      [I2C_EVENT_TRANSFER_DONE_LEN_GT_2] = I2C_STATE_R_LAST_TXN_PRE_MIDDLE,
    },
  },

  [I2C_STATE_R_LAST_TXN_PRE_MIDDLE] = {
    .entry_fn = go_r_any_txn_pre_middle,
    .next_state = {
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_2] = I2C_STATE_R_LAST_TXN_PRE_LAST,
      [I2C_EVENT_TRANSFER_DONE_LEN_GT_2] = I2C_STATE_R_LAST_TXN_PRE_MIDDLE,
    },
  },

  [I2C_STATE_R_LAST_TXN_PRE_LAST] = {
    .entry_fn = go_r_last_txn_pre_last,
    .next_state = {
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_1] = I2C_STATE_R_LAST_TXN_POST_LAST,
    },
  },

  [I2C_STATE_R_LAST_TXN_POST_LAST] = {
    .entry_fn = go_r_any_txn_post_last,
    .next_state = {
      [I2C_EVENT_AUTO]                   = I2C_STATE_STOPPING,
    },
  },


  /*
   * Write with restart
   */

  [I2C_STATE_W_MORE_TXN_ADDR] = {
    .entry_fn = go_w_any_txn_addr,
    .next_state = {
      [I2C_EVENT_ADDR_SENT_LEN_EQ_1]     = I2C_STATE_W_MORE_TXN_LAST,
      [I2C_EVENT_ADDR_SENT_LEN_EQ_2]     = I2C_STATE_W_MORE_TXN_MIDDLE,
      [I2C_EVENT_ADDR_SENT_LEN_GT_2]     = I2C_STATE_W_MORE_TXN_MIDDLE,
    },
  },

  [I2C_STATE_W_MORE_TXN_MIDDLE] = {
    .entry_fn = go_w_any_txn_middle,
    .next_state = {
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_1] = I2C_STATE_W_MORE_TXN_LAST,
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_2] = I2C_STATE_W_MORE_TXN_MIDDLE,
      [I2C_EVENT_TRANSFER_DONE_LEN_GT_2] = I2C_STATE_W_MORE_TXN_MIDDLE,
    },
  },

  [I2C_STATE_W_MORE_TXN_LAST] = {
    .entry_fn = go_w_more_txn_last,
    .next_state = {
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_0] = I2C_STATE_STARTING,
    },
  },

  /*
   * Write
   */

  [I2C_STATE_W_LAST_TXN_ADDR] = {
    .entry_fn = go_w_any_txn_addr,
    .next_state = {
      [I2C_EVENT_ADDR_SENT_LEN_EQ_1]     = I2C_STATE_W_LAST_TXN_LAST,
      [I2C_EVENT_ADDR_SENT_LEN_EQ_2]     = I2C_STATE_W_LAST_TXN_MIDDLE,
      [I2C_EVENT_ADDR_SENT_LEN_GT_2]     = I2C_STATE_W_LAST_TXN_MIDDLE,
    },
  },

  [I2C_STATE_W_LAST_TXN_MIDDLE] = {
    .entry_fn = go_w_any_txn_middle,
    .next_state = {
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_1] = I2C_STATE_W_LAST_TXN_LAST,
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_2] = I2C_STATE_W_LAST_TXN_MIDDLE,
      [I2C_EVENT_TRANSFER_DONE_LEN_GT_2] = I2C_STATE_W_LAST_TXN_MIDDLE,
    },
  },

  [I2C_STATE_W_LAST_TXN_LAST] = {
    .entry_fn = go_w_last_txn_last,
    .next_state = {
      [I2C_EVENT_TRANSFER_DONE_LEN_EQ_0] = I2C_STATE_STOPPING,
    },
  },
};

static void go_faulted (struct pios_i2c_adapter * i2c_adapter)
{
  PIOS_DEBUG_Assert(0);
}

static void go_stopping (struct pios_i2c_adapter * i2c_adapter)
{
  I2C_ITConfig(i2c_adapter->cfg->regs, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);

  /* Spin waiting for the stop bit to be cleared before continuing */
#define I2C_CR1_STOP_REQUESTED 0x0200
  while (i2c_adapter->cfg->regs->CR1 & I2C_CR1_STOP_REQUESTED);
}

static void go_stopped (struct pios_i2c_adapter * i2c_adapter)
{
#ifdef USE_FREERTOS  
  signed portBASE_TYPE pxHigherPriorityTaskWoken = pdFALSE;
#endif
  
  I2C_ITConfig(i2c_adapter->cfg->regs, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);
  I2C_AcknowledgeConfig(i2c_adapter->cfg->regs, ENABLE);

#ifdef USE_FREERTOS  
  if (xSemaphoreGiveFromISR(i2c_adapter->sem_ready, &pxHigherPriorityTaskWoken) != pdTRUE) {
    PIOS_DEBUG_Assert(0);
  }
  portEND_SWITCHING_ISR(pxHigherPriorityTaskWoken); /* FIXME: is this the right place for this? */
#endif /* USE_FREERTOS */
}

static void go_starting (struct pios_i2c_adapter * i2c_adapter)
{
  PIOS_DEBUG_Assert(i2c_adapter->active_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn >= i2c_adapter->first_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn <= i2c_adapter->last_txn);

  i2c_adapter->active_byte = &(i2c_adapter->active_txn->buf[0]);
  i2c_adapter->last_byte   = &(i2c_adapter->active_txn->buf[i2c_adapter->active_txn->len - 1]);

  I2C_GenerateSTART(i2c_adapter->cfg->regs, ENABLE);
  I2C_ITConfig(i2c_adapter->cfg->regs, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, ENABLE);
}

/* Common to 'more' and 'last' transaction */
static void go_r_any_txn_addr (struct pios_i2c_adapter * i2c_adapter)
{
  PIOS_DEBUG_Assert(i2c_adapter->active_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn >= i2c_adapter->first_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn <= i2c_adapter->last_txn);

  PIOS_DEBUG_Assert(i2c_adapter->active_txn->rw == PIOS_I2C_TXN_READ);

  I2C_Send7bitAddress(i2c_adapter->cfg->regs, (i2c_adapter->active_txn->addr)<<1, I2C_Direction_Receiver);
}

static void go_r_more_txn_pre_one (struct pios_i2c_adapter * i2c_adapter)
{
  I2C_AcknowledgeConfig(i2c_adapter->cfg->regs, DISABLE);
  I2C_GenerateSTART(i2c_adapter->cfg->regs, ENABLE);
}

static void go_r_last_txn_pre_one (struct pios_i2c_adapter * i2c_adapter)
{
  I2C_AcknowledgeConfig(i2c_adapter->cfg->regs, DISABLE);
  I2C_GenerateSTOP(i2c_adapter->cfg->regs, ENABLE);
}


/* Common to 'more' and 'last' transaction */
static void go_r_any_txn_pre_first (struct pios_i2c_adapter * i2c_adapter)
{
  I2C_AcknowledgeConfig(i2c_adapter->cfg->regs, ENABLE);
}

/* Common to 'more' and 'last' transaction */
static void go_r_any_txn_pre_middle (struct pios_i2c_adapter * i2c_adapter)
{
  PIOS_DEBUG_Assert(i2c_adapter->active_byte);
  PIOS_DEBUG_Assert(i2c_adapter->active_byte <= i2c_adapter->last_byte);

  *(i2c_adapter->active_byte) = I2C_ReceiveData(i2c_adapter->cfg->regs);

  /* Move to the next byte */
  i2c_adapter->active_byte++;
  PIOS_DEBUG_Assert(i2c_adapter->active_byte <= i2c_adapter->last_byte);
}

static void go_r_more_txn_pre_last (struct pios_i2c_adapter * i2c_adapter)
{
  PIOS_DEBUG_Assert(i2c_adapter->active_byte);
  PIOS_DEBUG_Assert(i2c_adapter->active_byte <= i2c_adapter->last_byte);

  I2C_AcknowledgeConfig(i2c_adapter->cfg->regs, DISABLE);
  PIOS_IRQ_Disable();
  I2C_ITConfig(i2c_adapter->cfg->regs, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);
  I2C_GenerateSTART(i2c_adapter->cfg->regs, ENABLE);
  *(i2c_adapter->active_byte) = I2C_ReceiveData(i2c_adapter->cfg->regs);
  I2C_ITConfig(i2c_adapter->cfg->regs, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, ENABLE);
  PIOS_IRQ_Enable();

  /* Move to the next byte */
  i2c_adapter->active_byte++;
  PIOS_DEBUG_Assert(i2c_adapter->active_byte <= i2c_adapter->last_byte);
}

static void go_r_last_txn_pre_last (struct pios_i2c_adapter * i2c_adapter)
{
  PIOS_DEBUG_Assert(i2c_adapter->active_byte);
  PIOS_DEBUG_Assert(i2c_adapter->active_byte <= i2c_adapter->last_byte);

  I2C_AcknowledgeConfig(i2c_adapter->cfg->regs, DISABLE);
  PIOS_IRQ_Disable();
  I2C_ITConfig(i2c_adapter->cfg->regs, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);
  I2C_GenerateSTOP(i2c_adapter->cfg->regs, ENABLE);
  *(i2c_adapter->active_byte) = I2C_ReceiveData(i2c_adapter->cfg->regs);
  I2C_ITConfig(i2c_adapter->cfg->regs, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, ENABLE);
  PIOS_IRQ_Enable();

  /* Move to the next byte */
  i2c_adapter->active_byte++;
  PIOS_DEBUG_Assert(i2c_adapter->active_byte <= i2c_adapter->last_byte);
}

/* Common to 'more' and 'last' transaction */
static void go_r_any_txn_post_last (struct pios_i2c_adapter * i2c_adapter)
{
  PIOS_DEBUG_Assert(i2c_adapter->active_byte);
  PIOS_DEBUG_Assert(i2c_adapter->active_byte == i2c_adapter->last_byte);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn >= i2c_adapter->first_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn <= i2c_adapter->last_txn);

  *(i2c_adapter->active_byte) = I2C_ReceiveData(i2c_adapter->cfg->regs);

  /* Move to the next byte */
  i2c_adapter->active_byte++;

  /* Move to the next transaction */
  i2c_adapter->active_txn++;
}

/* Common to 'more' and 'last' transaction */
static void go_w_any_txn_addr (struct pios_i2c_adapter * i2c_adapter)
{
  PIOS_DEBUG_Assert(i2c_adapter->active_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn >= i2c_adapter->first_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn <= i2c_adapter->last_txn);

  PIOS_DEBUG_Assert(i2c_adapter->active_txn->rw == PIOS_I2C_TXN_WRITE);

  I2C_Send7bitAddress(i2c_adapter->cfg->regs, (i2c_adapter->active_txn->addr)<<1, I2C_Direction_Transmitter);
}

static void go_w_any_txn_middle (struct pios_i2c_adapter * i2c_adapter)
{
  PIOS_DEBUG_Assert(i2c_adapter->active_byte);
  PIOS_DEBUG_Assert(i2c_adapter->active_byte < i2c_adapter->last_byte);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn >= i2c_adapter->first_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn <= i2c_adapter->last_txn);

  I2C_SendData(i2c_adapter->cfg->regs, *(i2c_adapter->active_byte));

  /* Move to the next byte */
  i2c_adapter->active_byte++;
  PIOS_DEBUG_Assert(i2c_adapter->active_byte <= i2c_adapter->last_byte);
}

static void go_w_more_txn_last (struct pios_i2c_adapter * i2c_adapter)
{
  PIOS_DEBUG_Assert(i2c_adapter->active_byte);
  PIOS_DEBUG_Assert(i2c_adapter->active_byte == i2c_adapter->last_byte);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn >= i2c_adapter->first_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn <= i2c_adapter->last_txn);

  I2C_SendData(i2c_adapter->cfg->regs, *(i2c_adapter->active_byte));

  /* Move to the next byte */
  i2c_adapter->active_byte++;

  /* Move to the next transaction */
  i2c_adapter->active_txn++;
  PIOS_DEBUG_Assert(i2c_adapter->active_txn <= i2c_adapter->last_txn);
}


static void go_w_last_txn_last (struct pios_i2c_adapter * i2c_adapter)
{
  PIOS_DEBUG_Assert(i2c_adapter->active_byte);
  PIOS_DEBUG_Assert(i2c_adapter->active_byte == i2c_adapter->last_byte);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn >= i2c_adapter->first_txn);
  PIOS_DEBUG_Assert(i2c_adapter->active_txn <= i2c_adapter->last_txn);

  I2C_ITConfig(i2c_adapter->cfg->regs, I2C_IT_BUF, DISABLE);
  I2C_SendData(i2c_adapter->cfg->regs, *(i2c_adapter->active_byte));

// SHOULD MOVE THIS INTO A STOPPING STATE AND SET IT ONLY AFTER THE BYTE WAS SENT
  I2C_GenerateSTOP(i2c_adapter->cfg->regs, ENABLE);

  /* Move to the next byte */
  i2c_adapter->active_byte++;
}

static void i2c_adapter_inject_event(struct pios_i2c_adapter * i2c_adapter, enum i2c_adapter_event event)
{
  PIOS_IRQ_Disable();

  /* 
   * Move to the next state
   *
   * This is done prior to calling the new state's entry function to 
   * guarantee that the entry function never depends on the previous
   * state.  This way, it cannot ever know what the previous state was.
   */
  enum i2c_adapter_state prev_state = i2c_adapter->curr_state;
  if (prev_state);

  i2c_adapter->curr_state = i2c_adapter_transitions[i2c_adapter->curr_state].next_state[event];

  /* Call the entry function (if any) for the next state. */
  if (i2c_adapter_transitions[i2c_adapter->curr_state].entry_fn) {
    i2c_adapter_transitions[i2c_adapter->curr_state].entry_fn(i2c_adapter);
  }
  PIOS_IRQ_Enable();
}

static void i2c_adapter_process_auto(struct pios_i2c_adapter * i2c_adapter)
{
  PIOS_IRQ_Disable();

  enum i2c_adapter_state prev_state = i2c_adapter->curr_state;
  if (prev_state);

  while (i2c_adapter_transitions[i2c_adapter->curr_state].next_state[I2C_EVENT_AUTO]) {
    i2c_adapter->curr_state = i2c_adapter_transitions[i2c_adapter->curr_state].next_state[I2C_EVENT_AUTO];

    /* Call the entry function (if any) for the next state. */
    if (i2c_adapter_transitions[i2c_adapter->curr_state].entry_fn) {
      i2c_adapter_transitions[i2c_adapter->curr_state].entry_fn(i2c_adapter);
    }
  }

  PIOS_IRQ_Enable();
}

static void i2c_adapter_fsm_init(struct pios_i2c_adapter * i2c_adapter)
{
  i2c_adapter->curr_state = I2C_STATE_STOPPED;
  //go_stopped(i2c_adapter);
}


#include <pios_i2c_priv.h>

static struct pios_i2c_adapter * find_i2c_adapter_by_id (uint8_t adapter)
{
  if (adapter >= pios_i2c_num_adapters) {
    /* Undefined I2C adapter for this board (see pios_board.c) */
    return NULL;
  }

  /* Get a handle for the device configuration */
  return &(pios_i2c_adapters[adapter]);
}

/**
* Initializes IIC driver
* \param[in] mode currently only mode 0 supported
* \return < 0 if initialisation failed
*/
int32_t PIOS_I2C_Init(void)
{
  struct pios_i2c_adapter * i2c_adapter;

  for (uint8_t i = 0; i < pios_i2c_num_adapters; i++) {
    /* Get a handle for the device configuration */
    i2c_adapter = find_i2c_adapter_by_id(i);
    PIOS_DEBUG_Assert(i2c_adapter);

#ifdef USE_FREERTOS
    /* 
     * Must be done prior to calling i2c_adapter_fsm_init()
     * since the sem_ready mutex is used in the initial state.
     */
    vSemaphoreCreateBinary(i2c_adapter->sem_ready);
    i2c_adapter->sem_busy = xSemaphoreCreateMutex();
#endif // USE_FREERTOS

    /* Initialize the state machine */
    i2c_adapter_fsm_init(i2c_adapter);

    /* Initialize the GPIO pins */
    GPIO_Init(i2c_adapter->cfg->scl.gpio, &(i2c_adapter->cfg->scl.init));
    GPIO_Init(i2c_adapter->cfg->sda.gpio, &(i2c_adapter->cfg->sda.init));

    /* Enable the associated peripheral clock */
    switch ((uint32_t)i2c_adapter->cfg->regs) {
    case (uint32_t)I2C1:
      /* Enable I2C peripheral clock (APB1 == slow speed) */
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
      break;
    case (uint32_t)I2C2:
      /* Enable I2C peripheral clock (APB1 == slow speed) */
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
      break;
    }

#if 0 /* FIXME: is this reset necessary? Should it be done in the stopped state? */
    /* Reset the I2C block */
    I2C_SoftwareResetCmd(i2c_adapter->cfg->regs, ENABLE);
    I2C_SoftwareResetCmd(i2c_adapter->cfg->regs, DISABLE);
#endif

    /* Reset the I2C block */
    I2C_DeInit(i2c_adapter->cfg->regs);

    /* Initialize the I2C block */
    I2C_Init(i2c_adapter->cfg->regs, &(i2c_adapter->cfg->init));

#if 1 /* FIXME: is this reset necessary? Should it be done in the stopped state? */
#define I2C_BUSY 0x20
    if (i2c_adapter->cfg->regs->SR2 & I2C_BUSY) {
      /* Reset the I2C block */
      I2C_SoftwareResetCmd(i2c_adapter->cfg->regs, ENABLE);
      I2C_SoftwareResetCmd(i2c_adapter->cfg->regs, DISABLE);
    }
#endif

    /* Configure and enable I2C interrupts */
    NVIC_Init(&(i2c_adapter->cfg->event.init));
    NVIC_Init(&(i2c_adapter->cfg->error.init));
  }

  /* No error */
  return 0;
}


bool PIOS_I2C_Transfer(uint8_t i2c, const struct pios_i2c_txn txn_list[], uint32_t num_txns)
{
  PIOS_DEBUG_Assert(txn_list);
  PIOS_DEBUG_Assert(num_txns);

  struct pios_i2c_adapter * i2c_adapter;

  i2c_adapter = find_i2c_adapter_by_id(i2c);
  PIOS_DEBUG_Assert(i2c_adapter);

#ifdef USE_FREERTOS
  /* Lock the bus */
  xSemaphoreTake(i2c_adapter->sem_busy, portMAX_DELAY);
#endif	/* USE_FREERTOS */

  PIOS_DEBUG_Assert(i2c_adapter->curr_state == I2C_STATE_STOPPED);

  i2c_adapter->first_txn  = &txn_list[0];
  i2c_adapter->last_txn   = &txn_list[num_txns - 1];
  i2c_adapter->active_txn = i2c_adapter->first_txn;

#ifdef USE_FREERTOS
  /* Make sure the done/ready semaphore is consumed before we start */
  xSemaphoreTake(i2c_adapter->sem_ready, portMAX_DELAY);
#endif

  i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_START);

  /* Wait for the transfer to complete */
#ifdef USE_FREERTOS
  xSemaphoreTake(i2c_adapter->sem_ready, portMAX_DELAY);
  xSemaphoreGive(i2c_adapter->sem_ready);
#else
  /* Spin waiting for the transfer to finish */
  while (i2c_adapter->curr_state != I2C_STATE_STOPPED);
#endif /* USE_FREERTOS */

#ifdef USE_FREERTOS
  /* Unlock the bus */
  xSemaphoreGive(i2c_adapter->sem_busy);
#endif	/* USE_FREERTOS */

  return TRUE;
}

#endif

void PIOS_I2C_EV_IRQ_Handler(uint8_t i2c)
{
  struct pios_i2c_adapter * i2c_adapter;

  i2c_adapter = find_i2c_adapter_by_id(i2c);
  PIOS_DEBUG_Assert(i2c_adapter);

  uint32_t event = I2C_GetLastEvent(i2c_adapter->cfg->regs);

  switch (event) {
  case (I2C_EVENT_MASTER_MODE_SELECT | 0x40):
    /* Unexplained event: EV5 + RxNE : Extraneous Rx.  Probably a late NACK from previous read. */
    /* Clean up the extra Rx until the root cause is identified and just keep going */
    (void) I2C_ReceiveData(i2c_adapter->cfg->regs);
    /* Fall through */
  case I2C_EVENT_MASTER_MODE_SELECT: /* EV5 */
    switch (i2c_adapter->active_txn->rw) {
    case PIOS_I2C_TXN_READ:
      if (i2c_adapter->active_txn == i2c_adapter->last_txn) {
	/* Final transaction */
	i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_STARTED_LAST_TXN_READ);
      } else if (i2c_adapter->active_txn < i2c_adapter->last_txn) {
	/* More transactions follow */
	i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_STARTED_MORE_TXN_READ);
      } else {
	PIOS_DEBUG_Assert(0);
      }
      break;
    case PIOS_I2C_TXN_WRITE:
      if (i2c_adapter->active_txn == i2c_adapter->last_txn) {
	/* Final transaction */
	i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_STARTED_LAST_TXN_WRITE);
      } else if (i2c_adapter->active_txn < i2c_adapter->last_txn) {
	/* More transactions follow */
	i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_STARTED_MORE_TXN_WRITE);
      } else {
	PIOS_DEBUG_Assert(0);
      }
      break;
    default:
      PIOS_DEBUG_Assert(0);
      break;
    }
    break;
  case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED: /* EV6 */
  case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED: /* EV6 */
    switch (i2c_adapter->last_byte - i2c_adapter->active_byte + 1) {
    case 0:
      i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_ADDR_SENT_LEN_EQ_0);
      break;
    case 1:
      i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_ADDR_SENT_LEN_EQ_1);
      break;
    case 2:
      i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_ADDR_SENT_LEN_EQ_2);
      break;
    default:
      i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_ADDR_SENT_LEN_GT_2);
      break;
    }
    break;
  case 0x80: /* TxE only.  TRA + MSL + BUSY have been cleared before we got here. */
    /* Ignore */
    {
      static volatile bool halt = FALSE;
      while (halt);
    }
    break;
  case 0: /* Unexplained spurious event.  Not sure what to do here. */
  case 0x40: /* RxNE only.  MSL + BUSY have already been cleared by HW. */
  case 0x44: /* RxNE + BTF.  MSL + BUSY have already been cleared by HW. */
  case I2C_EVENT_MASTER_BYTE_RECEIVED: /* EV7 */
  case (I2C_EVENT_MASTER_BYTE_RECEIVED | 0x4): /* EV7 + BTF */
  case I2C_EVENT_MASTER_BYTE_TRANSMITTED: /* EV8_2 */
  case 0x84: /* TxE + BTF. EV8_2 but TRA + MSL + BUSY have already been cleared by HW. */
    switch (i2c_adapter->last_byte - i2c_adapter->active_byte + 1) {
    case 0:
      i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_TRANSFER_DONE_LEN_EQ_0);
      break;
    case 1:
      i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_TRANSFER_DONE_LEN_EQ_1);
      break;
    case 2:
      i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_TRANSFER_DONE_LEN_EQ_2);
      break;
    default:
      i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_TRANSFER_DONE_LEN_GT_2);
      break;
    }
    break;
  case I2C_EVENT_MASTER_BYTE_TRANSMITTING: /* EV8 */
    /* Ignore this event and wait for TRANSMITTED in case we can't keep up */
    break;
  default:
    PIOS_DEBUG_Assert(0);
    break;
  }

  i2c_adapter_process_auto(i2c_adapter);
}

void PIOS_I2C_ER_IRQ_Handler(uint8_t i2c)
{
  struct pios_i2c_adapter * i2c_adapter;

  i2c_adapter = find_i2c_adapter_by_id(i2c);
  PIOS_DEBUG_Assert(i2c_adapter);

  /* Fail hard on any errors for now */
  PIOS_DEBUG_Assert(0);
}

/**
  * @}
  * @}
  */
