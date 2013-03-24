/**
 ******************************************************************************
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board specific static initializers for hardware for the OpenPilot board.
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

#include <pios.h>
#include <pios_udp_priv.h>
#include <pios_com_priv.h>
#include <openpilot.h>
#include <uavobjectsinit.h>

/**
 * PIOS_Board_Init()
 * initializes all the core systems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {

	/* Delay system */
	PIOS_DELAY_Init();

	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();

	/* Initialize the alarms library */
	AlarmsInitialize();

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

	/* Initialize the PiOS library */
	PIOS_COM_Init();

}


const struct pios_udp_cfg pios_udp0_cfg = {
  .ip = "0.0.0.0",
  .port = 9000,
};
const struct pios_udp_cfg pios_udp1_cfg = {
  .ip = "0.0.0.0",
  .port = 9001,
};
const struct pios_udp_cfg pios_udp2_cfg = {
  .ip = "0.0.0.0",
  .port = 9002,
};

#ifdef PIOS_COM_AUX
/*
 * AUX USART
 */
const struct pios_udp_cfg pios_udp3_cfg = {
  .ip = "0.0.0.0",
  .port = 9003,
};
#endif

/*
 * Board specific number of devices.
 */
struct pios_udp_dev pios_udp_devs[] = {
#define PIOS_UDP_TELEM  0
  {
    .cfg = &pios_udp0_cfg,
  },
#define PIOS_UDP_GPS    1
  {
    .cfg = &pios_udp1_cfg,
  },
#define PIOS_UDP_LOCAL    2
  {
    .cfg = &pios_udp2_cfg,
  },
#ifdef PIOS_COM_AUX
#define PIOS_UDP_AUX    3
  {
    .cfg = &pios_udp3_cfg,
  },
#endif
};

uint8_t pios_udp_num_devices = NELEMENTS(pios_udp_devs);

/*
 * COM devices
 */

/*
 * Board specific number of devices.
 */
extern const struct pios_com_driver pios_serial_com_driver;
extern const struct pios_com_driver pios_udp_com_driver;

struct pios_com_dev pios_com_devs[] = {
  {
    .id     = PIOS_UDP_TELEM,
    .driver = &pios_udp_com_driver,
  },
  {
    .id     = PIOS_UDP_GPS,
    .driver = &pios_udp_com_driver,
  },
  {
    .id     = PIOS_UDP_LOCAL,
    .driver = &pios_udp_com_driver,
  },
#ifdef PIOS_COM_AUX
  {
    .id     = PIOS_UDP_AUX,
    .driver = &pios_udp_com_driver,
  },
#endif
};

const uint8_t pios_com_num_devices = NELEMENTS(pios_com_devs);

/**
 * @}
 */
