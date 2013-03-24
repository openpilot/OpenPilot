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
#if defined(STM32F4XX)
#define MAGIC_REG_1     RTC_BKP_DR1
#define MAGIC_REG_2     RTC_BKP_DR2
#define IAP_BOOTCOUNT   RTC_BKP_DR3
#define IAP_RESVD1		RTC_BKP_DR4
#define IAP_RESVD2		RTC_BKP_DR5
#define IAP_RESVD3		RTC_BKP_DR6
#define IAP_RESVD4		RTC_BKP_DR7
#define IAP_RESVD5		RTC_BKP_DR8
#define IAP_RESVD6		RTC_BKP_DR9
#define IAP_CMD1        RTC_BKP_DR10
#define IAP_CMD2        RTC_BKP_DR11
#define IAP_CMD3        RTC_BKP_DR12
#else
#define MAGIC_REG_1     BKP_DR1
#define MAGIC_REG_2     BKP_DR2
#define IAP_BOOTCOUNT   BKP_DR3
#define IAP_RESVD1		BKP_DR4
#define IAP_RESVD2		BKP_DR5
#define IAP_RESVD3		BKP_DR6
#define IAP_RESVD4		BKP_DR7
#define IAP_RESVD5		BKP_DR8
#define IAP_RESVD6		BKP_DR9
#define IAP_CMD1        BKP_DR10
#define IAP_CMD2        BKP_DR11
#define IAP_CMD3        BKP_DR12
#endif

#define PIOS_IAP_CLEAR_FLASH_CMD_0 0xFA5F
#define PIOS_IAP_CLEAR_FLASH_CMD_1 0x0001
#define PIOS_IAP_CLEAR_FLASH_CMD_2 0x0000

#define PIOS_IAP_CMD_COUNT 3

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

/**
  * @brief  Return one of the IAP command values passed from bootloader.
  * @param  number: the index of the command value (0..2).
  * @retval the selected command value.
  */
uint32_t PIOS_IAP_ReadBootCmd(uint8_t number);

/**
  * @brief  Write one of the IAP command values to be passed to firmware from bootloader.
  * @param  number: the index of the command value (0..2).
  * @param  value: value to be written.
  */
void PIOS_IAP_WriteBootCmd(uint8_t number, uint32_t value);
/****************************************************************************************
 *  Public Data
 ****************************************************************************************/

#endif /* PIOS_IAP_H_ */
