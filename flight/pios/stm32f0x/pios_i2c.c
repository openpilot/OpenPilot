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

#include "pios.h"

#ifdef PIOS_INCLUDE_I2C

#if defined(PIOS_INCLUDE_FREERTOS)
#define USE_FREERTOS
#endif

#include <pios_i2c_priv.h>
/* Interrupt flags descriptions taken from UM1566 */

/* - TX1: Manages the event “Transmit Interrupt Status”
 * which means a new data shall be written in the I2C data register for the next transfer. */
/* - RXNE: Manages the event “Receive Buffer Not Empty” which
 * means a data has been received and should be read from the data register. */
/* - TCR: Available in Master mode only. Manages Transfer Complete Reload event which means
 * the master or slave received or transmitted nbytes and Reload =1. */
/* - TC: Available in Master mode only. Manages Transfer Complete event which means the master
 * received or transmitted all data and communication will be closed (Generate stop) or
 * another one will start (Repeated start). */
/* - ADDR: Manages the event “Address phase done” which means that the device in mode slave
 * received start bit followed by its own address and acknowledged it. */
/* - NACK: Manages the event “Not Acknowledge received flag” which means the device received
 * a NACK. In master mode this flag indicates an error (slave don’t respond to sent address).
 * In slave mode, when master received all data it send NACK to indicate to slave that all
 * data are received.*/
/* - STOP: Manages the event “Stop bit received” which means that the master has
 * closed the communication.*/


// Error mask = Bus error | Arbitration lost | Overrun/Underrun
#define I2C_ISR_ERROR_MASK (I2C_ISR_BERR | I2C_ISR_ARLO | I2C_ISR_OVR)
#define I2C_IT_ERR         (I2C_IT_ERRI)
// Interrupt sources

// Transfer Complete interrupt mask
#define I2C_IT_BUF         (I2C_IT_TCI)

// Stop Detection interrupt mask|Not Acknowledge received interrupt mask|
// Address Match interrupt mask|RX interrupt mask
#define I2C_IT_EVT         (I2C_IT_STOPI | I2C_IT_NACKI | I2C_IT_ADDRI | I2C_IT_RXI | I2C_IT_TXI)

// #define I2C_HALT_ON_ERRORS
#define I2C_ERROR_FLAGS \
    (I2C_FLAG_NACKF | I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR | \
     I2C_FLAG_PECERR | I2C_FLAG_TIMEOUT | I2C_FLAG_ALERT \
    )

#define I2C_EVENT_FLAGS \
    (I2C_FLAG_TXE | I2C_FLAG_TXIS | I2C_FLAG_RXNE | \
     I2C_FLAG_NACKF | I2C_FLAG_STOPF | I2C_FLAG_TC | I2C_FLAG_TCR)

enum i2c_adapter_state {
    I2C_STATE_FSM_FAULT = 0, /* Must be zero so undefined transitions land here */

    I2C_STATE_BUS_ERROR,

    I2C_STATE_STOPPED,
    I2C_STATE_STOPPING,

    I2C_STATE_TXN_SETUP,

    I2C_STATE_TRANSFER,

    I2C_STATE_NACK,

    I2C_STATE_NUM_STATES /* Must be last */
};

enum i2c_adapter_event {
    I2C_EVENT_BUS_ERROR,
    I2C_EVENT_START,
    I2C_EVENT_TRANSFER_DONE,
    I2C_EVENT_TRANSFER_DONE_NEXT_TXN,
    I2C_EVENT_NACK,
    I2C_EVENT_STOPPED,
    I2C_EVENT_AUTO, /* FIXME: remove this */

    I2C_EVENT_NUM_EVENTS /* Must be last */
};

#if defined(PIOS_I2C_DIAGNOSTICS)
static struct pios_i2c_fault_history i2c_adapter_fault_history;

volatile uint32_t i2c_evirq_history[I2C_LOG_DEPTH];
volatile uint8_t i2c_evirq_history_pointer = 0;

volatile uint8_t i2c_state_history[I2C_LOG_DEPTH];
volatile uint8_t i2c_state_history_pointer = 0;

volatile uint8_t i2c_state_event_history[I2C_LOG_DEPTH];
volatile uint8_t i2c_state_event_history_pointer;

static uint8_t i2c_fsm_fault_count   = 0;
static uint8_t i2c_bad_event_counter = 0;
static uint8_t i2c_error_interrupt_counter = 0;
static uint8_t i2c_nack_counter = 0;
static uint8_t i2c_timeout_counter   = 0;
#endif

static void go_fsm_fault(struct pios_i2c_adapter *i2c_adapter);
static void go_bus_error(struct pios_i2c_adapter *i2c_adapter);
static void go_stopping(struct pios_i2c_adapter *i2c_adapter);
static void go_stopped(struct pios_i2c_adapter *i2c_adapter);
void go_txn_setup(struct pios_i2c_adapter *i2c_adapter);
static void go_transfer(struct pios_i2c_adapter *i2c_adapter);

static void go_nack(struct pios_i2c_adapter *i2c_adapter);

struct i2c_adapter_transition {
    void (*entry_fn)(struct pios_i2c_adapter *i2c_adapter);
    enum i2c_adapter_state next_state[I2C_EVENT_NUM_EVENTS];
};

static void i2c_adapter_process_auto(struct pios_i2c_adapter *i2c_adapter);
static void i2c_adapter_inject_event(struct pios_i2c_adapter *i2c_adapter, enum i2c_adapter_event event);
static void i2c_adapter_fsm_init(struct pios_i2c_adapter *i2c_adapter);
// static bool i2c_adapter_wait_for_stopped(struct pios_i2c_adapter *i2c_adapter);
static void i2c_adapter_reset_bus(struct pios_i2c_adapter *i2c_adapter);

#ifdef PIOS_I2C_DIAGNOSTICS
static void i2c_adapter_log_fault(enum pios_i2c_error_type type);
#endif

static const struct i2c_adapter_transition i2c_adapter_transitions[I2C_STATE_NUM_STATES] = {
    [I2C_STATE_FSM_FAULT] = {
        .entry_fn   = go_fsm_fault,
        .next_state =       {
            [I2C_EVENT_AUTO] = I2C_STATE_STOPPING,
        },
    },
    [I2C_STATE_BUS_ERROR] = {
        .entry_fn   = go_bus_error,
        .next_state =       {
            [I2C_EVENT_AUTO] = I2C_STATE_STOPPING,
        },
    },

    [I2C_STATE_STOPPED] =   {
        .entry_fn   = go_stopped,
        .next_state =       {
            [I2C_EVENT_START]     = I2C_STATE_TXN_SETUP,
            [I2C_EVENT_BUS_ERROR] = I2C_STATE_BUS_ERROR,
        },
    },

    [I2C_STATE_STOPPING] =  {
        .entry_fn   = go_stopping,
        .next_state =       {
            [I2C_EVENT_STOPPED]   = I2C_STATE_STOPPED,
            [I2C_EVENT_BUS_ERROR] = I2C_STATE_BUS_ERROR,
        },
    },
    /*
     * Transaction setup
     */

    [I2C_STATE_TXN_SETUP] = {
        .entry_fn   = go_txn_setup,
        .next_state =       {
            [I2C_EVENT_TRANSFER_DONE] = I2C_STATE_TRANSFER,
            [I2C_EVENT_NACK] = I2C_STATE_NACK,
            [I2C_EVENT_BUS_ERROR]     = I2C_STATE_BUS_ERROR,
        },
    },

    /*
     * Transfers
     */

    [I2C_STATE_TRANSFER] =  {
        .entry_fn   = go_transfer,
        .next_state =       {
            [I2C_EVENT_TRANSFER_DONE] = I2C_STATE_TRANSFER,
            [I2C_EVENT_TRANSFER_DONE_NEXT_TXN] = I2C_STATE_TXN_SETUP,
            [I2C_EVENT_NACK]      = I2C_STATE_STOPPING,
            [I2C_EVENT_STOPPED]   = I2C_STATE_STOPPING,
            [I2C_EVENT_BUS_ERROR] = I2C_STATE_BUS_ERROR,
        },
    },

    [I2C_STATE_NACK] =      {
        .entry_fn   = go_nack,
        .next_state =       {
            [I2C_EVENT_AUTO] = I2C_STATE_STOPPING,
        },
    },
};

static void go_fsm_fault(struct pios_i2c_adapter *i2c_adapter)
{
#if defined(I2C_HALT_ON_ERRORS)
    PIOS_DEBUG_Assert(0);
#endif
    /* Note that this transfer has hit a bus error */
    i2c_adapter->bus_error = true;

    i2c_adapter_reset_bus(i2c_adapter);
}

static void go_bus_error(struct pios_i2c_adapter *i2c_adapter)
{
    /* Note that this transfer has hit a bus error */
    i2c_adapter->bus_error = true;

    i2c_adapter_reset_bus(i2c_adapter);
}

static void go_stopping(struct pios_i2c_adapter *i2c_adapter)
{
#ifdef USE_FREERTOS
    signed portBASE_TYPE pxHigherPriorityTaskWoken = pdFALSE;
#endif

    I2C_ITConfig(i2c_adapter->cfg->regs, I2C_IT_ERR | I2C_IT_BUF | I2C_IT_EVT, DISABLE);

#ifdef USE_FREERTOS
    if (xSemaphoreGiveFromISR(i2c_adapter->sem_ready, &pxHigherPriorityTaskWoken) != pdTRUE) {
#if defined(I2C_HALT_ON_ERRORS)
        PIOS_DEBUG_Assert(0);
#endif
    }
    portEND_SWITCHING_ISR(pxHigherPriorityTaskWoken);
#endif /* USE_FREERTOS */
}

static void go_stopped(struct pios_i2c_adapter *i2c_adapter)
{
    I2C_ITConfig(i2c_adapter->cfg->regs, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);
    // I2C_AcknowledgeConfig(i2c_adapter->cfg->regs, ENABLE);
}
/**
 * Setup a new transfer
 * @param i2c_adapter
 */
void go_txn_setup(struct pios_i2c_adapter *i2c_adapter)
{
    PIOS_DEBUG_Assert(i2c_adapter->active_txn);
    PIOS_DEBUG_Assert(i2c_adapter->active_txn >= i2c_adapter->first_txn);
    PIOS_DEBUG_Assert(i2c_adapter->active_txn <= i2c_adapter->last_txn);

    i2c_adapter->active_byte = &(i2c_adapter->active_txn->buf[0]);
    i2c_adapter->last_byte   = &(i2c_adapter->active_txn->buf[i2c_adapter->active_txn->len - 1]);

    I2C_ITConfig(i2c_adapter->cfg->regs, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, ENABLE);
    if (i2c_adapter->active_txn->rw == PIOS_I2C_TXN_READ) {
        I2C_TransferHandling(i2c_adapter->cfg->regs, i2c_adapter->active_txn->addr, i2c_adapter->active_txn->len,
                             /* Only last transaction generates Auto End */
                             i2c_adapter->active_txn == i2c_adapter->last_txn ? I2C_AutoEnd_Mode : I2C_SoftEnd_Mode,
                             I2C_Generate_Start_Read);
    } else {
        I2C_TransferHandling(i2c_adapter->cfg->regs, i2c_adapter->active_txn->addr, i2c_adapter->active_txn->len,
                             /* Only last transaction generates Auto End */
                             i2c_adapter->active_txn == i2c_adapter->last_txn ? I2C_AutoEnd_Mode : I2C_SoftEnd_Mode,
                             I2C_Generate_Start_Write);
    }
}

/**
 * transfer a byte and advance to the next byte/txn if this is not the last byte of the last txn
 * @param i2c_adapter
 */
static void go_transfer(struct pios_i2c_adapter *i2c_adapter)
{
    PIOS_DEBUG_Assert(i2c_adapter->active_byte);
    PIOS_DEBUG_Assert(i2c_adapter->active_byte <= i2c_adapter->last_byte);
    if (i2c_adapter->active_txn->rw == PIOS_I2C_TXN_READ) {
        *(i2c_adapter->active_byte) = I2C_ReceiveData(i2c_adapter->cfg->regs);
    } else {
        I2C_SendData(i2c_adapter->cfg->regs, *(i2c_adapter->active_byte));
    }
    /* it this the last transaction? */
    if (i2c_adapter->active_byte == i2c_adapter->last_byte &&
        i2c_adapter->active_txn < i2c_adapter->last_txn) {
        i2c_adapter->active_txn++;
        return;
    }
    /* Move to the next byte */
    i2c_adapter->active_byte++;
}

static void go_nack(struct pios_i2c_adapter *i2c_adapter)
{
    I2C_TransferHandling(i2c_adapter->cfg->regs, 0, 0, I2C_AutoEnd_Mode, I2C_Generate_Stop);
}

static void i2c_adapter_inject_event(struct pios_i2c_adapter *i2c_adapter, enum i2c_adapter_event event)
{
    PIOS_IRQ_Disable();

#if defined(PIOS_I2C_DIAGNOSTICS)
    i2c_state_event_history[i2c_state_event_history_pointer] = event;
    i2c_state_event_history_pointer = (i2c_state_event_history_pointer + 1) % I2C_LOG_DEPTH;

    i2c_state_history[i2c_state_history_pointer] = i2c_adapter->curr_state;
    i2c_state_history_pointer = (i2c_state_history_pointer + 1) % I2C_LOG_DEPTH;

    if (i2c_adapter_transitions[i2c_adapter->curr_state].next_state[event] == I2C_STATE_FSM_FAULT) {
        i2c_adapter_log_fault(PIOS_I2C_ERROR_FSM);
    }
#endif
    /*
     * Move to the next state
     *
     * This is done prior to calling the new state's entry function to
     * guarantee that the entry function never depends on the previous
     * state.  This way, it cannot ever know what the previous state was.
     */
    i2c_adapter->curr_state = i2c_adapter_transitions[i2c_adapter->curr_state].next_state[event];

    /* Call the entry function (if any) for the next state. */
    if (i2c_adapter_transitions[i2c_adapter->curr_state].entry_fn) {
        i2c_adapter_transitions[i2c_adapter->curr_state].entry_fn(i2c_adapter);
    }

    /* Process any AUTO transitions in the FSM */
    i2c_adapter_process_auto(i2c_adapter);

    PIOS_IRQ_Enable();
}

static void i2c_adapter_process_auto(struct pios_i2c_adapter *i2c_adapter)
{
    PIOS_IRQ_Disable();
    while (i2c_adapter_transitions[i2c_adapter->curr_state].next_state[I2C_EVENT_AUTO]) {
        i2c_adapter->curr_state = i2c_adapter_transitions[i2c_adapter->curr_state].next_state[I2C_EVENT_AUTO];

        /* Call the entry function (if any) for the next state. */
        if (i2c_adapter_transitions[i2c_adapter->curr_state].entry_fn) {
            i2c_adapter_transitions[i2c_adapter->curr_state].entry_fn(i2c_adapter);
        }
    }

    PIOS_IRQ_Enable();
}

static void i2c_adapter_fsm_init(struct pios_i2c_adapter *i2c_adapter)
{
    i2c_adapter_reset_bus(i2c_adapter);
    i2c_adapter->curr_state = I2C_STATE_STOPPED;
}
#if 0
static bool i2c_adapter_wait_for_stopped(struct pios_i2c_adapter *i2c_adapter)
{
    uint32_t guard;

    /*
     * Wait for the bus to return to the stopped state.
     * This was pulled out of the FSM due to occasional
     * failures at this transition which previously resulted
     * in spinning on this bit in the ISR forever.
     */
#define I2C_CR1_STOP_REQUESTED 0x0200
    for (guard = 1000000; /* FIXME: should use the configured bus timeout */
         guard && (i2c_adapter->cfg->regs->ISR & I2C_FLAG_STOPF); guard--) {
        continue;
    }
    if (!guard) {
        /* We timed out waiting for the stop condition */
        return false;
    }

    return true;
}
#endif
static void i2c_adapter_reset_bus(struct pios_i2c_adapter *i2c_adapter)
{
    (void)i2c_adapter;
    // I2C_SoftwareResetCmd(i2c_adapter->cfg->regs);
}

#include <pios_i2c_priv.h>

/* Return true if the FSM is in a terminal state */
static bool i2c_adapter_fsm_terminated(struct pios_i2c_adapter *i2c_adapter)
{
    switch (i2c_adapter->curr_state) {
    case I2C_STATE_STOPPING:
    case I2C_STATE_STOPPED:
        return true;

    default:
        return false;
    }
}

#if defined(PIOS_I2C_DIAGNOSTICS)

/**
 * Logs the last N state transitions and N IRQ events due to
 * an error condition
 * \param[in] i2c the adapter number to log an event for
 */
void i2c_adapter_log_fault(enum pios_i2c_error_type type)
{
    i2c_adapter_fault_history.type = type;
    for (uint8_t i = 0; i < I2C_LOG_DEPTH; i++) {
        i2c_adapter_fault_history.evirq[i] =
            i2c_evirq_history[(I2C_LOG_DEPTH + i2c_evirq_history_pointer - 1 - i) % I2C_LOG_DEPTH];
        i2c_adapter_fault_history.event[i] =
            i2c_state_event_history[(I2C_LOG_DEPTH + i2c_state_event_history_pointer - 1 - i) % I2C_LOG_DEPTH];
        i2c_adapter_fault_history.state[i] =
            i2c_state_history[(I2C_LOG_DEPTH + i2c_state_history_pointer - 1 - i) % I2C_LOG_DEPTH];
    }
    switch (type) {
    case PIOS_I2C_ERROR_EVENT:
        i2c_bad_event_counter++;
        break;
    case PIOS_I2C_ERROR_FSM:
        i2c_fsm_fault_count++;
        break;
    case PIOS_I2C_ERROR_INTERRUPT:
        i2c_error_interrupt_counter++;
        break;
    }
}

#endif /* if defined(PIOS_I2C_DIAGNOSTICS) */

/**
 * Logs the last N state transitions and N IRQ events due to
 * an error condition
 * \param[out] data address where to copy the pios_i2c_fault_history structure to
 * \param[out] counts three uint16 that receive the bad event, fsm, and error irq
 * counts
 */
void PIOS_I2C_GetDiagnostics(struct pios_i2c_fault_history *data, uint8_t *counts)
{
#if defined(PIOS_I2C_DIAGNOSTICS)
    memcpy(data, &i2c_adapter_fault_history, sizeof(i2c_adapter_fault_history));
    counts[0] = i2c_bad_event_counter;
    counts[1] = i2c_fsm_fault_count;
    counts[2] = i2c_error_interrupt_counter;
    counts[3] = i2c_nack_counter;
    counts[4] = i2c_timeout_counter;
#else
    struct pios_i2c_fault_history i2c_adapter_fault_history;
    i2c_adapter_fault_history.type = PIOS_I2C_ERROR_EVENT;

    memcpy(data, &i2c_adapter_fault_history, sizeof(i2c_adapter_fault_history));
    counts[0] = counts[1] = counts[2] = 0;
#endif
}

static bool PIOS_I2C_validate(struct pios_i2c_adapter *i2c_adapter)
{
    return i2c_adapter->magic == PIOS_I2C_DEV_MAGIC;
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_i2c_adapter *PIOS_I2C_alloc(void)
{
    struct pios_i2c_adapter *i2c_adapter;

    i2c_adapter = (struct pios_i2c_adapter *)pvPortMalloc(sizeof(*i2c_adapter));
    if (!i2c_adapter) {
        return NULL;
    }

    i2c_adapter->magic = PIOS_I2C_DEV_MAGIC;
    return i2c_adapter;
}
#else
static struct pios_i2c_adapter pios_i2c_adapters[PIOS_I2C_MAX_DEVS];
static uint8_t pios_i2c_num_adapters;
static struct pios_i2c_adapter *PIOS_I2C_alloc(void)
{
    struct pios_i2c_adapter *i2c_adapter;

    if (pios_i2c_num_adapters >= PIOS_I2C_MAX_DEVS) {
        return NULL;
    }

    i2c_adapter = &pios_i2c_adapters[pios_i2c_num_adapters++];
    i2c_adapter->magic = PIOS_I2C_DEV_MAGIC;

    return i2c_adapter;
}
#endif /* if defined(PIOS_INCLUDE_FREERTOS) */


/**
 * Initializes IIC driver
 * \param[in] mode currently only mode 0 supported
 * \return < 0 if initialisation failed
 */
int32_t PIOS_I2C_Init(uint32_t *i2c_id, const struct pios_i2c_adapter_cfg *cfg)
{
    PIOS_DEBUG_Assert(i2c_id);
    PIOS_DEBUG_Assert(cfg);

    struct pios_i2c_adapter *i2c_adapter;

    i2c_adapter = (struct pios_i2c_adapter *)PIOS_I2C_alloc();
    if (!i2c_adapter) {
        goto out_fail;
    }

    /* Bind the configuration to the device instance */
    i2c_adapter->cfg = cfg;

#ifdef USE_FREERTOS
    /*
     * Must be done prior to calling i2c_adapter_fsm_init()
     * since the sem_ready mutex is used in the initial state.
     */
    vSemaphoreCreateBinary(i2c_adapter->sem_ready);
    i2c_adapter->sem_busy = xSemaphoreCreateMutex();
#endif // USE_FREERTOS

    /* Enable the associated peripheral clock */
    switch ((uint32_t)i2c_adapter->cfg->regs) {
    case (uint32_t)I2C1:
        /* Enable I2C peripheral clock (APB1 == slow speed) */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
        RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);
        break;
    case (uint32_t)I2C2:
        /* Enable I2C peripheral clock (APB1 == slow speed) */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
        break;
    }

    if (i2c_adapter->cfg->remap) {
        GPIO_PinAFConfig(i2c_adapter->cfg->scl.gpio, i2c_adapter->cfg->scl.pin_source, i2c_adapter->cfg->remap);
        GPIO_PinAFConfig(i2c_adapter->cfg->sda.gpio, i2c_adapter->cfg->sda.pin_source, i2c_adapter->cfg->remap);
    }

    GPIO_Init(i2c_adapter->cfg->scl.gpio, &i2c_adapter->cfg->scl.init);
    GPIO_Init(i2c_adapter->cfg->sda.gpio, &i2c_adapter->cfg->sda.init);

    I2C_Init(i2c_adapter->cfg->regs, &i2c_adapter->cfg->init);

    I2C_Cmd(i2c_adapter->cfg->regs, ENABLE);

    /* Initialize the state machine */
    i2c_adapter_fsm_init(i2c_adapter);

    *i2c_id = (uint32_t)i2c_adapter;

    /* Configure and enable I2C interrupt */
    NVIC_Init(&(i2c_adapter->cfg->event.init));

    /* No error */
    return 0;

out_fail:
    return -1;
}

/**
 * @brief Perform a series of I2C transactions
 * @returns 0 if success or error code
 * @retval -1 for failed transaction
 * @retval -2 for failure to get semaphore
 */
int32_t PIOS_I2C_Transfer(uint32_t i2c_id, const struct pios_i2c_txn txn_list[], uint32_t num_txns)
{
    struct pios_i2c_adapter *i2c_adapter = (struct pios_i2c_adapter *)i2c_id;

    bool valid = PIOS_I2C_validate(i2c_adapter);

    PIOS_Assert(valid)

    PIOS_DEBUG_Assert(txn_list);
    PIOS_DEBUG_Assert(num_txns);

    bool semaphore_success = true;

#ifdef USE_FREERTOS
    /* Lock the bus */
    portTickType timeout;
    timeout = i2c_adapter->cfg->transfer_timeout_ms / portTICK_RATE_MS;
    if (xSemaphoreTake(i2c_adapter->sem_busy, timeout) == pdFALSE) {
        return -2;
    }
#else
    uint32_t timeout = 0xfff;
    while (i2c_adapter->busy && --timeout) {
        ;
    }
    if (timeout == 0) { // timed out
        return false;
    }

    PIOS_IRQ_Disable();
    if (i2c_adapter->busy) {
        return false;
    }
    i2c_adapter->busy = 1;
    PIOS_IRQ_Enable();
#endif /* USE_FREERTOS */

    PIOS_DEBUG_Assert(i2c_adapter->curr_state == I2C_STATE_STOPPED);

    i2c_adapter->first_txn  = &txn_list[0];
    i2c_adapter->last_txn   = &txn_list[num_txns - 1];
    i2c_adapter->active_txn = i2c_adapter->first_txn;

#ifdef USE_FREERTOS
    /* Make sure the done/ready semaphore is consumed before we start */
    semaphore_success &= (xSemaphoreTake(i2c_adapter->sem_ready, timeout) == pdTRUE);
#endif

    i2c_adapter->bus_error = false;
    i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_START);

    /* Wait for the transfer to complete */
#ifdef USE_FREERTOS
    semaphore_success &= (xSemaphoreTake(i2c_adapter->sem_ready, timeout) == pdTRUE);
    xSemaphoreGive(i2c_adapter->sem_ready);
#else
    PIOS_IRQ_Disable();
    i2c_adapter->busy = 0;
    PIOS_IRQ_Enable();
#endif /* USE_FREERTOS */

    /* Spin waiting for the transfer to finish */
    while (!i2c_adapter_fsm_terminated(i2c_adapter)) {
        ;
    }

    i2c_adapter_fsm_init(i2c_adapter);


#ifdef USE_FREERTOS
    /* Unlock the bus */
    xSemaphoreGive(i2c_adapter->sem_busy);
    if (!semaphore_success) {
#ifdef PIOS_I2C_DIAGNOSTICS
        i2c_timeout_counter++;
#endif
    }
#endif /* USE_FREERTOS */

    return !semaphore_success ? -2 :
           i2c_adapter->bus_error ? -1 :
           0;
}

void PIOS_I2C_IRQ_Handler(uint32_t i2c_id)
{
    struct pios_i2c_adapter *i2c_adapter = (struct pios_i2c_adapter *)i2c_id;

    bool valid = PIOS_I2C_validate(i2c_adapter);

    PIOS_Assert(valid)

    uint32_t event = i2c_adapter->cfg->regs->ISR;

#if defined(PIOS_I2C_DIAGNOSTICS)
    i2c_evirq_history[i2c_evirq_history_pointer] = event;
    i2c_evirq_history_pointer = (i2c_evirq_history_pointer + 1) % I2C_LOG_DEPTH;
#endif
    if (event & I2C_ERROR_FLAGS) {
        if (event & I2C_FLAG_NACKF) {
#ifdef PIOS_I2C_DIAGNOSTICS
            i2c_nack_counter++;
#endif
            I2C_ClearFlag(i2c_adapter->cfg->regs, I2C_FLAG_NACKF);

            i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_NACK);
        } else { /* Mostly bus errors here */
#ifdef PIOS_I2C_DIAGNOSTICS
            i2c_adapter_log_fault(PIOS_I2C_ERROR_INTERRUPT);
#endif
            /* Fail hard on any errors for now */
            i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_BUS_ERROR);
        }
        return;
    }
    // I2C_FLAG_TCR and reload not handled right now. Transfers limited to 255 bytes
    if (event & (I2C_FLAG_TXIS | I2C_ISR_RXNE)) {
        if (i2c_adapter->active_byte <= i2c_adapter->last_byte) {
            i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_TRANSFER_DONE);
            return;
        }
    }
    if (event & I2C_ISR_TC) {
        i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_TRANSFER_DONE_NEXT_TXN);
    }

    if (event & I2C_ISR_STOPF) {
        I2C_ClearFlag(i2c_adapter->cfg->regs, I2C_FLAG_STOPF);
        i2c_adapter_inject_event(i2c_adapter, I2C_EVENT_STOPPED);
        return;
    }
}

#endif /* PIOS_INCLUDE_I2C */

/**
 * @}
 * @}
 */
