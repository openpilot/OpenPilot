/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_TSLRSdebug TSLRS debug functions
 * @brief Code to read TSLRS debug serial stream
 * @{
 *
 * @file       pios_tslrsdebug.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Code to read TSLRS debug serial stream
 *             TSLRS debug code from http://code.google.com/p/minrxosd/
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

#ifdef PIOS_INCLUDE_TSLRSDEBUG

#include "pios_tslrsdebug.h"
#include "pios_tslrsdebug_priv.h"

struct pios_tslrsdebug_state *tslrsdebug_state;

/* Forward Declarations */
static uint16_t PIOS_TSLRSdebug_RxInCallback(uint32_t tslrsdebug_id, uint8_t *buf, uint16_t buf_len, uint16_t *headroom, bool *need_yield);
static void PIOS_TSLRSdebug_Supervisor(uint32_t tslrsdebug_id);

enum pios_tslrsdebug_dev_magic {
    PIOS_TSLRSDEBUG_DEV_MAGIC = 0x42537357,
};

struct pios_tslrsdebug_dev {
    enum pios_tslrsdebug_dev_magic   magic;
    const struct pios_tslrsdebug_cfg *cfg;
    struct pios_tslrsdebug_state     state;
};


//---------------------------------

static portTickType     LastPacketTime = 0;
static uint8_t          PacketTimeout = 37;
static uint8_t          PacketsPerSecond = 30;
static uint8_t          PacketWindow[PACKET_WINDOW_MAX];
static uint32_t         scan_value;


uint8_t tslrsdebug_packet_window_percent(void) {
    int i;
    uint8_t bads = 0;

    for (i = 0; i < PacketsPerSecond; i++) {
        if (PacketWindow[i] == PACKED_BAD) bads++;
    }

    return (uint8_t) ((1.0f - (float) bads / (float) PacketsPerSecond) * 100.0f + 0.5f);
}


static void scan_value_clear(void) {
    scan_value = 0;
}


static void scan_value_add(char c) {
    scan_value = scan_value * 10 + c - '0';
}


static void packet_window_set(uint8_t good_bad, uint8_t cnt) {
    static uint8_t index = 0;
    int i;

    for (i = 0; i < cnt; i++) {
        PacketWindow[index++] = good_bad;
        index = index >= PacketsPerSecond ? 0 : index;
    }
}


static int detect_str_eeprom(uint8_t c) {
    static char detect_str[] = " EEPROM";
    static int detect_cnt = 0;

    if (detect_cnt == 7) return 1;

    if (c == detect_str[detect_cnt]) detect_cnt++;
    else detect_cnt = 0;

    return 0;
}


static int detect_str_contact(uint8_t c) {
    static char detect_str[] = "Contact";
    static int detect_cnt = 0;

    if (detect_cnt == 7) return 1;

    if (c == detect_str[detect_cnt]) detect_cnt++;
    else detect_cnt = 0;

    return 0;
}


static uint16_t detect_frameduration(uint8_t c) {
    #define RATE_STR_LEN    6
    static char detect_str[] = "Rate: ";
    static int detect_cnt = 0;
    static uint16_t frameduration = 0;

    if (detect_cnt == RATE_STR_LEN + 6) return frameduration;

    if (detect_cnt == RATE_STR_LEN + 5) {
        PacketTimeout = (uint8_t) ((frameduration * PACKET_TIMEOUT_FACTOR + 500.0f) / 1000.0f);
        PacketsPerSecond = (uint8_t) (1000.0f / (frameduration / 1000.0f));
        PacketsPerSecond = PacketsPerSecond > PACKET_WINDOW_MAX ? PACKET_WINDOW_MAX : PacketsPerSecond;
        detect_cnt++;
        return 0;
    }

    if (detect_cnt >= RATE_STR_LEN) {
        frameduration = frameduration * 10 + c - '0';
        detect_cnt++;
        return 0;
    }

    if (c == detect_str[detect_cnt]) detect_cnt++;
    else detect_cnt = 0;

    return 0;
}


static void scan_value_set(struct pios_tslrsdebug_state *state) {
    switch (state->state) {
        case TSRX_FAILSAVE_SCAN:
            state->FailsafesDelta = scan_value - state->Failsafes;
            state->Failsafes = scan_value;
        break;
        case TSRX_GOOD_SCAN:
            state->GoodPacketsDelta = scan_value - state->GoodPackets;
            state->GoodPackets = scan_value;
        break;
        case TSRX_BAD_SCAN:
            state->BadPacketsDelta = scan_value - state->BadPackets;
            state->BadPackets = scan_value;
        break;
    }
    state->scan_value_percent = (uint8_t) ((1.0f - (float) state->BadPacketsDelta / (float) (state->GoodPacketsDelta + state->BadPacketsDelta)) * 100.0f + 0.5f);
}


static void tsrxtalk_parse(struct pios_tslrsdebug_state *state, uint8_t c)
{
    static uint16_t new_chan_fails_val;
    static int8_t   channel_cnt = -100;
    int i;

    switch (state->state) {
        case TSRX_BOOT:
            if (detect_str_eeprom(c) && !detect_str_contact(c)) {
                detect_frameduration(c);
#if 0   // TODO write TSLRS start message to buffer
                if (c == '\n' || c == '\r') {
                    if (c == '\n') osd.write('|');
                } else {
                    if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
                    osd.write(c);
                }
#endif
            }
        break;
        case TSRX_VERSION_CHECK:
            if (detect_str_eeprom(c) && detect_str_contact(c)) {
                state->version = TSRX_IDLE_FROM_V25;
            }
            state->state = state->version;
            tsrxtalk_parse(state, c);
        break;
        case TSRX_IDLE_OLDER:
            if (c < FIRST_CHANNEL)          // lower than known channels
                i = 0;
            else if (c > LAST_CHANNEL)      // higher than known channels
                i = TSRX_CHANNEL_MAX - 1;
            else                            // known channel
                i = c - FIRST_CHANNEL + 1;
            state->BadChannel++;
            state->BadChannelDelta++;
            state->BadChannelTime = xTaskGetTickCount();
            state->ChannelFails[i]++;
        break;
        case TSRX_IDLE_FROM_V25:
            switch (c) {
                case TOKEN_FAILSAVE:
                    state->state = TSRX_FAILSAVE_START;
                break;
                case TOKEN_GOOD:
                    state->state = TSRX_GOOD_START;
                break;
                case TOKEN_BAD:
                    state->state = TSRX_BAD_START;
                break;
                case TOKEN_VALUE:
                    state->state = TSRX_VALUE_START;
                break;
            }
        break;
        case TSRX_FAILSAVE_START:
        case TSRX_GOOD_START:
        case TSRX_BAD_START:
            switch (c) {
                case SUBTOKEN_FGB:
                    state->state++;
                    scan_value_clear();
                break;
                default:
                    state->state = state->version;
            }
        break;
        case TSRX_FAILSAVE_SCAN:
        case TSRX_GOOD_SCAN:
        case TSRX_BAD_SCAN:
            if (c >= '0' && c <= '9') {
                scan_value_add(c);
            } else {
                scan_value_set(state);
                state->state = state->version;
                tsrxtalk_parse(state, c);
            }
        break;
        case TSRX_VALUE_START:
            switch (c) {
                case SUBTOKEN_VALUE_ZERO:
                    channel_cnt = -1;
                    state->state = state->version;
                break;
                case SUBTOKEN_VALUE_DATA_1:
                    channel_cnt++;
                    if (channel_cnt < 0 || channel_cnt >= TSRX_CHANNEL_MAX) channel_cnt = -100;
                    state->state++;
                break;
                default:
                    state->state = state->version;
            }
        break;
        case TSRX_VALUE_READ_1:                 // hi byte
            new_chan_fails_val = c<<8;
            state->state++;
        break;
        case TSRX_VALUE_NEXT:
            switch (c) {
                case SUBTOKEN_VALUE_DATA_2:
                    state->state++;
                break;
                default:
                    state->state = state->version;
            }
        break;
        case TSRX_VALUE_READ_2:                 // lo byte
            new_chan_fails_val += c;
            state->state++;
        break;
        case TSRX_VALUE_PLOT:                   // plot marker
            LastPacketTime = xTaskGetTickCount();
            if (channel_cnt >= 0 && channel_cnt < TSRX_CHANNEL_MAX) {
                state->ChannelCount++;
                if (state->ChannelFails[channel_cnt] != new_chan_fails_val) {
                    uint16_t delta_channel_fails = new_chan_fails_val - state->ChannelFails[channel_cnt];
                    packet_window_set(PACKED_BAD, delta_channel_fails);
                    state->BadChannel += delta_channel_fails;
                    state->BadChannelDelta += delta_channel_fails;
                    state->BadChannelTime = xTaskGetTickCount();
                    state->ChannelFails[channel_cnt] = new_chan_fails_val;
                } else {
                    packet_window_set(PACKED_GOOD, 1);
                }
            }
            state->state = state->version;
        break;

        default:
            state->state = state->version;
    }
}

//---------------------------------

/* Allocate device descriptor */
#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_tslrsdebug_dev *PIOS_TSLRSdebug_Alloc(void)
{
    struct pios_tslrsdebug_dev *tslrsdebug_dev;

    tslrsdebug_dev = (struct pios_tslrsdebug_dev *)pvPortMalloc(sizeof(*tslrsdebug_dev));
    if (!tslrsdebug_dev) {
        return NULL;
    }
    tslrsdebug_state = &tslrsdebug_dev->state;

    tslrsdebug_dev->magic = PIOS_TSLRSDEBUG_DEV_MAGIC;
    return tslrsdebug_dev;
}
#else
static struct pios_tslrsdebug_dev pios_tslrsdebug_devs[1];
static uint8_t pios_tslrsdebug_num_devs;
static struct pios_tslrsdebug_dev *PIOS_TSLRSdebug_Alloc(void)
{
    struct pios_tslrsdebug_dev *tslrsdebug_dev;

    if (pios_tslrsdebug_num_devs >= 1) {
        return NULL;
    }

    tslrsdebug_dev = &pios_tslrsdebug_devs[pios_tslrsdebug_num_devs++];
    tslrsdebug_dev->magic = PIOS_TSLRSDEBUG_DEV_MAGIC;

    return tslrsdebug_dev;
}
#endif /* if defined(PIOS_INCLUDE_FREERTOS) */

/* Validate device descriptor */
static bool PIOS_TSLRSdebug_Validate(struct pios_tslrsdebug_dev *tslrsdebug_dev)
{
    return tslrsdebug_dev->magic == PIOS_TSLRSDEBUG_DEV_MAGIC;
}

/* Reset receiver state */
static void PIOS_TSLRSdebug_ResetState(struct pios_tslrsdebug_state *state)
{
    int i;
    for (i = 0; i < PACKET_WINDOW_MAX; i++) {
        PacketWindow[i] = PACKED_GOOD;
    }
    for (i = 0; i < TSRX_CHANNEL_MAX; i++) {
        state->ChannelFails[i] = 0;
    }
    state->state = TSRX_BOOT;
    state->version = TSRX_IDLE_OLDER;
    state->scan_value_percent = 0;
    state->ChannelCount = 0;
    state->BadChannelTime = 0;
    state->BadChannel = 0;
    state->BadChannelDelta = 0;
    state->Failsafes = 0;
    state->FailsafesDelta = 0;
    state->BadPackets = 0;
    state->BadPacketsDelta = 0;
    state->GoodPackets = 0;
    state->GoodPacketsDelta = 0;
    LastPacketTime = xTaskGetTickCount();
}

/* Initialise TSLRSdebug interface */
int32_t PIOS_TSLRSdebug_Init(uint32_t *tslrsdebug_id, const struct pios_tslrsdebug_cfg *cfg, const struct pios_com_driver *driver, uint32_t lower_id)
{
    PIOS_DEBUG_Assert(tslrsdebug_id);
    PIOS_DEBUG_Assert(cfg);
    PIOS_DEBUG_Assert(driver);

    struct pios_tslrsdebug_dev *tslrsdebug_dev;

    tslrsdebug_dev = (struct pios_tslrsdebug_dev *)PIOS_TSLRSdebug_Alloc();
    if (!tslrsdebug_dev) {
        goto out_fail;
    }

    /* Bind the configuration to the device instance */
    tslrsdebug_dev->cfg = cfg;

    PIOS_TSLRSdebug_ResetState(&(tslrsdebug_dev->state));

    *tslrsdebug_id = (uint32_t)tslrsdebug_dev;

    /* Set comm driver callback */
    (driver->bind_rx_cb)(lower_id, PIOS_TSLRSdebug_RxInCallback, *tslrsdebug_id);

    if (!PIOS_RTC_RegisterTickCallback(PIOS_TSLRSdebug_Supervisor, *tslrsdebug_id)) {
        PIOS_DEBUG_Assert(0);
    }

    return 0;
out_fail:
    return -1;
}


/* Comm byte received callback */
static uint16_t PIOS_TSLRSdebug_RxInCallback(uint32_t tslrsdebug_id, uint8_t *buf, uint16_t buf_len, uint16_t *headroom, bool *need_yield)
{
    struct pios_tslrsdebug_dev *tslrsdebug_dev = (struct pios_tslrsdebug_dev *)tslrsdebug_id;

    bool valid = PIOS_TSLRSdebug_Validate(tslrsdebug_dev);

    PIOS_Assert(valid);

    struct pios_tslrsdebug_state *state = &(tslrsdebug_dev->state);

    /* process byte(s) and clear receive timer */
    for (uint8_t i = 0; i < buf_len; i++) {
        tsrxtalk_parse(state, buf[i]);
    }

    /* reset and check version after boot time */
    if (state->state == TSRX_BOOT && xTaskGetTickCount() > BOOT_WAIT_TIME) {
        PIOS_TSLRSdebug_ResetState(state);
        state->state = TSRX_VERSION_CHECK;
    }

    /* clear BadChannelDelta after some time */
    if (state->BadChannelDelta && xTaskGetTickCount() > state->BadChannelTime + CHANNEL_DELTA_DURATION) {
        state->BadChannelDelta = 0;
    }

    /* Always signal that we can accept another byte */
    if (headroom) {
        *headroom = 1;
    }

    /* We never need a yield */
    *need_yield = false;

    /* Always indicate that all bytes were consumed */
    return buf_len;
}


/* Comm supervisor callback */
static void PIOS_TSLRSdebug_Supervisor(uint32_t tslrsdebug_id)
{
    struct pios_tslrsdebug_dev *tslrsdebug_dev = (struct pios_tslrsdebug_dev *)tslrsdebug_id;

    bool valid = PIOS_TSLRSdebug_Validate(tslrsdebug_dev);

    PIOS_Assert(valid);

    struct pios_tslrsdebug_state *state = &(tslrsdebug_dev->state);

    /* simulate bad channel if no packet was received for PacketTimeout ms */
    if (state->state >= TSRX_IDLE_FROM_V25 && xTaskGetTickCount() > LastPacketTime + PacketTimeout) {
        LastPacketTime = xTaskGetTickCount();
        packet_window_set(PACKED_BAD, 1);
        state->BadChannel++;
        state->BadChannelDelta++;
        state->BadChannelTime = xTaskGetTickCount();
    }
}

#endif /* PIOS_INCLUDE_TSLRSDEBUG */

/**
 * @}
 * @}
 */
