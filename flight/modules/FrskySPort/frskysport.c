/**
 ******************************************************************************
 *
 * @file       frskysport.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      This module does sends telemetry data to an S.Port compatible RX
 *
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


#include "inc/frskysport.h"

#include <openpilot.h>
#include <pios_port_abstraction.h>
#include <flightbatterystate.h>
#include <gpspositionsensor.h>
#include <barosensor.h>
#include <frskysportmodulesettings.h>
#include <pios_com.h>
#include <pios_com_priv.h>
#include <pios_usart.h>
#include <pios_usart_priv.h>
#include "inc/frskyprotocol.h"
//
// Configuration
//
#define SAMPLE_PERIOD_MS 500
#define UART_BUFFER_LEN (4 * sizeof(frskyPacket))
#define BAUD_RATE 57600
// Private constants

// Private types

// Private variables
static bool moduleEnabled = false;
static uint32_t usartId;
static uint32_t comPortId;
// Private functions
static void onTimer(UAVObjEvent *ev);

static void SendU32(uint16_t id, uint32_t value);
static void SendI32(uint16_t id, int32_t value);
//static void SendU8(uint16_t id, uint8_t value);

/**
 * Initialise the module, called on startup.
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t FrskySPortInitialize()
{
    FrskySportModuleSettingsData settings;
    // Check if there is any port defined for this module
    FrskySportModuleSettingsInitialize();
    FlightBatteryStateInitialize();
    GPSPositionSensorInitialize();

    FrskySportModuleSettingsGet(&settings);
    if(settings.SerialPort != FRSKYSPORTMODULESETTINGS_SERIALPORT_DISABLED){
        uint8_t serialPort = (settings.SerialPort - FRSKYSPORTMODULESETTINGS_SERIALPORT_SERIALPORT0);
        usartId = PIOS_PORT_ABSTRACTION_PickPort(serialPort);
        moduleEnabled = (usartId != 0);
    } else {
        moduleEnabled = false;
    }
    return 0;
}

int32_t FrskySPortStart(void)
{
    if(moduleEnabled){
        uint8_t *tx_buffer = (uint8_t *)pvPortMalloc(UART_BUFFER_LEN);
            PIOS_Assert(tx_buffer);
        if (PIOS_COM_Init(&comPortId, &pios_usart_com_driver, usartId, NULL, 0, tx_buffer, UART_BUFFER_LEN)) {
            PIOS_Assert(0);
        }
        PIOS_COM_ChangeBaud(comPortId, BAUD_RATE);
        UAVObjEvent ev;
        memset(&ev, 0, sizeof(UAVObjEvent));
        EventPeriodicCallbackCreate(&ev, onTimer, SAMPLE_PERIOD_MS / portTICK_RATE_MS);
    }
    return 0;
}

MODULE_INITCALL(FrskySPortInitialize, FrskySPortStart);

static void onTimer(__attribute__((unused)) UAVObjEvent *ev)
{
    static uint8_t lastItem = 0;
    // FILL_U8(packet, valueId, value)

    // simply send sequentially all the data for each sensor
    switch(lastItem++){
        case 0: // flightbattery data
            {
                float tmp;
                FlightBatteryStateVoltageGet(&tmp);
                SendU32(VFAS_FIRST_ID, (uint32_t)(tmp * 100));
                FlightBatteryStateCurrentGet(&tmp);
                SendU32(CURR_FIRST_ID, (uint32_t)(tmp * 100));
                FlightBatteryStateConsumedEnergyGet(&tmp);
                SendU32(FUEL_FIRST_ID, (uint32_t)(tmp * 1000));
            }
            break;
        case 1: // gps position
            break;
        case 2: // baro altitude
            {
                float tmp;
                BaroSensorAltitudeGet(&tmp);
                SendI32(CURR_FIRST_ID, (int32_t)tmp);
                lastItem = 0;
            }
            break;

    }
}

static void SendU32(uint16_t id, uint32_t value){
    frskyPacket packet;
    FILL_U32(packet, id,value);
    PIOS_COM_SendBufferNonBlocking(comPortId, (uint8_t *)&packet, sizeof(frskyPacket));
}

static void SendI32(uint16_t id, int32_t value){
    frskyPacket packet;
    FILL_I32(packet, id,value);
    PIOS_COM_SendBufferNonBlocking(comPortId, (uint8_t *)&packet, sizeof(frskyPacket));
}
/*
static void SendU8(uint16_t id, uint8_t value){
    frskyPacket packet;
    FILL_U8(packet, id,value);
    PIOS_COM_SendBufferNonBlocking(comPortId, (uint8_t *)&packet, sizeof(frskyPacket));
}
*/
