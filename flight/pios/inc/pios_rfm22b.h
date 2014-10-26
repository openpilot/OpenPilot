/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_RFM22B Radio Functions
 * @brief PIOS interface for RFM22B Radio
 * @{
 *
 * @file       pios_rfm22b.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      RFM22B functions header.
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

#ifndef PIOS_RFM22B_H
#define PIOS_RFM22B_H

#include <uavobjectmanager.h>
#include <oplinksettings.h>

/* Constant definitions */
enum gpio_direction { GPIO0_TX_GPIO1_RX, GPIO0_RX_GPIO1_TX };
#define RFM22B_PPM_NUM_CHANNELS 8

/* Global Types */
struct pios_rfm22b_cfg {
    const struct pios_spi_cfg  *spi_cfg; /* Pointer to SPI interface configuration */
    const struct pios_exti_cfg *exti_cfg; /* Pointer to the EXTI configuration */
    uint8_t RFXtalCap;
    uint8_t slave_num;
    enum gpio_direction gpio_direction;
};
typedef void (*PPMReceivedCallback)(const int16_t *channels);

enum rfm22b_tx_power {
    RFM22_tx_pwr_txpow_0 = 0x00, // +1dBm .. 1.25mW
    RFM22_tx_pwr_txpow_1 = 0x01, // +2dBm .. 1.6mW
    RFM22_tx_pwr_txpow_2 = 0x02, // +5dBm .. 3.16mW
    RFM22_tx_pwr_txpow_3 = 0x03, // +8dBm .. 6.3mW
    RFM22_tx_pwr_txpow_4 = 0x04, // +11dBm .. 12.6mW
    RFM22_tx_pwr_txpow_5 = 0x05, // +14dBm .. 25mW
    RFM22_tx_pwr_txpow_6 = 0x06, // +17dBm .. 50mW
    RFM22_tx_pwr_txpow_7 = 0x07 // +20dBm .. 100mW
};

enum rfm22b_datarate {
    RFM22_datarate_9600   = 0,
    RFM22_datarate_19200  = 1,
    RFM22_datarate_32000  = 2,
    RFM22_datarate_57600  = 3,
    RFM22_datarate_64000  = 4,
    RFM22_datarate_100000 = 5,
    RFM22_datarate_128000 = 6,
    RFM22_datarate_192000 = 7,
    RFM22_datarate_256000 = 8,
};

typedef enum {
    PIOS_RFM22B_INT_FAILURE,
    PIOS_RFM22B_INT_SUCCESS,
    PIOS_RFM22B_TX_COMPLETE,
    PIOS_RFM22B_RX_COMPLETE
} pios_rfm22b_int_result;

struct rfm22b_stats {
    uint16_t packets_per_sec;
    uint16_t tx_byte_count;
    uint16_t rx_byte_count;
    uint16_t tx_seq;
    uint16_t rx_seq;
    uint8_t  rx_good;
    uint8_t  rx_corrected;
    uint8_t  rx_error;
    uint8_t  rx_missed;
    uint8_t  rx_failure;
    uint8_t  tx_dropped;
    uint8_t  tx_failure;
    uint8_t  resets;
    uint8_t  timeouts;
    uint8_t  link_quality;
    int8_t   rssi;
    int8_t   afc_correction;
    uint8_t  link_state;
};

/* Public Functions */
extern int32_t PIOS_RFM22B_Init(uint32_t *rfb22b_id, uint32_t spi_id, uint32_t slave_num, const struct pios_rfm22b_cfg *cfg);
extern void PIOS_RFM22B_Reinit(uint32_t rfb22b_id);
extern void PIOS_RFM22B_SetTxPower(uint32_t rfm22b_id, enum rfm22b_tx_power tx_pwr);
extern void PIOS_RFM22B_SetChannelConfig(uint32_t rfm22b_id, enum rfm22b_datarate datarate, uint8_t min_chan, uint8_t max_chan, uint8_t chan_set, bool coordinator, bool oneway, bool ppm_mode, bool ppm_only);
extern void PIOS_RFM22B_SetCoordinatorID(uint32_t rfm22b_id, uint32_t coord_id);
extern uint32_t PIOS_RFM22B_DeviceID(uint32_t rfb22b_id);
extern void PIOS_RFM22B_GetStats(uint32_t rfm22b_id, struct rfm22b_stats *stats);
extern uint8_t PIOS_RFM2B_GetPairStats(uint32_t rfm22b_id, uint32_t *device_ids, int8_t *RSSIs, uint8_t max_pairs);
extern bool PIOS_RFM22B_InRxWait(uint32_t rfb22b_id);
extern bool PIOS_RFM22B_LinkStatus(uint32_t rfm22b_id);
extern bool PIOS_RFM22B_ReceivePacket(uint32_t rfm22b_id, uint8_t *p);
extern bool PIOS_RFM22B_TransmitPacket(uint32_t rfm22b_id, uint8_t *p, uint8_t len);
extern pios_rfm22b_int_result PIOS_RFM22B_ProcessTx(uint32_t rfm22b_id);
extern pios_rfm22b_int_result PIOS_RFM22B_ProcessRx(uint32_t rfm22b_id);
extern void PIOS_RFM22B_SetPPMCallback(uint32_t rfm22b_id, PPMReceivedCallback cb);
extern void PIOS_RFM22B_PPMSet(uint32_t rfm22b_id, int16_t *channels);
extern void PIOS_RFM22B_PPMGet(uint32_t rfm22b_id, int16_t *channels);

/* Global Variables */
extern const struct pios_com_driver pios_rfm22b_com_driver;

#endif /* PIOS_RFM22B_H */

/**
 * @}
 * @}
 */
