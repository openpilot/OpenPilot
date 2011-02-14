/**
 ******************************************************************************
 * @addtogroup OpenPilotBL OpenPilot BootLoader
 * @brief These files contain the code to the OpenPilot MB Bootloader.
 *
 * @{
 * @file       main.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      This is the file with the main function of the OpenPilot BootLoader
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
/* Bootloader Includes */
#include <pios.h>
#include <bl_array.h>
#define MAX_WRI_RETRYS 3
/* Prototype of PIOS_Board_Init() function */
extern void PIOS_Board_Init(void);
extern void FLASH_Download();
void error();

int main() {

	PIOS_SYS_Init();
	PIOS_Board_Init();
	uint32_t base_adress;
	base_adress=SCB->VTOR;
	if(0x08000000+(sizeof(dataArray)*4)>base_adress)
		error();

	for (int x=0;x<sizeof(dataArray);++x)
	{
		int result=0;
		for (int retry = 0; retry < MAX_WRI_RETRYS; ++retry) {
			if (result == 0) {
				result = (FLASH_ProgramWord(0x08000000+(x*4), dataArray[x]) == FLASH_COMPLETE) ? 1
						: 0;
			}
		}
		if(result==0)
			error();
	}
	for(int x=0;x<3;++x)
	{
		PIOS_DELAY_WaitmS(2000);
	}
	PIOS_SYS_Reset();
}
void error()
{
	for(;;)
	{

	}
}
