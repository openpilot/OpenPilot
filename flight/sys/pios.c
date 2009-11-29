
/**
 * Project: OpenPilot
 *    
 * @author The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2009.
 *    
 * @file pios.c
 * PiOS build, sets up main tasks, tickhook, and contains the Main function
 * It all starts from here
 *
 * @see The GNU Public License (GPL)
 */
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


/* Project Includes */
#include "pios.h"


/* FreeRTOS Includes */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>


/* FreeRTOS Prototypes */
void PiosMainTask( void *pvParameters );
void SensorTask( void *pvParameters );


int main()
{

  /* Setup Hardware */
  SysInit();

  /* Start Main tasks. */
  xTaskCreate( PiosMainTask, ( signed portCHAR * ) "PiosMain", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
  xTaskCreate( SensorTask, ( signed portCHAR * ) "Sensor", configMINIMAL_STACK_SIZE, NULL, 2, NULL );


  /* Start the scheduler. */
  vTaskStartScheduler();


  /* If all is well we will never reach here as the scheduler will now be
  running. */ 

  return 0;
	  			
}	            


/**
* Function Name  : PiosMainTask
* Description    : PIOS
* Input          : None
* Output         : None
* Return         : None
*/
void PiosMainTask( void *pvParameters )
{

    while(1)
        {
        }
}

/**
* Function Name  : SensorTask
* Description    : PIOS
* Input          : None
* Output         : None
* Return         : None
*/
void SensorTask( void *pvParameters )
{

    while(1)
        {
        }
}


