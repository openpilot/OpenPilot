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
#define IAP_COMM		BKP_DR3

#define IAP_COMM_INVALID	0
#define IAP_COMM_USB		1
#define IAP_COMM_TELEMETRY	2
#define IAP_COMM_SPI_AHRS	3
#define IAP_COMM_I2C		4
// Additional types can be added along with the proper support code.

/****************************************************************************************
 *  Public Functions
 ****************************************************************************************/
void		PIOS_IAP_Init(void);
uint32_t 	PIOS_IAP_CRCVerify( void );
uint32_t	PIOS_IAP_CheckRequest( void );
void 		PIOS_IAP_SetCommInput( uint16_t comm );
uint16_t 	PIOS_IAP_GetCommInput( void );
void		PIOS_IAP_SetRequest1(void);
void		PIOS_IAP_SetRequest2(void);
void		PIOS_IAP_ClearRequest(void);
void 		PIOS_IAP_SetCRC( uint32_t crcval );

/****************************************************************************************
 *  Public Data
 ****************************************************************************************/

#endif /* PIOS_IAP_H_ */
