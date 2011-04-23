/**
 ******************************************************************************
 *
 * @file       firmwareiap.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      In Application Programming module to support firmware upgrades by
 * 				providing a means to enter the bootloader.
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
#include <stdint.h>

#include "pios.h"
#include "openpilot.h"
#include "firmwareiap.h"
#include "firmwareiapobj.h"

// Private constants
#define IAP_CMD_STEP_1      1122
#define IAP_CMD_STEP_2      2233
#define IAP_CMD_STEP_3      3344

#define IAP_CMD_CRC         100
#define IAP_CMD_VERIFY      101
#define IAP_CMD_VERSION	    102

#define IAP_STATE_READY     0
#define IAP_STATE_STEP_1    1
#define IAP_STATE_STEP_2    2
#define IAP_STATE_RESETTING 3

#define RESET_DELAY         500 /* delay between sending reset ot INS */

#define TICKS2MS(t)	((t)/portTICK_RATE_MS)
#define MS2TICKS(m)	((m)*portTICK_RATE_MS)

const uint32_t    iap_time_2_low_end = 500;
const uint32_t    iap_time_2_high_end = 5000;
const uint32_t    iap_time_3_low_end = 500;
const uint32_t    iap_time_3_high_end = 5000;

// Private types

// Private variables
// TODO: Ed: those two variables below are not used, shall we remove them ?
//const static uint8_t 	version[] = { 0, 0, 1 };
//const static uint16_t	SVN = 12345;
static uint8_t reset_count = 0;
static portTickType lastResetSysTime;

// Private functions
static void FirmwareIAPCallback(UAVObjEvent* ev);

static uint32_t	iap_calc_crc(void);

static void read_description(uint8_t *);

FirmwareIAPObjData 	data;

static uint32_t	get_time(void);

// Private types

// Private functions
static void resetTask(UAVObjEvent *);

/**
 * Initialise the module, called on startup.
 * \returns 0 on success or -1 if initialisation failed
 */

/*!
 * \brief   Performs object initialization functions.
 * \param   None.
 * \return  0 - under all cases
 *
 * \note
 *
 */

int32_t FirmwareIAPInitialize()
{
	data.BoardType= BOARD_TYPE;
	read_description(data.Description);
	PIOS_SYS_SerialNumberGetBinary(data.CPUSerial);
	data.BoardRevision= BOARD_REVISION;
	data.ArmReset=0;
	data.crc = 0;
	FirmwareIAPObjSet( &data );
	FirmwareIAPObjConnectCallback( &FirmwareIAPCallback );
	return 0;
}

/*!
 * \brief	FirmwareIAPCallback - callback function for firmware IAP requests
 * \param[in]  ev - pointer objevent
 * \retval	None.
 *
 * \note
 *
 */
static uint8_t    iap_state = IAP_STATE_READY;
static void FirmwareIAPCallback(UAVObjEvent* ev)
{
	static uint32_t   last_time = 0;
	uint32_t          this_time;
	uint32_t          delta;
	
	if(iap_state == IAP_STATE_RESETTING)
		return;
	
	if ( ev->obj == FirmwareIAPObjHandle() )	{
		// Get the input object data
		FirmwareIAPObjGet(&data);
		this_time = get_time();
		delta = this_time - last_time;
		last_time = this_time;
		if((data.BoardType==BOARD_TYPE)&&(data.crc != iap_calc_crc()))
		{
			read_description(data.Description);
			PIOS_SYS_SerialNumberGetBinary(data.CPUSerial);
			data.BoardRevision=BOARD_REVISION;
			data.crc = iap_calc_crc();
			FirmwareIAPObjSet( &data );
		}
		if((data.ArmReset==1)&&(iap_state!=IAP_STATE_RESETTING))
		{
			data.ArmReset=0;
			FirmwareIAPObjSet( &data );
		}
		switch(iap_state) {
			case IAP_STATE_READY:
				if( data.Command == IAP_CMD_STEP_1 ) {
					iap_state = IAP_STATE_STEP_1;
				}            break;
			case IAP_STATE_STEP_1:
				if( data.Command == IAP_CMD_STEP_2 ) {
					if( delta > iap_time_2_low_end && delta < iap_time_2_high_end ) {
						iap_state = IAP_STATE_STEP_2;
					} else {
						iap_state = IAP_STATE_READY;
					}
				} else {
					iap_state = IAP_STATE_READY;
				}
				break;
			case IAP_STATE_STEP_2:
				if( data.Command == IAP_CMD_STEP_3 ) {
					if( delta > iap_time_3_low_end && delta < iap_time_3_high_end ) {
						// we've met the three sequence of command numbers
						// we've met the time requirements.
						PIOS_IAP_SetRequest1();
						PIOS_IAP_SetRequest2();
						
						/* Note: Cant just wait timeout value, because first time is randomized */
						reset_count = 0;
						lastResetSysTime = xTaskGetTickCount();
						UAVObjEvent * ev = pvPortMalloc(sizeof(UAVObjEvent));
						memset(ev,0,sizeof(UAVObjEvent));
						EventPeriodicCallbackCreate(ev, resetTask, 100);
						iap_state = IAP_STATE_RESETTING;
					} else {
						iap_state = IAP_STATE_READY;
					}
				} else {
					iap_state = IAP_STATE_READY;
				}
				break;
			case IAP_STATE_RESETTING:
				// stay here permanentally, should reboot
				break;
			default:
				iap_state = IAP_STATE_READY;
				last_time = 0; // Reset the time counter, as we are not doing a IAP reset
				break;
		}
	}
}



// Returns number of milliseconds from the start of the kernel.

/*!
 * \brief   Returns number of milliseconds from the start of the kernel
 * \param   None.
 * \return  number of milliseconds from the start of the kernel.
 *
 * \note
 *
 */

static uint32_t get_time(void)
{
	portTickType	ticks;
	
	ticks = xTaskGetTickCount();
	
	return TICKS2MS(ticks);
}



/*!
 * \brief   Calculate the CRC value of the code in flash.
 * \param   None
 * \return  calculated CRC value using STM32's builtin CRC hardware
 *
 * \note
 *	I copied this function as the function crc calc function in pios_bl_helper.c
 *	is only included when the PIOS_BL_HELPER is defined, but this also includes
 *	the flash unlock and erase functions.  It is safer to only have the flash
 *	functions in the bootloader.
 *
 */

static uint32_t iap_calc_crc(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
	CRC_ResetDR();
	CRC_CalcBlockCRC((uint32_t *) START_OF_USER_CODE, (SIZE_OF_CODE) >> 2);
	return CRC_GetCRC();
}
static uint8_t *FLASH_If_Read(uint32_t SectorAddress)
{
	return (uint8_t *) (SectorAddress);
}
static void read_description(uint8_t * array)
{
	uint8_t x = 0;
	for (uint32_t i = START_OF_USER_CODE + SIZE_OF_CODE; i < START_OF_USER_CODE + SIZE_OF_CODE + FIRMWAREIAPOBJ_DESCRIPTION_NUMELEM; ++i) {
		array[x] = *FLASH_If_Read(i);
		++x;
	}
}

/**
 * Executed by event dispatcher callback to reset INS before resetting OP 
 */
static void resetTask(UAVObjEvent * ev)
{		
	PIOS_LED_Toggle(LED1);
#if (PIOS_LED_NUM > 1)
	PIOS_LED_Toggle(LED2);
#endif
	
	if((portTickType) (xTaskGetTickCount() - lastResetSysTime) > RESET_DELAY / portTICK_RATE_MS) {
		lastResetSysTime = xTaskGetTickCount();
		data.BoardType=0xFF;
		data.ArmReset=1;
		data.crc=reset_count; /* Must change a value for this to get to INS */
		FirmwareIAPObjSet(&data);
		++reset_count;
		if(reset_count>3)
		{
			PIOS_SYS_Reset();
		}
	}
}
