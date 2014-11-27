/**
 ******************************************************************************
 *
 * @file       gps9maghandler.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      handles GPSV9 onboard magnetometer and sends its data.
 *             --
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
#include <openpilot.h>
#include <pios_struct_helper.h>
#include <pios_helpers.h>
#include <ubx_utils.h>
#include <pios_hmc5x83.h>
#include "inc/gps9protocol.h"
#define MAG_RATE_HZ 30
extern pios_hmc5x83_dev_t onboard_mag;

void handleMag()
{
#ifdef PIOS_HMC5X83_HAS_GPIOS
    if (!PIOS_HMC5x83_NewDataAvailable(onboard_mag)) {
        return;
    }
#else
    static uint32_t lastUpdate = 0;
    if (PIOS_DELAY_DiffuS(lastUpdate) < (1000000 / MAG_RATE_HZ)) {
        return;
    }
    lastUpdate = PIOS_DELAY_GetRaw();
#endif
    static int16_t mag[3];

    if (PIOS_HMC5x83_ReadMag(onboard_mag, mag) == 0) {
        MagUbxPkt magPkt;
        // swap axis so that if side with connector is aligned to revo side with connectors, mags data are aligned
        magPkt.fragments.data.X = -mag[1];
        magPkt.fragments.data.Y = mag[0];
        magPkt.fragments.data.Z = mag[2];
        magPkt.fragments.data.status = 1;
        ubx_buildPacket(&magPkt.packet, UBX_OP_CUST_CLASS, UBX_OP_MAG, sizeof(MagData));
        PIOS_COM_SendBuffer(pios_com_main_id, magPkt.packet.binarystream, sizeof(MagUbxPkt));
        return;
    }
}
