/**
 ******************************************************************************
 *
 * @file       gps9gpshandler.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      handler for GPSV9 onboard ubx gps module.
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
#include <pios_ubx_ddc.h>

#include "gps9gpshandler.h"
#include "gps9protocol.h"

uint32_t lastUnsentData = 0;
uint8_t buffer[BUFFER_SIZE];

void handleGPS()
{
    bool completeSentenceSent = false;
    int8_t maxCount = 2;

    do {
        int32_t datacounter = PIOS_UBX_DDC_GetAvailableBytes(PIOS_I2C_GPS);
        if (datacounter > 0) {
            uint8_t toRead = (uint32_t)datacounter > BUFFER_SIZE - lastUnsentData ? BUFFER_SIZE - lastUnsentData : (uint8_t)datacounter;
            //uint8_t toRead = (uint32_t)datacounter > BUFFER_SIZE ? BUFFER_SIZE  : (uint8_t)datacounter;
            uint8_t toSend = toRead;
            PIOS_UBX_DDC_ReadData(PIOS_I2C_GPS, buffer, toRead);

            uint8_t *lastSentence;
            uint16_t lastSentenceLength;
            completeSentenceSent = ubx_getLastSentence(buffer, toRead, &lastSentence, &lastSentenceLength);
            if (completeSentenceSent) {
                toSend = (uint8_t)(lastSentence - buffer + lastSentenceLength);
            } else {
                lastUnsentData = 0;
            }

            PIOS_COM_SendBuffer(pios_com_main_id, buffer, toSend);

            if (toRead > toSend) {
                // move unsent data at the beginning of buffer to be sent next time
                lastUnsentData = toRead - toSend;
                memcpy(buffer, (buffer + toSend), lastUnsentData);
            }
        }

        datacounter = PIOS_COM_ReceiveBuffer(pios_com_main_id, buffer, BUFFER_SIZE, 0);
        if (datacounter > 0) {
            PIOS_UBX_DDC_WriteData(PIOS_I2C_GPS, buffer, datacounter);
        }
        if(maxCount){
            // Note: this delay is needed as querying too quickly the UBX module's I2C(DDC)
            // port causes a lot of weird issues (it stops sending nav sentences)
            vTaskDelay(2 * configTICK_RATE_HZ / 1000);
        }
    } while (maxCount--);
}

typedef struct {
    uint8_t size;
    const uint8_t *sentence;
} ubx_init_sentence;


void setupGPS()
{
    CfgPrtPkt cfgprt;

    cfgprt.fragments.data.portID       = CFG_PRT_DATA_PORTID_DDC;
    cfgprt.fragments.data.reserved0    = 0;
    cfgprt.fragments.data.txReady      = CFG_PRT_DATA_TXREADI_DISABLED;
    cfgprt.fragments.data.mode = CFG_PRT_DATA_MODE_ADDR;
    cfgprt.fragments.data.reserved3    = 0;
    cfgprt.fragments.data.inProtoMask  = CFG_PRT_DATA_PROTO_UBX | CFG_PRT_DATA_PROTO_NMEA |CFG_PRT_DATA_PROTO_RTCM;
    cfgprt.fragments.data.outProtoMask = CFG_PRT_DATA_PROTO_UBX;
    cfgprt.fragments.data.flags = 0;
    cfgprt.fragments.data.reserved5    = 0;

    ubx_buildPacket(&cfgprt.packet, UBX_CFG_CLASS, UBX_CFG_PRT, sizeof(CfgPrtData));
    PIOS_UBX_DDC_WriteData(PIOS_I2C_GPS, cfgprt.packet.binarystream, sizeof(CfgPrtPkt));
}
