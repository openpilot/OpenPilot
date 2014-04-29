/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MSP MSP  functions
 * @brief Code for MSP
 * @{
 *
 * @file       pios_msp.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Code for MSP
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

#ifdef PIOS_INCLUDE_MSP

#include "pios_msp.h"
#include "pios_msp_priv.h"

static uint8_t blankserialRequest[6] = {'$', 'M', '<', 0, 0, 0};

static uint8_t serialBuffer[SERIALBUFFERSIZE];
static uint8_t receiverIndex;
static uint8_t dataSize;
static uint8_t rcvChecksum;
static uint8_t readIndex;

static uint8_t  MwArmed = 0;
static uint8_t  MwMode  = 0;
static uint16_t MwRcData[8] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};
static uint16_t MwAngle[2]  = {0, 0};


uint8_t read8()  {
    return serialBuffer[readIndex++];
}


uint16_t read16() {
    uint16_t t = read8();
    t |= (uint16_t) read8() << 8;
    return t;
}


uint32_t read32() {
    uint32_t t = read16();
    t |= (uint32_t) read16() << 16;
    return t;
}


void serialMSPCheck(uint8_t cmdMSP)
{
    uint8_t i;

    readIndex = 0;

    if (cmdMSP == MSP_STATUS)
    {
        uint32_t MwSensorActive;

        MwSensorActive = read16();      // dummy read
        MwSensorActive = read32();      // dummy read
        MwSensorActive = read32();
        MwArmed = MwSensorActive & MSP_MASK_BOXARM;
        MwMode  = MwSensorActive & MSP_MASK_BOXHORIZON ? 2 : MwSensorActive & MSP_MASK_BOXANGLE ? 1 : 0;
    }

    if (cmdMSP == MSP_RC)
    {
        for (i=0; i<8; i++)
            MwRcData[i] = read16();
    }

    if (cmdMSP == MSP_ATTITUDE)
    {
        for (i=0; i<2; i++)
            MwAngle[i] = read16();
    }
}


void MSPblankserialRequest(uint32_t port, uint8_t requestMSP)
{
    blankserialRequest[4] = requestMSP;
    blankserialRequest[5] = requestMSP;
    PIOS_COM_SendBuffer(port, blankserialRequest, 6);
}


void MSPRequests(uint32_t port)
{
    static portTickType last_time_hi = 0;
    static portTickType last_time_lo = 0;
    portTickType current_time = xTaskGetTickCount();

    if (current_time - last_time_hi >= REQUEST_TIME_HI) {
        last_time_hi = current_time;
        MSPblankserialRequest(port, MSP_ATTITUDE);
    }

    if (current_time - last_time_lo >= REQUEST_TIME_LO) {
        last_time_lo = current_time;
        MSPblankserialRequest(port, MSP_RC);
        MSPblankserialRequest(port, MSP_STATUS);
    }
}


void MSPInputStream(uint8_t c)
{
    static uint8_t cmdMSP;
    static enum _serial_state {
        IDLE,
        HEADER_START,
        HEADER_M,
        HEADER_ARROW,
        HEADER_SIZE,
        HEADER_CMD,
    } c_state = IDLE;

    if (c_state == IDLE) {
        c_state = (c == '$') ? HEADER_START : IDLE;
    }
    else if (c_state == HEADER_START) {
        c_state = (c == 'M') ? HEADER_M : IDLE;
    }
    else if (c_state == HEADER_M) {
        c_state = (c == '>') ? HEADER_ARROW : IDLE;
    }
    else if (c_state == HEADER_ARROW) {
        c_state = HEADER_SIZE;
        dataSize = c;
        rcvChecksum = c;
    }
    else if (c_state == HEADER_SIZE) {
        c_state = HEADER_CMD;
        cmdMSP = c;
        rcvChecksum ^= c;
        receiverIndex = 0;
    }
    else if (c_state == HEADER_CMD) {
        rcvChecksum ^= c;
        if (receiverIndex == dataSize) {
            if (rcvChecksum == 0) {
                serialMSPCheck(cmdMSP);
            }
            c_state = IDLE;
        }
        else {
            serialBuffer[receiverIndex++] = c;
        }
    }
}


uint8_t MSPGetArmed(void)
{
    return MwArmed;
}


uint8_t MSPGetMode(void)
{
    return MwMode;
}


uint16_t MSPGetRC(uint8_t i)
{
    return MwRcData[i];
}


uint16_t MSPGetAngle(uint8_t i)
{
    return MwAngle[i];
}


#endif /* PIOS_INCLUDE_MSP */

/**
 * @}
 * @}
 */
