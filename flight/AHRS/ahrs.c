/**
 ******************************************************************************
 *
 * @file       ahrs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Main AHRS functions
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


/* OpenPilot Includes */
#include "ahrs.h"

/* Global Variables */

/* Local Variables */

/* Function Prototypes */

/**
* AHRS Main function
*/
int main()
{
	/* Brings up System using CMSIS functions, enables the LEDs. */
	PIOS_SYS_Init();

	/* Delay system */
	PIOS_DELAY_Init();

	/* Communication system */
	PIOS_COM_Init();

	/* ADC system */
	PIOS_ADC_Init();

	/* Magnetic sensor system */
	PIOS_I2C_Init();
	PIOS_HMC5843_Init();

	/* Configure the HMC5843 Sensor */
	PIOS_HMC5843_ConfigTypeDef HMC5843_InitStructure;
	HMC5843_InitStructure.M_ODR = PIOS_HMC5843_ODR_10;
	HMC5843_InitStructure.Meas_Conf = PIOS_HMC5843_MEASCONF_NORMAL;
	HMC5843_InitStructure.Gain = PIOS_HMC5843_GAIN_2;
	HMC5843_InitStructure.Mode = PIOS_HMC5843_MODE_CONTINUOS;
	PIOS_HMC5843_Config(&HMC5843_InitStructure);

	uint8_t id[3] = {0};
	int16_t data[3] = {0};
	int32_t heading = 0;

	// Main loop
	for(;;) {
		// Alive signal
		PIOS_LED_Toggle(LED1);

		// Get 3 ID bytes
		PIOS_HMC5843_ReadID(id);

		// Get magnetic readings
		PIOS_HMC5843_ReadMag(data);

		// Calculate the heading
		heading = atan2((double)(data[0]), (double)(-1 * data[1])) * (180 / M_PI);
		if(heading < 0) heading += 360;

		// Output Heading data to
		PIOS_COM_SendFormattedString(COM_USART1, "Chip ID: %s\rHeading: %d\rRaw Mag Values: X=%d Y=%d Y=%d\r", id, heading, data[0], data[1], data[2]);

		// Test ADC
		PIOS_COM_SendFormattedString(COM_USART1, "ADC Values: %d,%d,%d,%d,%d,%d\r\n", PIOS_ADC_PinGet(0), PIOS_ADC_PinGet(1), PIOS_ADC_PinGet(2), PIOS_ADC_PinGet(3), PIOS_ADC_PinGet(4), PIOS_ADC_PinGet(5));

		PIOS_DELAY_WaitmS(250);
	}

	return 0;
}

