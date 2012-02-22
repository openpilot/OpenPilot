/*!
 * 	@File iap.h
 *	@Brief	Header file for the In-Application-Programming Module
 *
 *  Created on: Sep 6, 2010
 *      Author: joe
 */

#ifndef PIOS_IAP_H_
#define PIOS_IAP_H_


/****************************************************************************************
 *  Header files
 ****************************************************************************************/

/*****************************************************************************************
 *	Public Definitions/Macros
 ****************************************************************************************/
#define MAGIC_REG_1		BKP_DR1
#define MAGIC_REG_2		BKP_DR2
#define IAP_BOOTCOUNT		BKP_DR3

/****************************************************************************************
 *  Public Functions
 ****************************************************************************************/
void		PIOS_IAP_Init(void);
uint32_t	PIOS_IAP_CheckRequest( void );
void		PIOS_IAP_SetRequest1(void);
void		PIOS_IAP_SetRequest2(void);
void		PIOS_IAP_ClearRequest(void);
uint16_t	PIOS_IAP_ReadBootCount(void);
void		PIOS_IAP_WriteBootCount(uint16_t);

/****************************************************************************************
 *  Public Data
 ****************************************************************************************/

#endif /* PIOS_IAP_H_ */
