/**
 ******************************************************************************
 *
 * @file       openpilot.c 
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      Sets up ans runs main OpenPilot tasks.
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
#include "openpilot.h"


/* Local Variables */


/* Local Functions */


/**
* Main function
*/
void OpenPilotInit(void)
{
	/* Initialise Logging */
	OP_Logging_Init();
	
	
	/* Possible task setup:
		-> Supervisor: Monitors mainly which state the system should be in, starts/stops Manual/ARC/HARC tasks
			-> Manual: Simply passes servo inputs to servo outputs, maybe some logging too
			-> ARC: Assisted RC, fancy name for stabilisation only (aka. Auto1)
			-> HARC: Higly Assisted RC, fancy name for full autonomous (aka. Auto2)
		-> GPS: Simply parses incomming GPS data and puts it into the GPS struct, also set states of FIX/NOFIX etc.
		-> Gatekeepers: Tasks which use queue's to access shared resources
			-> MicroSD: Simply logs data to the MicroSD card
			-> Telemetry: Sends telemetry using a queue
		
		- Supervisor should have highest possibly priority (Idle + 14?)
		- Supervisor should also act as the warnings system, low batter etc)
		- Supervisor should handle all telemetry inputs (not outputs), and act accordingly
		- Sub tasks of the supervisor should have a priority just lower than the supervisor (Idle + 12?)
		- Sub tasks of the supervisor shoud ONLY be pre-empted by system interrupts such as UART, I2C etc
		- Gatekeepers should sit in a blocked state while there is nothing on the que, with a low priority
		- With the low priority of gatekeepers, they should only be running while the supervisor tasks are not working
		
		- I2C module sending and receiving needs to be included in here somwhere. Inputs would be interrupt triggered.
	*/

}
