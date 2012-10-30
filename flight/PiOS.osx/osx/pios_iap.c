/*!
 * 	@File iap.c
 *	@Brief	
 *
 *  Created on: Sep 6, 2010
 *      Author: joe
 */


/****************************************************************************************
 *  Header files
 ****************************************************************************************/
#include <pios.h>

/*!
 * \brief	PIOS_IAP_Init - performs required initializations for iap module.
 * \param   none.
 * \return	none.
 * \retval	none.
 *
 *	Created: Sep 8, 2010 10:10:48 PM by joe
 */
void PIOS_IAP_Init( void )
{

}

/*!
 * \brief     Determines if an In-Application-Programming request has been made.
 * \param   *comm - Which communication stream to use for the IAP (USB, Telemetry, I2C, SPI, etc)
 * \return    TRUE - if correct sequence found, along with 'comm' updated.
 * 			FALSE - Note that 'comm' will have an invalid comm identifier.
 * \retval
 *
 */
uint32_t	PIOS_IAP_CheckRequest( void )
{

	return false;
}



/*!
 * \brief   Sets the 1st word of the request sequence.
 * \param   n/a
 * \return  n/a
 * \retval
 */
void	PIOS_IAP_SetRequest1(void)
{
}

void	PIOS_IAP_SetRequest2(void)
{
}

void	PIOS_IAP_ClearRequest(void)
{
}

uint16_t PIOS_IAP_ReadBootCount(void)
{
	return 0;
}

void PIOS_IAP_WriteBootCount (uint16_t boot_count)
{
}
