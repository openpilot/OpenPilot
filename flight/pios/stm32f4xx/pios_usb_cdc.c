/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USB_COM USB COM Functions
 * @brief PIOS USB COM implementation for CDC interfaces
 * @notes      This implements a CDC Serial Port
 * @{
 *
 * @file       pios_usb_com_cdc.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      USB COM functions (STM32 dependent code)
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

#ifdef PIOS_INCLUDE_USB_CDC

#include "pios_usb_cdc_priv.h"
#include "pios_usb_board_data.h" /* PIOS_BOARD_*_DATA_LENGTH */
#include "pios_usbhook.h" /* PIOS_USBHOOK_* */

/* Implement COM layer driver API */
static void PIOS_USB_CDC_RegisterTxCallback(uint32_t usbcdc_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_USB_CDC_RegisterRxCallback(uint32_t usbcdc_id, pios_com_callback rx_in_cb, uint32_t context);
static void PIOS_USB_CDC_TxStart(uint32_t usbcdc_id, uint16_t tx_bytes_avail);
static void PIOS_USB_CDC_RxStart(uint32_t usbcdc_id, uint16_t rx_bytes_avail);
static bool PIOS_USB_CDC_Available(uint32_t usbcdc_id);

const struct pios_com_driver pios_usb_cdc_com_driver = {
    .tx_start   = PIOS_USB_CDC_TxStart,
    .rx_start   = PIOS_USB_CDC_RxStart,
    .bind_tx_cb = PIOS_USB_CDC_RegisterTxCallback,
    .bind_rx_cb = PIOS_USB_CDC_RegisterRxCallback,
    .available  = PIOS_USB_CDC_Available,
};

enum pios_usb_cdc_dev_magic {
    PIOS_USB_CDC_DEV_MAGIC = 0xAABBCCDD,
};

struct pios_usb_cdc_dev {
    enum pios_usb_cdc_dev_magic   magic;
    const struct pios_usb_cdc_cfg *cfg;

    uint32_t lower_id;

    pios_com_callback rx_in_cb;
    uint32_t rx_in_context;
    pios_com_callback tx_out_cb;
    uint32_t tx_out_context;

    bool     usb_ctrl_if_enabled;
    bool     usb_data_if_enabled;

    uint8_t  rx_packet_buffer[PIOS_USB_BOARD_CDC_DATA_LENGTH] __attribute__((aligned(4)));
    volatile bool rx_active;

    /*
     * NOTE: This is -1 as somewhat of a hack.  It ensures that we always send packets
     * that are strictly < maxPacketSize for this interface which means we never have
     * to bother with zero length packets (ZLP).
     */
    uint8_t  tx_packet_buffer[PIOS_USB_BOARD_CDC_DATA_LENGTH - 1] __attribute__((aligned(4)));
    volatile bool tx_active;

    uint8_t  ctrl_tx_packet_buffer[PIOS_USB_BOARD_CDC_MGMT_LENGTH] __attribute__((aligned(4)));

    uint32_t rx_dropped;
    uint32_t rx_oversize;

    /*
     * Used to hold the current state of the simulated UART.  Changes to this
     * variable may trigger new USB CDC Notification packets to be sent to the host.
     */
    volatile uint16_t prev_uart_state;
};

static bool PIOS_USB_CDC_validate(struct pios_usb_cdc_dev *usb_cdc_dev)
{
    return usb_cdc_dev && (usb_cdc_dev->magic == PIOS_USB_CDC_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_usb_cdc_dev *PIOS_USB_CDC_alloc(void)
{
    struct pios_usb_cdc_dev *usb_cdc_dev;

    usb_cdc_dev = (struct pios_usb_cdc_dev *)pvPortMalloc(sizeof(struct pios_usb_cdc_dev));
    if (!usb_cdc_dev) {
        return NULL;
    }

    memset(usb_cdc_dev, 0, sizeof(struct pios_usb_cdc_dev));
    usb_cdc_dev->magic = PIOS_USB_CDC_DEV_MAGIC;
    return usb_cdc_dev;
}
#else
static struct pios_usb_cdc_dev pios_usb_cdc_devs[PIOS_USB_CDC_MAX_DEVS];
static uint8_t pios_usb_cdc_num_devs;
static struct pios_usb_cdc_dev *PIOS_USB_CDC_alloc(void)
{
    struct pios_usb_cdc_dev *usb_cdc_dev;

    if (pios_usb_cdc_num_devs >= PIOS_USB_CDC_MAX_DEVS) {
        return NULL;
    }

    usb_cdc_dev = &pios_usb_cdc_devs[pios_usb_cdc_num_devs++];

    memset(usb_cdc_dev, 0, sizeof(struct pios_usb_cdc_dev));
    usb_cdc_dev->magic = PIOS_USB_CDC_DEV_MAGIC;

    return usb_cdc_dev;
}
#endif /* if defined(PIOS_INCLUDE_FREERTOS) */

/* Implement USB_IFOPS for CDC Control Interface */
static void PIOS_USB_CDC_CTRL_IF_Init(uint32_t usb_cdc_id);
static void PIOS_USB_CDC_CTRL_IF_DeInit(uint32_t usb_cdc_id);
static bool PIOS_USB_CDC_CTRL_IF_Setup(uint32_t usb_cdc_id, struct usb_setup_request *req);
static void PIOS_USB_CDC_CTRL_IF_CtrlDataOut(uint32_t usb_cdc_id, const struct usb_setup_request *req);

static struct pios_usb_ifops usb_cdc_ctrl_ifops = {
    .init   = PIOS_USB_CDC_CTRL_IF_Init,
    .deinit = PIOS_USB_CDC_CTRL_IF_DeInit,
    .setup  = PIOS_USB_CDC_CTRL_IF_Setup,
    .ctrl_data_out = PIOS_USB_CDC_CTRL_IF_CtrlDataOut,
};

/* Implement USB_IFOPS for CDC Data Interface */
static void PIOS_USB_CDC_DATA_IF_Init(uint32_t usb_cdc_id);
static void PIOS_USB_CDC_DATA_IF_DeInit(uint32_t usb_cdc_id);
static bool PIOS_USB_CDC_DATA_IF_Setup(uint32_t usb_cdc_id, struct usb_setup_request *req);
static void PIOS_USB_CDC_DATA_IF_CtrlDataOut(uint32_t usb_cdc_id, const struct usb_setup_request *req);

static struct pios_usb_ifops usb_cdc_data_ifops = {
    .init   = PIOS_USB_CDC_DATA_IF_Init,
    .deinit = PIOS_USB_CDC_DATA_IF_DeInit,
    .setup  = PIOS_USB_CDC_DATA_IF_Setup,
    .ctrl_data_out = PIOS_USB_CDC_DATA_IF_CtrlDataOut,
};

static uint32_t pios_usb_cdc_id;

int32_t PIOS_USB_CDC_Init(uint32_t *usbcdc_id, const struct pios_usb_cdc_cfg *cfg, uint32_t lower_id)
{
    PIOS_Assert(usbcdc_id);
    PIOS_Assert(cfg);

    struct pios_usb_cdc_dev *usb_cdc_dev;

    usb_cdc_dev = (struct pios_usb_cdc_dev *)PIOS_USB_CDC_alloc();
    if (!usb_cdc_dev) {
        goto out_fail;
    }

    /* Bind the configuration to the device instance */
    usb_cdc_dev->cfg = cfg;
    usb_cdc_dev->lower_id = lower_id;

    pios_usb_cdc_id  = (uint32_t)usb_cdc_dev;

    /* Rx and Tx are not active yet */
    usb_cdc_dev->rx_active           = false;
    usb_cdc_dev->tx_active           = false;

    /* Clear stats */
    usb_cdc_dev->rx_dropped          = 0;
    usb_cdc_dev->rx_oversize         = 0;

    /* Initialize the uart state */
    usb_cdc_dev->prev_uart_state     = 0;

    /* Register class specific interface callbacks with the USBHOOK layer */
    usb_cdc_dev->usb_ctrl_if_enabled = false;
    PIOS_USBHOOK_RegisterIfOps(cfg->ctrl_if, &usb_cdc_ctrl_ifops, (uint32_t)usb_cdc_dev);

    /* Register class specific interface callbacks with the USBHOOK layer */
    usb_cdc_dev->usb_data_if_enabled = false;
    PIOS_USBHOOK_RegisterIfOps(cfg->data_if, &usb_cdc_data_ifops, (uint32_t)usb_cdc_dev);

    *usbcdc_id = (uint32_t)usb_cdc_dev;

    return 0;

out_fail:
    return -1;
}

static bool PIOS_USB_CDC_SendData(struct pios_usb_cdc_dev *usb_cdc_dev)
{
    uint16_t bytes_to_tx;

    if (!usb_cdc_dev->tx_out_cb) {
        return false;
    }

    bool need_yield = false;
    bytes_to_tx = (usb_cdc_dev->tx_out_cb)(usb_cdc_dev->tx_out_context,
                                           usb_cdc_dev->tx_packet_buffer,
                                           sizeof(usb_cdc_dev->tx_packet_buffer),
                                           NULL,
                                           &need_yield);
    if (bytes_to_tx == 0) {
        return false;
    }

    /*
     * Mark this endpoint as being tx active _before_ actually transmitting
     * to make sure we don't race with the Tx completion interrupt
     */
    usb_cdc_dev->tx_active = true;

    PIOS_USBHOOK_EndpointTx(usb_cdc_dev->cfg->data_tx_ep,
                            usb_cdc_dev->tx_packet_buffer,
                            bytes_to_tx);

#if defined(PIOS_INCLUDE_FREERTOS)
    if (need_yield) {
        vPortYieldFromISR();
    }
#endif /* PIOS_INCLUDE_FREERTOS */

    return true;
}

static void PIOS_USB_CDC_RxStart(uint32_t usbcdc_id, uint16_t rx_bytes_avail)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)usbcdc_id;

    bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);

    PIOS_Assert(valid);

    /* Make sure this USB interface has been initialized */
    if (!usb_cdc_dev->usb_data_if_enabled) {
        return;
    }

    if (!PIOS_USB_CheckAvailable(usb_cdc_dev->lower_id)) {
        return;
    }

    // If endpoint was stalled and there is now space make it valid
    if (!usb_cdc_dev->rx_active && (rx_bytes_avail >= PIOS_USB_BOARD_CDC_DATA_LENGTH)) {
        PIOS_USBHOOK_EndpointRx(usb_cdc_dev->cfg->data_rx_ep,
                                usb_cdc_dev->rx_packet_buffer,
                                sizeof(usb_cdc_dev->rx_packet_buffer));
        usb_cdc_dev->rx_active = true;
    }
}

static void PIOS_USB_CDC_TxStart(uint32_t usbcdc_id, __attribute__((unused)) uint16_t tx_bytes_avail)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)usbcdc_id;

    bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);

    PIOS_Assert(valid);

    /* Make sure this USB interface has been initialized */
    if (!usb_cdc_dev->usb_data_if_enabled) {
        return;
    }

    if (!PIOS_USB_CheckAvailable(usb_cdc_dev->lower_id)) {
        return;
    }

    if (!usb_cdc_dev->tx_active) {
        /* Transmitter is not currently active, send a report */
        PIOS_USB_CDC_SendData(usb_cdc_dev);
    }
}

static void PIOS_USB_CDC_RegisterRxCallback(uint32_t usbcdc_id, pios_com_callback rx_in_cb, uint32_t context)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)usbcdc_id;

    bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);

    PIOS_Assert(valid);

    /*
     * Order is important in these assignments since ISR uses _cb
     * field to determine if it's ok to dereference _cb and _context
     */
    usb_cdc_dev->rx_in_context = context;
    usb_cdc_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_USB_CDC_RegisterTxCallback(uint32_t usbcdc_id, pios_com_callback tx_out_cb, uint32_t context)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)usbcdc_id;

    bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);

    PIOS_Assert(valid);

    /*
     * Order is important in these assignments since ISR uses _cb
     * field to determine if it's ok to dereference _cb and _context
     */
    usb_cdc_dev->tx_out_context = context;
    usb_cdc_dev->tx_out_cb = tx_out_cb;
}

static bool PIOS_USB_CDC_CTRL_EP_IN_Callback(uint32_t usb_cdc_id, uint8_t epnum, uint16_t len);

static void PIOS_USB_CDC_CTRL_IF_Init(uint32_t usb_cdc_id)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)usb_cdc_id;

    if (!PIOS_USB_CDC_validate(usb_cdc_dev)) {
        return;
    }

    /* Register endpoint specific callbacks with the USBHOOK layer */
    PIOS_USBHOOK_RegisterEpInCallback(usb_cdc_dev->cfg->ctrl_tx_ep,
                                      sizeof(usb_cdc_dev->ctrl_tx_packet_buffer),
                                      PIOS_USB_CDC_CTRL_EP_IN_Callback,
                                      (uint32_t)usb_cdc_dev);
    usb_cdc_dev->usb_ctrl_if_enabled = true;
}

static void PIOS_USB_CDC_CTRL_IF_DeInit(uint32_t usb_cdc_id)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)usb_cdc_id;

    if (!PIOS_USB_CDC_validate(usb_cdc_dev)) {
        return;
    }

    /* DeRegister endpoint specific callbacks with the USBHOOK layer */
    usb_cdc_dev->usb_data_if_enabled = false;
}

static uint8_t cdc_altset;
static struct usb_cdc_line_coding line_coding = {
    .dwDTERate   = htousbl(57600),
    .bCharFormat = USB_CDC_LINE_CODING_STOP_1,
    .bParityType = USB_CDC_LINE_CODING_PARITY_NONE,
    .bDataBits   = 8,
};

static uint16_t control_line_state;

static bool PIOS_USB_CDC_CTRL_IF_Setup(uint32_t usb_cdc_id, struct usb_setup_request *req)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)usb_cdc_id;

    if (!PIOS_USB_CDC_validate(usb_cdc_dev)) {
        return false;
    }

    /* Make sure this is a request for an interface we know about */
    uint8_t ifnum = req->wIndex & 0xFF;
    if (ifnum != usb_cdc_dev->cfg->ctrl_if) {
        return false;
    }

    switch (req->bmRequestType & (USB_REQ_TYPE_MASK | USB_REQ_RECIPIENT_MASK)) {
    case (USB_REQ_TYPE_STANDARD | USB_REQ_RECIPIENT_INTERFACE):
        switch (req->bRequest) {
        case USB_REQ_GET_INTERFACE:
            PIOS_USBHOOK_CtrlTx(&cdc_altset, 1);
            break;
        case USB_REQ_SET_INTERFACE:
            cdc_altset = (uint8_t)(req->wValue);
            break;
        default:
            /* Unhandled standard request */
            return false;

            break;
        }
        break;
    case (USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE):
        switch (req->bRequest) {
        case USB_CDC_REQ_SET_LINE_CODING:
            PIOS_USBHOOK_CtrlRx((uint8_t *)&line_coding, sizeof(line_coding));
            break;
        case USB_CDC_REQ_GET_LINE_CODING:
            PIOS_USBHOOK_CtrlTx((uint8_t *)&line_coding, sizeof(line_coding));
            break;
        case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
            control_line_state = req->wValue;
            break;
        default:
            /* Unhandled class request */
            return false;

            break;
        }
        break;
    default:
        /* Unhandled request */
        return false;
    }

    return true;
}

static bool PIOS_USB_CDC_Available(uint32_t usbcdc_id)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)usbcdc_id;

    bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);

    PIOS_Assert(valid);

    return PIOS_USB_CheckAvailable(usb_cdc_dev->lower_id) &&
           (control_line_state & USB_CDC_CONTROL_LINE_STATE_DTE_PRESENT);
}

/**
 * Called *after* the data has been written to the buffer provided in the setup stage.  The
 * setup request is passed in here again so we know *which* EP0 data out has just completed.
 */
static void PIOS_USB_CDC_CTRL_IF_CtrlDataOut(uint32_t usb_cdc_id, const struct usb_setup_request *req)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)usb_cdc_id;

    if (!PIOS_USB_CDC_validate(usb_cdc_dev)) {
        return;
    }

    /* Make sure this is a request for an interface we know about */
    uint8_t ifnum = req->wIndex & 0xFF;
    if (ifnum != usb_cdc_dev->cfg->ctrl_if) {
        return;
    }

    switch (req->bmRequestType & (USB_REQ_TYPE_MASK | USB_REQ_RECIPIENT_MASK)) {
    case (USB_REQ_TYPE_STANDARD | USB_REQ_RECIPIENT_INTERFACE):
        switch (req->bRequest) {
        default:
            /* Unhandled standard request */
            return;

            break;
        }
        break;
    case (USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_INTERFACE):
        switch (req->bRequest) {
        case USB_CDC_REQ_SET_LINE_CODING:
            /*
             * If we cared to, this is where we would apply the new line coding
             * that is now stored in the line_coding struct.  This could be used
             * to notify the upper COM layer that the baud rate has changed.  This
             * may be useful in the case of a COM USB bridge where we would
             * auto-adjust the USART baud rate based on the line coding set here.
             */
            break;
        default:
            /* Unhandled class request */
            return;

            break;
        }
        break;
    default:
        /* Unhandled request */
        return;
    }
}

static struct usb_cdc_serial_state_report uart_state = {
    .bmRequestType = 0xA1,
    .bNotification = USB_CDC_NOTIFICATION_SERIAL_STATE,
    .wValue      = 0,
    .wIndex      = htousbs(1),
    .wLength     = htousbs(2),
    .bmUartState = htousbs(0),
};

static bool PIOS_USB_CDC_CTRL_EP_IN_Callback(
    __attribute__((unused)) uint32_t usb_cdc_id,
    __attribute__((unused)) uint8_t epnum,
    __attribute__((unused)) uint16_t len)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)pios_usb_cdc_id;

    bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);

    PIOS_Assert(valid);

    /* Give back UART State Bitmap */
    /* UART State Bitmap
     *   15-7: reserved
     *      6:  bOverRun    overrun error
     *      5:  bParity     parity error
     *      4:  bFraming    framing error
     *      3:  bRingSignal RI
     *      2:  bBreak      break reception
     *      1:  bTxCarrier  DSR
     *      0:  bRxCarrier  DCD
     */

    /* Currently, we only handle TxCarrier and RxCarrier reporting */
    uint16_t new_uart_state = 0;
    if (usb_cdc_dev->tx_out_cb) {
        /* Someone is going to providing FC->PC data, advertise an RxCarrier to the host */
        new_uart_state |= 0x1;
    }
    if (usb_cdc_dev->rx_in_cb) {
        /* Someone is consuming PC->FC data, advertise a TxCarrier to the host */
        new_uart_state |= 0x2;
    }

    /* Has anything changed since we last sent a notification? */
    if ((new_uart_state ^ usb_cdc_dev->prev_uart_state) & 0x3) {
        usb_cdc_dev->prev_uart_state = new_uart_state;

        uart_state.bmUartState = htousbs(new_uart_state);

        PIOS_USBHOOK_EndpointTx(usb_cdc_dev->cfg->ctrl_tx_ep,
                                (uint8_t *)&uart_state,
                                sizeof(uart_state));
    }

    return true;
}

static bool PIOS_USB_CDC_DATA_EP_IN_Callback(uint32_t usb_cdc_id, uint8_t epnum, uint16_t len);
static bool PIOS_USB_CDC_DATA_EP_OUT_Callback(uint32_t usb_cdc_id, uint8_t epnum, uint16_t len);

static void PIOS_USB_CDC_DATA_IF_Init(uint32_t usb_cdc_id)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)usb_cdc_id;

    if (!PIOS_USB_CDC_validate(usb_cdc_dev)) {
        return;
    }

    /* Register endpoint specific callbacks with the USBHOOK layer */
    PIOS_USBHOOK_RegisterEpInCallback(usb_cdc_dev->cfg->data_tx_ep,
                                      sizeof(usb_cdc_dev->tx_packet_buffer),
                                      PIOS_USB_CDC_DATA_EP_IN_Callback,
                                      (uint32_t)usb_cdc_dev);
    PIOS_USBHOOK_RegisterEpOutCallback(usb_cdc_dev->cfg->data_rx_ep,
                                       sizeof(usb_cdc_dev->rx_packet_buffer),
                                       PIOS_USB_CDC_DATA_EP_OUT_Callback,
                                       (uint32_t)usb_cdc_dev);
    usb_cdc_dev->usb_data_if_enabled = true;
}

static void PIOS_USB_CDC_DATA_IF_DeInit(uint32_t usb_cdc_id)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)usb_cdc_id;

    if (!PIOS_USB_CDC_validate(usb_cdc_dev)) {
        return;
    }

    /* DeRegister endpoint specific callbacks with the USBHOOK layer */
    usb_cdc_dev->usb_data_if_enabled = false;
    PIOS_USBHOOK_DeRegisterEpInCallback(usb_cdc_dev->cfg->data_tx_ep);
    PIOS_USBHOOK_DeRegisterEpOutCallback(usb_cdc_dev->cfg->data_rx_ep);
}

static bool PIOS_USB_CDC_DATA_IF_Setup(
    __attribute__((unused)) uint32_t usb_cdc_id,
    __attribute__((unused)) struct usb_setup_request *req)
{
    /* There are no valid EP0 transactions for CDC DATA interfaces */
    PIOS_Assert(0);

    return false;
}

/**
 * Called *after* the data has been written to the buffer provided in the setup stage.  The
 * setup request is passed in here again so we know *which* EP0 data out has just completed.
 */
static void PIOS_USB_CDC_DATA_IF_CtrlDataOut(
    __attribute__((unused)) uint32_t usb_cdc_id,
    __attribute__((unused)) const struct usb_setup_request *req)
{
    /* CDC DATA interfaces don't have any OUT data stages on the control endpoint */
    PIOS_Assert(0);
}

/**
 * @brief Callback used to indicate a transmission from device INto host completed
 * Checks if any data remains, pads it into HID packet and sends.
 */
static bool PIOS_USB_CDC_DATA_EP_IN_Callback(
    __attribute__((unused)) uint32_t usb_cdc_id,
    __attribute__((unused)) uint8_t epnum,
    __attribute__((unused)) uint16_t len)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)pios_usb_cdc_id;

    bool valid = PIOS_USB_CDC_validate(usb_cdc_dev);

    PIOS_Assert(valid);

    bool rc = PIOS_USB_CDC_SendData(usb_cdc_dev);
    if (!rc) {
        /* No additional data was transmitted, note that tx is no longer active */
        usb_cdc_dev->tx_active = false;
    }

    return rc;
}

static bool PIOS_USB_CDC_DATA_EP_OUT_Callback(
    __attribute__((unused)) uint32_t usb_cdc_id,
    __attribute__((unused)) uint8_t epnum,
    uint16_t len)
{
    struct pios_usb_cdc_dev *usb_cdc_dev = (struct pios_usb_cdc_dev *)pios_usb_cdc_id;

    if (!PIOS_USB_CDC_validate(usb_cdc_dev)) {
        return false;
    }

    if (len > sizeof(usb_cdc_dev->rx_packet_buffer)) {
        len = sizeof(usb_cdc_dev->rx_packet_buffer);
    }

    if (!usb_cdc_dev->rx_in_cb) {
        /* No Rx call back registered, disable the receiver */
        usb_cdc_dev->rx_active = false;
        return false;
    }

    uint16_t headroom;
    bool need_yield = false;
    uint16_t bytes_rxed;
    bytes_rxed = (usb_cdc_dev->rx_in_cb)(usb_cdc_dev->rx_in_context,
                                         usb_cdc_dev->rx_packet_buffer,
                                         len,
                                         &headroom,
                                         &need_yield);

    if (bytes_rxed < len) {
        /* Lost bytes on rx */
        usb_cdc_dev->rx_dropped += (len - bytes_rxed);
    }

    bool rc;
    if (headroom >= sizeof(usb_cdc_dev->rx_packet_buffer)) {
        /* We have room for a maximum length message */
        PIOS_USBHOOK_EndpointRx(usb_cdc_dev->cfg->data_rx_ep,
                                usb_cdc_dev->rx_packet_buffer,
                                sizeof(usb_cdc_dev->rx_packet_buffer));
        rc = true;
    } else {
        /* Not enough room left for a message, apply backpressure */
        usb_cdc_dev->rx_active = false;
        rc = false;
    }

#if defined(PIOS_INCLUDE_FREERTOS)
    if (need_yield) {
        vPortYieldFromISR();
    }
#endif /* PIOS_INCLUDE_FREERTOS */

    return rc;
}

#endif /* PIOS_INCLUDE_USB_CDC */
