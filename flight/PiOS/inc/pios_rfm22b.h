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

#include <packet_handler.h>

enum gpio_direction {GPIO0_TX_GPIO1_RX, GPIO0_RX_GPIO1_TX};

/* Global Types */
struct pios_rfm22b_cfg {
	const struct pios_spi_cfg * spi_cfg; /* Pointer to SPI interface configuration */
	const struct pios_exti_cfg * exti_cfg; /* Pointer to the EXTI configuration */
	uint32_t frequencyHz;
	uint32_t minFrequencyHz;
	uint32_t maxFrequencyHz;
	uint8_t RFXtalCap;
	uint32_t maxRFBandwidth;
	uint8_t maxTxPower;
	uint8_t slave_num;
	enum gpio_direction gpio_direction;
};

enum rfm22b_tx_power {
	RFM22_tx_pwr_txpow_0 = 0x00,  //  +1dBm .. 1.25mW
	RFM22_tx_pwr_txpow_1 = 0x01,  //  +2dBm .. 1.6mW
	RFM22_tx_pwr_txpow_2 = 0x02,  //  +5dBm .. 3.16mW
	RFM22_tx_pwr_txpow_3 = 0x03,  //  +8dBm .. 6.3mW
	RFM22_tx_pwr_txpow_4 = 0x04,  // +11dBm .. 12.6mW
	RFM22_tx_pwr_txpow_5 = 0x05,  // +14dBm .. 25mW
	RFM22_tx_pwr_txpow_6 = 0x06,  // +17dBm .. 50mW
	RFM22_tx_pwr_txpow_7 = 0x07   // +20dBm .. 100mW
};

enum rfm22b_datarate {
	RFM22_datarate_500 = 0,
	RFM22_datarate_1000 = 1,
	RFM22_datarate_2000 = 2,
	RFM22_datarate_4000 = 3,
	RFM22_datarate_8000 = 4,
	RFM22_datarate_9600 = 5,
	RFM22_datarate_16000 = 6,
	RFM22_datarate_19200 = 7,
	RFM22_datarate_24000 = 8,
	RFM22_datarate_32000 = 9,
	RFM22_datarate_64000 = 10,
	RFM22_datarate_128000 = 11,
	RFM22_datarate_192000 = 12,
	RFM22_datarate_256000 = 13,
};

/* Public Functions */
extern int32_t PIOS_RFM22B_Init(uint32_t *rfb22b_id, uint32_t spi_id, uint32_t slave_num, const struct pios_rfm22b_cfg *cfg);
extern void PIOS_RFM22B_SetTxPower(uint32_t rfm22b_id, enum rfm22b_tx_power tx_pwr);
extern void RFM22_SetDatarate(uint32_t rfm22b_id, enum rfm22b_datarate datarate, bool data_whitening);
extern void PIOS_RFM22B_SetDestinationId(uint32_t rfm22b_id, uint32_t dest_id);
extern uint32_t PIOS_RFM22B_DeviceID(uint32_t rfb22b_id);
extern uint16_t PIOS_RFM22B_Resets(uint32_t rfm22b_id);
extern uint16_t PIOS_RFM22B_Timeouts(uint32_t rfm22b_id);
extern uint8_t PIOS_RFM22B_LinkQuality(uint32_t rfm22b_id);
extern int8_t PIOS_RFM22B_RSSI(uint32_t rfm22b_id);
extern bool PIOS_RFM22B_Send_Packet(uint32_t rfm22b_id, PHPacketHandle p, uint32_t max_delay);
extern uint32_t PIOS_RFM22B_Receive_Packet(uint32_t rfm22b_id, PHPacketHandle *p, uint32_t max_delay);

#endif /* PIOS_RFM22B_H */

/**
  * @}
  * @}
  */
