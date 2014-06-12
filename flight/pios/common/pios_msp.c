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

static uint32_t outputPort = 0;

static uint8_t blankserialRequest[6] = {'$', 'M', '<', 0, 0, 0};
static uint8_t serialBuffer[SERIALBUFFERSIZE];
static uint8_t serialRequest[SERIALREQUESTSIZE];
static uint8_t receiverIndex;
static uint8_t dataSize;
static uint8_t rcvChecksum;
static uint8_t readIndex;

static uint32_t modeMSPRequests = 0;

static uint8_t  writeEEPROM = 0;

static uint8_t  MwProfile   = 0;
static uint8_t  MwArmed     = 0;
static uint8_t  MwMode      = 0;
static uint16_t MwRcData[8] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};
static uint16_t MwAngle[2]  = {0, 0};

static uint8_t  MwP[MSP_PIDITEMS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t  MwI[MSP_PIDITEMS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t  MwD[MSP_PIDITEMS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static uint8_t  MwRcRate        = 0;
static uint8_t  MwRcExpo        = 0;
static uint8_t  MwRollPitchRate = 0;
static uint8_t  MwYawRate       = 0;
static uint8_t  MwDynThrPID     = 0;
static uint8_t  MwThrMid        = 0;
static uint8_t  MwThrExpo       = 0;


char MSPText[TEXTROWS][TEXTCOLS] = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, };
static MSPConfig config;
static MSPPage page;


typedef struct {
    int8_t row_min, row_max;
    int8_t col_min, col_max;
} CursorTransitions;


CursorTransitions PageTransitions[MAXPAGE] =
{
        {
                2, 8,
                0, 2,
        },
        {
                2, 8,
                0, 0,
        },
};


void setPage(void) {
    switch (config.Page) {
    case 1:
        sprintf(MSPText[0], "1/2      PID CONFIG");
        sprintf(MSPText[1], "           P      I      D");
        sprintf(MSPText[2], "ROLL      %3u    %3u    %3u ", MwP[MSP_PID_ROLL],  MwI[MSP_PID_ROLL],  MwD[MSP_PID_ROLL]);
        sprintf(MSPText[3], "PITCH     %3u    %3u    %3u ", MwP[MSP_PID_PITCH], MwI[MSP_PID_PITCH], MwD[MSP_PID_PITCH]);
        sprintf(MSPText[4], "YAW       %3u    %3u    %3u ", MwP[MSP_PID_YAW],   MwI[MSP_PID_YAW],   MwD[MSP_PID_YAW]);
        sprintf(MSPText[5], "ALT       %3u    %3u    %3u ", MwP[MSP_PID_ALT],   MwI[MSP_PID_ALT],   MwD[MSP_PID_ALT]);
        sprintf(MSPText[6], "POS       %3u    %3u        ", MwP[MSP_PID_GPS],   MwI[MSP_PID_GPS]);
        sprintf(MSPText[7], "LEVEL     %3u    %3u    %3u ", MwP[MSP_PID_LEVEL], MwI[MSP_PID_LEVEL], MwD[MSP_PID_LEVEL]);
        sprintf(MSPText[8], "MAG       %3u               ", MwP[MSP_PID_MAG]);
        if (config.Row < ACTION_ROW) {
            MSPText[config.Row][ 9 + config.Col * 7] = '>';
            MSPText[config.Row][13 + config.Col * 7] = '<';
        }
        break;
    case 2:
        sprintf(MSPText[0], "2/2      RC TUNING");
        sprintf(MSPText[1], "                   VALUE");
        sprintf(MSPText[2], "ROLL PITCH RATE     %3u ", MwRollPitchRate);
        sprintf(MSPText[3], "YAW RATE            %3u ", MwYawRate);
        sprintf(MSPText[4], "TPA                 %3u ", MwDynThrPID);
        sprintf(MSPText[5], "THROTTLE MID        %3u ", MwThrMid);
        sprintf(MSPText[6], "THROTTLE EXPO       %3u ", MwThrExpo);
        sprintf(MSPText[7], "RC RATE             %3u ", MwRcRate);
        sprintf(MSPText[8], "RC EXPO             %3u ", MwRcExpo);
        if (config.Row < ACTION_ROW) {
            MSPText[config.Row][19] = '>';
            MSPText[config.Row][23] = '<';
        }
        break;
    case PAGE_SAVED:
        sprintf(MSPText[0], "            SAVE");
        sprintf(MSPText[1], " ");
        sprintf(MSPText[2], " ");
        sprintf(MSPText[3], " ");
        sprintf(MSPText[4], "        PAGE %u SAVED", config.PageSaved);
        sprintf(MSPText[5], " ");
        sprintf(MSPText[6], " ");
        sprintf(MSPText[7], " ");
        sprintf(MSPText[8], " ");
        sprintf(MSPText[ACTION_INDEX], "           > OK <");
        break;
    }
    if (config.Page != PAGE_SAVED) {
        sprintf(MSPText[ACTION_INDEX], "  PAGE      EXIT      SAVE ");
        if (config.Row == ACTION_ROW) {
            MSPText[ACTION_INDEX][1 + config.Col * 10] = '>';
            MSPText[ACTION_INDEX][6 + config.Col * 10] = '<';
        }
    }

    page.Mode = config.Mode;
}


void checkTransition(void) {
    int8_t pagenum = config.Page - 1;

    if (config.Row < PageTransitions[pagenum].row_min)      config.Row = PageTransitions[pagenum].row_min;
    if (config.Row == ACTION_ROW - 1)                       config.Row = PageTransitions[pagenum].row_max;
    if (config.Row > PageTransitions[pagenum].row_max)      config.Row = ACTION_ROW;

    if (config.Row < ACTION_ROW) {
        if (config.Col < PageTransitions[pagenum].col_min)  config.Col = PageTransitions[pagenum].col_min;
        if (config.Col > PageTransitions[pagenum].col_max)  config.Col = PageTransitions[pagenum].col_max;
    } else {
        if (config.Col < PAGE_COL)                          config.Col = PAGE_COL;
        if (config.Col > SAVE_COL)                          config.Col = SAVE_COL;
    }
}


void configExit(void) {
    config.Mode = 0;
    page.Mode = config.Mode;
    config.Page = MINPAGE;
    config.PageSaved = config.Page;
    config.Row = ACTION_ROW;
    config.Col = PAGE_COL;
}


void configSave(void) {
    uint8_t *tx = serialRequest;
    uint8_t txCheckSum;
    uint8_t txSize = 0;

    *tx++ = '$';
    *tx++ = 'M';
    *tx++ = '<';
    txCheckSum=0;
    switch (config.Page) {
    case 1:
        txSize = 30;
        *tx++ = txSize;
        txCheckSum ^= txSize;
        *tx++ = MSP_SET_PID;
        txCheckSum ^= MSP_SET_PID;
        for (uint8_t i=0; i<MSP_PIDITEMS; i++) {
            *tx++ = MwP[i];
            txCheckSum ^= MwP[i];
            *tx++ = MwI[i];
            txCheckSum ^= MwI[i];
            *tx++ = MwD[i];
            txCheckSum ^= MwD[i];
        }
        *tx++ = txCheckSum;
        break;
    case 2:
        txSize = 7;
        *tx++ = txSize;
        txCheckSum ^= txSize;
        *tx++ = MSP_SET_RC_TUNING;
        txCheckSum ^= MSP_SET_RC_TUNING;
        *tx++ = MwRcRate;
        txCheckSum ^= MwRcRate;
        *tx++ = MwRcExpo;
        txCheckSum ^= MwRcExpo;
        *tx++ = MwRollPitchRate;
        txCheckSum ^= MwRollPitchRate;
        *tx++ = MwYawRate;
        txCheckSum ^= MwYawRate;
        *tx++ = MwDynThrPID;
        txCheckSum ^= MwDynThrPID;
        *tx++ = MwThrMid;
        txCheckSum ^= MwThrMid;
        *tx++ = MwThrExpo;
        txCheckSum ^= MwThrExpo;
        *tx++ = txCheckSum;
        break;
    }

    if (outputPort && txSize) {
        PIOS_COM_SendBuffer(outputPort, serialRequest, txSize + 6);
        writeEEPROM = 3;

        config.PageSaved = config.Page;
        config.Page = PAGE_SAVED;
        config.Row = ACTION_ROW;
        config.Col = PAGE_COL;
    }
}


void configChange(int8_t value) {
    uint8_t *param = 0;
    uint8_t delta = PageTransitions[config.Page - 1].row_min;
    uint8_t access;
    uint8_t upper_limit = 255;
    uint8_t lower_limit = 0;

    switch (config.Page) {
    case 1:
        access = (config.Row - delta <= MSP_PID_GPS) ? config.Row - delta : config.Row;
        switch (config.Col) {
        case 0:
            param = &MwP[access];
            break;
        case 1:
            if (access != MSP_PID_MAG) param = &MwI[access];
            break;
        case 2:
            if (access != MSP_PID_MAG && access != MSP_PID_GPS) param = &MwD[access];
            break;
        }
        break;
    case 2:
        switch (config.Row - delta) {
        case 0:
            param = &MwRollPitchRate;
            break;
        case 1:
            param = &MwYawRate;
            break;
        case 2:
            param = &MwDynThrPID;
            break;
        case 3:
            param = &MwThrMid;
            upper_limit = 100;
            break;
        case 4:
            param = &MwThrExpo;
            upper_limit = 100;
            break;
        case 5:
            param = &MwRcRate;
            upper_limit = 250;
            break;
        case 6:
            param = &MwRcExpo;
            upper_limit = 100;
            break;
        }
        break;
    }

    if ((value > 0 && *param == upper_limit) || (value < 0 && *param == lower_limit)) return;
    if (param) *param = *param + value;
}


void handleRawRC(void) {
    static uint8_t waitStick = 0;
    static portTickType timeout = 1000;
    static portTickType stickTime = 0;
    static uint8_t init = 0;

    if (!init) {
        init++;
        config.Mode = 0;
        config.Page = MINPAGE;
        config.PageSaved = config.Page;
        config.Row = ACTION_ROW;
        config.Col = PAGE_COL;
        page.Mode = config.Mode;
        page.Rows = TEXTROWS;
        page.Cols = TEXTCOLS;
        page.Text = (char*) MSPText;
    }

    if (MwRcData[MSP_PITCH] > MEDSTICK - DELTASTICK && MwRcData[MSP_PITCH] < MEDSTICK + DELTASTICK &&
        MwRcData[MSP_ROLL]  > MEDSTICK - DELTASTICK && MwRcData[MSP_ROLL]  < MEDSTICK + DELTASTICK &&
        MwRcData[MSP_YAW]   > MEDSTICK - DELTASTICK && MwRcData[MSP_YAW]   < MEDSTICK + DELTASTICK
    ) {
            waitStick = 0;
            timeout = 1000;
    }
    else if (waitStick == 1) {
        if (config.Page == PAGE_SAVED)
            timeout = 1000;
        else
            timeout = 100;
        if ((xTaskGetTickCount() - stickTime) > timeout) waitStick = 0;
    }

    if (!waitStick) {
        if (!config.Mode &&
            MwRcData[MSP_PITCH]    > MAXSTICK &&
            MwRcData[MSP_YAW]      > MAXSTICK &&
            MwRcData[MSP_THROTTLE] > MINSTICK
        ) {                                                                             // enter config mode using stick combination
            waitStick = 2;                                                              // sticks must return to center before continue
            config.Mode = 1;
            modeMSPRequests = REQ_MSP_RC_TUNING | REQ_MSP_PID;
        }
        else if (config.Mode) {
            if (MwRcData[MSP_THROTTLE] < MINSTICK) {                                    // EXIT
                waitStick = 2;
                configExit();
            }
            else if (config.Page != PAGE_SAVED && MwRcData[MSP_ROLL] > MAXSTICK) {      // MOVE RIGHT
                waitStick = 1;
                config.Col++;
                checkTransition();
            }
            else if (config.Page != PAGE_SAVED && MwRcData[MSP_ROLL] < MINSTICK) {      // MOVE LEFT
                waitStick = 1;
                config.Col--;
                checkTransition();
            }
            else if (config.Page != PAGE_SAVED && MwRcData[MSP_PITCH] > MAXSTICK) {     // MOVE UP
                waitStick = 1;
                if (config.Row == ACTION_ROW) {
                    config.Row = PageTransitions[config.Page - 1].row_min;
                    config.Col = PAGE_COL;
                } else {
                    config.Row--;
                }
                checkTransition();
            }
            else if (config.Page != PAGE_SAVED && MwRcData[MSP_PITCH] < MINSTICK) {     // MOVE DOWN
                waitStick = 1;
                if (config.Row == PageTransitions[config.Page - 1].row_max) {
                    config.Row = ACTION_ROW;
                    config.Col = PAGE_COL;
                } else {
                    config.Row++;
                }
                checkTransition();
            }
            else if (MwRcData[MSP_YAW] > MAXSTICK || MwRcData[MSP_YAW] < MINSTICK) {    // CHANGE
                waitStick = 1;
                if (config.Row == ACTION_ROW) {
                    if (config.Col == PAGE_COL) {
                        if (config.Page == PAGE_SAVED) {
                            switch (config.PageSaved) {
                            case 1:
                                modeMSPRequests |= REQ_MSP_PID;
                                break;
                            case 2:
                                modeMSPRequests |= REQ_MSP_RC_TUNING;
                                break;
                            }
                            config.Page = config.PageSaved;
                        } else {
                            if (MwRcData[MSP_YAW] > MAXSTICK) {
                                config.Page++;
                                if (config.Page > MAXPAGE) config.Page = MAXPAGE;
                            } else {
                                config.Page--;
                                if (config.Page < MINPAGE) config.Page = MINPAGE;
                            }
                        }
                    }
                    if (config.Col == EXIT_COL) configExit();
                    if (config.Col == SAVE_COL) configSave();
                } else {
                    if (MwRcData[MSP_YAW] > MAXSTICK) {
                        configChange(+1);
                    } else {
                        configChange(-1);
                    }
                }
            }
        }
        if (waitStick == 1) stickTime = xTaskGetTickCount();
        if (config.Mode == 1) setPage();
    }
}


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
        MwProfile = read8() + 1;
        MwArmed = MwSensorActive & MSP_MASK_BOXARM;
        MwMode  = MwSensorActive & MSP_MASK_BOXHORIZON ? 2 : MwSensorActive & MSP_MASK_BOXANGLE ? 1 : 0;
    }

    if (cmdMSP == MSP_RC)
    {
        for (i=0; i<8; i++)
            MwRcData[i] = read16();
        if (!MwArmed && xTaskGetTickCount() > 5000) handleRawRC();
    }

    if (cmdMSP == MSP_ATTITUDE)
    {
        for (i=0; i<2; i++)
            MwAngle[i] = read16();
    }

    if (cmdMSP == MSP_PID)
    {
        /*
            baseflight code info:
                enum {
                    PIDROLL,
                    PIDPITCH,
                    PIDYAW,
                    PIDALT,
                    PIDPOS,
                    PIDPOSR,
                    PIDNAVR,
                    PIDLEVEL,
                    PIDMAG,
                    PIDVEL,
                    PIDITEMS
                };
         */
        for(i=0; i<MSP_PIDITEMS; i++) {
            MwP[i] = read8();
            MwI[i] = read8();
            MwD[i] = read8();
        }
        modeMSPRequests &= ~REQ_MSP_PID;
        setPage();
    }

    if (cmdMSP == MSP_RC_TUNING)
    {
        /*
            baseflight code info:
                serialize8(cfg.rcRate8);
                serialize8(cfg.rcExpo8);
                serialize8(cfg.rollPitchRate);
                serialize8(cfg.yawRate);
                serialize8(cfg.dynThrPID);
                serialize8(cfg.thrMid8);
                serialize8(cfg.thrExpo8);
        */
        MwRcRate        = read8();
        MwRcExpo        = read8();
        MwRollPitchRate = read8();
        MwYawRate       = read8();
        MwDynThrPID     = read8();
        MwThrMid        = read8();
        MwThrExpo       = read8();
        modeMSPRequests &= ~REQ_MSP_RC_TUNING;
        setPage();
    }
}


void MSPblankserialRequest(uint8_t requestMSP)
{
    blankserialRequest[4] = requestMSP;
    blankserialRequest[5] = requestMSP;
    if (outputPort) PIOS_COM_SendBuffer(outputPort, blankserialRequest, 6);
}


void MSPRequests(uint32_t port)
{
    static portTickType last_time_hi = 0;
    static portTickType last_time_lo = 0;
    portTickType current_time = xTaskGetTickCount();

    outputPort = port;

    if (current_time - last_time_hi >= REQUEST_TIME_HI) {
        last_time_hi = current_time;
        MSPblankserialRequest(MSP_ATTITUDE);
    }

    if (current_time - last_time_lo >= REQUEST_TIME_LO) {
        last_time_lo = current_time;
        MSPblankserialRequest(MSP_RC);
        MSPblankserialRequest(MSP_STATUS);

        if (modeMSPRequests & REQ_MSP_PID) {
            MSPblankserialRequest(MSP_PID);
        }

        if (modeMSPRequests & REQ_MSP_RC_TUNING) {
            MSPblankserialRequest(MSP_RC_TUNING);
        }

        if (writeEEPROM)
            if (writeEEPROM-- == 1)
                MSPblankserialRequest(MSP_EEPROM_WRITE);
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


MSPPage MSPGetPage(void)
{
    return page;
}


uint8_t MSPGetProfile(void)
{
    return MwProfile;
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
