/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SRXL Multiplex SRXL receiver functions
 * @brief Code to read Multiplex SRXL receiver serial stream
 * @{
 *
 * @file       pios_srxl_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Code to read Multiplex SRXL receiver serial stream
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

#ifndef PIOS_SRXL_PRIV_H
#define PIOS_SRXL_PRIV_H

#include <pios.h>
#include <pios_stm32.h>
#include <pios_usart_priv.h>

/*
 * Multiplex SRXL serial port settings:
 *  115200bps inverted serial stream, 8 bits, no parity, 1 stop bits
 *  frame period is 14ms (FastResponse ON) or 21ms (FastResponse OFF)
 *
 * Frame structure:
 *  1 byte  - start and version
 *      0xa1 (v1 12-channels)
 *      0xa2 (v2 16-channels)
 * 24/32 bytes - channel data (4 + 12 bit/channel, 12/16 channels, MSB first)
 *      16 bits per channel. 4 first reserved/not used. 12 bits channel data in
 *          4095 steps, 0x000(800µs) - 0x800(1500µs) - 0xfff(2200µs)
 *  2 bytes checksum (calculated over all bytes including start and version)
 *
 * Checksum calculation:
 * u16 CRC16(u16 crc, u8 value) {
 *     u8 i;
 *     crc = crc ^ (s16)value << 8;
 *     for(i = 0; i < 8; i++) {
 *         if(crc & 0x8000) {
 *             crc = crc << 1 ^ 0x1021;
 *         } else {
 *             crc = crc << 1;
 *         }
 *     }
 *     return crc;
 * }
 */

#define SRXL_V1_HEADER             0xa1
#define SRXL_V2_HEADER             0xa2

#define SRXL_HEADER_LENGTH         1
#define SRXL_CHECKSUM_LENGTH       2
#define SRXL_V1_CHANNEL_DATA_BYTES (12 * 2)
#define SRXL_V2_CHANNEL_DATA_BYTES (16 * 2)
#define SRXL_FRAME_LENGTH          (SRXL_HEADER_LENGTH + SRXL_V2_CHANNEL_DATA_BYTES + SRXL_CHECKSUM_LENGTH)

/*
 * Multiplex SRXL protocol provides 16 proportional channels.
 * Do not change unless driver code is updated accordingly.
 */
#if (PIOS_SRXL_NUM_INPUTS != 16)
#error "Multiplex SRXL protocol provides 16 proportional channels."
#endif

extern const struct pios_rcvr_driver pios_srxl_rcvr_driver;

extern int32_t PIOS_SRXL_Init(uint32_t *srxl_id,
                              const struct pios_com_driver *driver,
                              uint32_t lower_id);

#endif /* PIOS_SRXL_PRIV_H */

/**
 * @}
 * @}
 */
