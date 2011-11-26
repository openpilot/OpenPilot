/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_DSM Spektrum/JR DSMx satellite receiver functions
 * @brief PIOS interface to bind and read Spektrum/JR DSMx satellite receiver
 * @{
 *
 * @file       pios_dsm_priv.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Spektrum/JR DSMx satellite receiver private structures.
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

#ifndef PIOS_DSM_PRIV_H
#define PIOS_DSM_PRIV_H

#include <pios.h>
#include <pios_stm32.h>
#include <pios_usart_priv.h>

/*
 * Currently known DSMx (DSM2, DSMJ, DSMX) satellite serial port settings:
 *  115200bps serial stream, 8 bits, no parity, 1 stop bit
 *  size of each frame: 16 bytes
 *  data resolution: 10 or 11 bits
 *  number of frames: 1 or 2
 *  frame period: 11ms or 22ms
 *
 * Currently known DSMx frame structure:
 *  2 bytes - depend on protocol version:
 *    for DSM2/DSMJ:
 *      1 byte - lost frame counter (8 bit)
 *      1 byte - data format (for master receiver bound with 3 or 5 pulses),
 *      	 or unknown (for slave receiver bound with 4 or 6 pulses,
 *               some sources call it also the lost frame counter)
 *    for DSMX:
 *      1 byte - unknown data (does not look like lost frame counter)
 *      1 byte - unknown data, has been seen only 0xB2 so far

 * 14 bytes - up to 7 channels (16 bit word per channel) with encoded channel
 * 	      number, channel value, the "2nd frame in a sequence" flag.
 * 	      Unused channels have FF FF instead of data bytes.
 *
 * Data format identification:
 *   - for DSM2/DSMJ: [0 0 0 R 0 0 N1 N0]
 *     where
 *   	  R is data resolution (0 - 10 bits, 1 - 11 bits),
 *   	  N1..N0 is the number of frames required to receive all channel
 *        data (01 or 10 are known to the moment, which means 1 or 2 frames).
 *        Three values for the transmitter information byte have been seen
 *        thus far: 0x01, 0x02, 0x12.
 *   - for DSMX this byte contains just 0xB2 or 0xA2 value for any resolution.
 *     It is not known at the moment how to find the exact resolution from the
 *     DSMX data stream. The frame number (1 or 2) and 10/11 bit resolution were
 *     found in different data streams. So it is safer at the moment to ask user
 *     explicitly choose the resolution.
 *     Also some weird throttle channel (0) behavior was found in some streams
 *     from DX8 transmitter (all zeroes). Thus DSMX needs special processing.
 *
 * Channel data are:
 * - for 10 bit: [F 0 C3 C2 C1 C0 D9 D8 D7 D6 D5 D4 D3 D2 D1 D0]
 * - for 11 bit: [F C3 C2 C1 C0 D10 D9 D8 D7 D6 D5 D4 D3 D2 D1 D0]
 *   where
 *   	F is normally 0 but set to 1 for the first channel of the 2nd frame,
 *      C3 to C0 is the channel number, 4 bit, zero-based, in any order,
 *      Dx..D0 - channel data (10 or 11 bits)
 *
 * DSM2 protocol bug: in some cases in 2-frame format some bytes of the
 * frame can contain invalid values from the previous frame. They usually
 * are the last 5 bytes and can be equal to FF or other data from last
 * frame. There is no explicit workaround currently known.
 *
 * Binding: the number of pulses within bind window after power up defines
 * if this receiver is a master (provides receiver capabilities info to
 * the transmitter to choose data format) or slave (does not respond to
 * the transmitter which falls back to the old DSM mode in that case).
 * Currently known are 3(4) pulses for low resolution (10 bit) DSM2 mode,
 * 5(6) pulses for high resolution (11 bit) DSM2 mode, and also 7(8) and
 * 9(10) pulses for DSMX modes. Thus only 3, 5, 7 or 9 pulses should be
 * used for stand-alone satellite receiver to be bound correctly as the
 * master.
 */

#define DSM_CHANNELS_PER_FRAME		7
#define DSM_FRAME_LENGTH		(1+1+DSM_CHANNELS_PER_FRAME*2)
#define DSM_DSM2_RES_MASK		0x0010
#define DSM_2ND_FRAME_MASK		0x8000

/*
 * Include lost frame counter and provide it as a last channel value
 * for debugging. Currently is not used by the receiver layer.
 */
//#define DSM_LOST_FRAME_COUNTER

/* DSM protocol variations */
enum pios_dsm_proto {
	PIOS_DSM_PROTO_DSM2,
	PIOS_DSM_PROTO_DSMX10BIT,
	PIOS_DSM_PROTO_DSMX11BIT,
};

/* DSM receiver instance configuration */
struct pios_dsm_cfg {
	struct stm32_gpio bind;
};

extern const struct pios_rcvr_driver pios_dsm_rcvr_driver;

extern int32_t PIOS_DSM_Init(uint32_t *dsm_id,
			     const struct pios_dsm_cfg *cfg,
			     const struct pios_com_driver *driver,
			     uint32_t lower_id,
			     enum pios_dsm_proto proto,
			     uint8_t bind);

#endif /* PIOS_DSM_PRIV_H */

/**
 * @}
 * @}
 */
