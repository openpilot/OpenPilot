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

/****************************************************************************************
 *  Private Definitions/Macros
 ****************************************************************************************/

/* these definitions reside here for protection and privacy. */
#define IAP_MAGIC_WORD_1	0x1122
#define IAP_MAGIC_WORD_2	0xAA55

#define IAP_REQLOC_1		BKP_DR1
#define IAP_CRCLOC_LOW		BKP_DR2
#define IAP_CRCLOC_UPPER	BKP_DR3
#define IAP_PORTLOC			BKP_DR4
#define IAP_REQLOC_2		BKP_RR5

#define IAP_UPLOAD_REQ_1	0x20AA
#define	IAP_UPLOAD_REQ_2	0x2055
#define IAP_DNLOAD_REQ_1	0x30AA
#define IAP_DNLOAD_REQ_2	0x3055

#define UPPERWORD16(lw)	(uint16_t)((uint32_t)(lw)>>16)
#define LOWERWORD16(lw)	(uint16_t)((uint32_t)(lw)&0x0000ffff)
#define UPPERBYTE(w)	(uint8_t)((w)>>8)
#define LOWERBYTE(w)	(uint8_t)((w)&0x00ff)

/****************************************************************************************
 *  Private Functions
 ****************************************************************************************/

/****************************************************************************************
 *  Private (static) Data
 ****************************************************************************************/

/****************************************************************************************
 *  Public/Global Data
 ****************************************************************************************/

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
#if 0
	/* Enable CRC clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);

	/* Enable PWR and BKP clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Enable write access to Backup domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Clear Tamper pin Event(TE) pending flag */
	BKP_ClearFlag();
#endif
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
#if 0	uint32_t	retval = FALSE;
	uint16_t	reg1;
	uint16_t	reg2;

	reg1 = BKP_ReadBackupRegister( MAGIC_REG_1 );
	reg2 = BKP_ReadBackupRegister( MAGIC_REG_2 );

	if( reg1 == IAP_MAGIC_WORD_1 && reg2 == IAP_MAGIC_WORD_2 ) {
		// We have a match.
		retval = TRUE;
	} else {
		retval = FALSE;
	}
	return retval;
#endif
}



/*!
 * \brief   Sets the 1st word of the request sequence.
 * \param   n/a
 * \return  n/a
 * \retval
 */
void	PIOS_IAP_SetRequest1(void)
{
#if 0
	BKP_WriteBackupRegister( MAGIC_REG_1, IAP_MAGIC_WORD_1);
#endif
}

void	PIOS_IAP_SetRequest2(void)
{
#if 0
	BKP_WriteBackupRegister( MAGIC_REG_2, IAP_MAGIC_WORD_2);
#endif
}

void	PIOS_IAP_ClearRequest(void)
{
#if 0
	BKP_WriteBackupRegister( MAGIC_REG_1, 0);
	BKP_WriteBackupRegister( MAGIC_REG_2, 0);
#endif
}
