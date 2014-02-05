/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup  PIOS_OPLinkRCVR OPLink Receiver Input Functions
 * @brief	Code to read the channels within the OPLinkReceiver UAVObject
 * @{
 *
 * @file       pios_opinkrcvr.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      GCS Input functions (STM32 dependent)
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

#include <pios.h>

#ifdef PIOS_INCLUDE_OPLINKRCVR

#include <uavobjectmanager.h>
#include <oplinkreceiver.h>
#include <pios_oplinkrcvr_priv.h>

//#define OPLINKRCVR_DEBUGLOG 1

#if defined OPLINKRCVR_DEBUGLOG && defined FLASH_FREERTOS
#define OPLINKRCVR_DEBUGLOG_PRINTF(...) PIOS_DEBUGLOG_Printf(__VA_ARGS__)
#endif
#ifndef OPLINKRCVR_DEBUGLOG_PRINTF
#define OPLINKRCVR_DEBUGLOG_PRINTF(...)
#endif

static OPLinkReceiverData oplinkreceiverdata;

/* Provide a RCVR driver */
static int32_t PIOS_OPLinkRCVR_Get(uint32_t rcvr_id, uint8_t channel);
static void PIOS_oplinkrcvr_Supervisor(uint32_t ppm_id);

const struct pios_rcvr_driver pios_oplinkrcvr_rcvr_driver = {
    .read = PIOS_OPLinkRCVR_Get,
};

/* Local Variables */
enum pios_oplinkrcvr_dev_magic {
    PIOS_OPLINKRCVR_DEV_MAGIC = 0x07ab9e2544cf5029,
};

struct pios_oplinkrcvr_dev {
    enum pios_oplinkrcvr_dev_magic magic;

    uint8_t supv_timer;
    bool    Fresh;
};

static struct pios_oplinkrcvr_dev *global_oplinkrcvr_dev;

static bool PIOS_oplinkrcvr_validate(struct pios_oplinkrcvr_dev *oplinkrcvr_dev)
{
    return oplinkrcvr_dev->magic == PIOS_OPLINKRCVR_DEV_MAGIC;
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_oplinkrcvr_dev *PIOS_oplinkrcvr_alloc(void)
{
    struct pios_oplinkrcvr_dev *oplinkrcvr_dev;

    oplinkrcvr_dev = (struct pios_oplinkrcvr_dev *)pvPortMalloc(sizeof(*oplinkrcvr_dev));
    if (!oplinkrcvr_dev) {
        return NULL;
    }

    oplinkrcvr_dev->magic = PIOS_OPLINKRCVR_DEV_MAGIC;
    oplinkrcvr_dev->Fresh = false;
    oplinkrcvr_dev->supv_timer = 0;

    /* The update callback cannot receive the device pointer, so set it in a global */
    global_oplinkrcvr_dev = oplinkrcvr_dev;

    return oplinkrcvr_dev;
}
#else
static struct pios_oplinkrcvr_dev pios_oplinkrcvr_devs[PIOS_OPLINKRCVR_MAX_DEVS];
static uint8_t pios_oplinkrcvr_num_devs;
static struct pios_oplinkrcvr_dev *PIOS_oplinkrcvr_alloc(void)
{
    struct pios_oplinkrcvr_dev *oplinkrcvr_dev;

    if (pios_oplinkrcvr_num_devs >= PIOS_OPLINKRCVR_MAX_DEVS) {
        return NULL;
    }

    oplinkrcvr_dev = &pios_oplinkrcvr_devs[pios_oplinkrcvr_num_devs++];
    oplinkrcvr_dev->magic = PIOS_OPLINKRCVR_DEV_MAGIC;
    oplinkrcvr_dev->Fresh = false;
    oplinkrcvr_dev->supv_timer = 0;

    global_oplinkrcvr_dev = oplinkrcvr_dev;

    return oplinkrcvr_dev;
}
#endif /* if defined(PIOS_INCLUDE_FREERTOS) */

static void oplinkreceiver_updated(UAVObjEvent *ev)
{
    struct pios_oplinkrcvr_dev *oplinkrcvr_dev = global_oplinkrcvr_dev;

    if (ev->obj == OPLinkReceiverHandle()) {
        OPLinkReceiverGet(&oplinkreceiverdata);
        oplinkrcvr_dev->Fresh = true;
    }
}

extern int32_t PIOS_OPLinkRCVR_Init(__attribute__((unused)) uint32_t *oplinkrcvr_id)
{
    struct pios_oplinkrcvr_dev *oplinkrcvr_dev;

    /* Allocate the device structure */
    oplinkrcvr_dev = (struct pios_oplinkrcvr_dev *)PIOS_oplinkrcvr_alloc();
    if (!oplinkrcvr_dev) {
        return -1;
    }

    for (uint8_t i = 0; i < OPLINKRECEIVER_CHANNEL_NUMELEM; i++) {
        /* Flush channels */
        oplinkreceiverdata.Channel[i] = PIOS_RCVR_TIMEOUT;
    }

    /* Register uavobj callback */
    OPLinkReceiverConnectCallback(oplinkreceiver_updated);

    /* Register the failsafe timer callback. */
    if (!PIOS_RTC_RegisterTickCallback(PIOS_oplinkrcvr_Supervisor, (uint32_t)oplinkrcvr_dev)) {
        PIOS_DEBUG_Assert(0);
    }

    return 0;
}

/**
 * Get the value of an input channel
 * \param[in] channel Number of the channel desired (zero based)
 * \output PIOS_RCVR_INVALID channel not available
 * \output PIOS_RCVR_TIMEOUT failsafe condition or missing receiver
 * \output >=0 channel value
 */
static int32_t PIOS_OPLinkRCVR_Get(__attribute__((unused)) uint32_t rcvr_id, uint8_t channel)
{
    if (channel >= OPLINKRECEIVER_CHANNEL_NUMELEM) {
        /* channel is out of range */
        return PIOS_RCVR_INVALID;
    }

    return oplinkreceiverdata.Channel[channel];
}

static void PIOS_oplinkrcvr_Supervisor(uint32_t oplinkrcvr_id)
{
    /* Recover our device context */
    struct pios_oplinkrcvr_dev *oplinkrcvr_dev = (struct pios_oplinkrcvr_dev *)oplinkrcvr_id;

    if (!PIOS_oplinkrcvr_validate(oplinkrcvr_dev)) {
        /* Invalid device specified */
        return;
    }

    /*
     * RTC runs at 625Hz.
     */
    if (++(oplinkrcvr_dev->supv_timer) < (PIOS_OPLINK_RCVR_TIMEOUT_MS * 1000 / 625)) {
        return;
    }
    oplinkrcvr_dev->supv_timer = 0;

    if (!oplinkrcvr_dev->Fresh) {
        OPLINKRCVR_DEBUGLOG_PRINTF("OPLINKRECEIVER TIMED OUT");
        for (int32_t i = 0; i < OPLINKRECEIVER_CHANNEL_NUMELEM; i++) {
            oplinkreceiverdata.Channel[i] = PIOS_RCVR_TIMEOUT;
        }
    }

    oplinkrcvr_dev->Fresh = false;
}

#endif /* PIOS_INCLUDE_OPLINKRCVR */

/**
 * @}
 * @}
 */
