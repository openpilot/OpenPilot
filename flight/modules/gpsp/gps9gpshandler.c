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

// cfg-prt I2C. In UBX+RTCM, Out UBX, Slave Addr 0x42
const char cfg_settings[] = "\xB5\x62\x06\x00\x14\x00\x00\x00\x00\x00\x84\x00\x00\x00\x00\x00\x00\x00\x07\x00\x01\x00\x00\x00\x00\x00\xA6\xC6";

void handleGPS()
{
    bool completeSentenceSent = false;
    int8_t maxCount = 3;

    do {
        int32_t datacounter = PIOS_UBX_DDC_GetAvailableBytes(PIOS_I2C_GPS);
        if (datacounter > 0) {
            uint8_t toRead = (uint32_t)datacounter > BUFFER_SIZE - lastUnsentData ? BUFFER_SIZE - lastUnsentData : (uint8_t)datacounter;
            uint8_t toSend = toRead;
            PIOS_UBX_DDC_ReadData(PIOS_I2C_GPS, buffer, toRead);

            uint8_t *lastSentence;
            static uint16_t lastSentenceLenght;
            completeSentenceSent = ubx_getLastSentence(buffer, toRead, &lastSentence, &lastSentenceLenght);
            if (completeSentenceSent) {
                toSend = (uint8_t)(lastSentence - buffer + lastSentenceLenght);
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
    } while (maxCount--);
}

typedef struct {
    uint8_t size;
    const uint8_t *sentence;
} ubx_init_sentence;

const ubx_init_sentence gps_config[] = {
    [0] = {
        .sentence = (uint8_t *)cfg_settings,
        .size     = sizeof(cfg_settings),
    },
};


void setupGPS()
{
    for (uint8_t i = 0; i < NELEMENTS(gps_config); i++) {
        PIOS_UBX_DDC_WriteData(PIOS_I2C_GPS, gps_config[i].sentence, gps_config[i].size);
    }
}
