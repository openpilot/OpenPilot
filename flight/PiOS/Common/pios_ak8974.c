/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_AK8794 AK8794 Magnetic Compass Functions
 * @brief Deals with the hardware interface to the magnetometers
 * @{
 *
 * @file       pios_ak8794.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      AK8794 Magnetic Sensor Functions from AHRS
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
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

#if defined(PIOS_INCLUDE_AK8974)

/*
 * AK8974 registers. Based on a Linux kernel patch (akm8974.c) and the datasheet
 * for AKM8975 which is a similiar device
 */

#define AK8974_SELFTEST	 			0x0C
#define AK8974_INFO	 				0x0D
#define AK8974_WHOAMI	 			0x0F
#define AK8974_DATA_X	 			0x10
#define AK8974_DATA_Y				0x12
#define AK8974_DATA_Z				0x14
#define AK8974_INT_SRC				0x16
#define AK8974_STATUS	 			0x18
#define AK8974_INT_CLEAR			0x1A
#define AK8974_CTRL1	 			0x1B
#define AK8974_CTRL2	 			0x1C
#define AK8974_CTRL3	 			0x1D
#define AK8974_INT_CTRL				0x1E
#define AK8974_OFFSET_X				0x20
#define AK8974_OFFSET_Y				0x22
#define AK8974_OFFSET_Z				0x24
#define AK8974_INT_THRES			0x26 /* absolute any axis value threshold */
#define AK8974_PRESET				0x30
#define AK8974_TEMP					0x31

#define AK8974_SELFTEST_IDLE		0x55
#define AK8974_SELFTEST_OK			0xAA

#define AK8974_WHOAMI_VALUE_AK8974 0x48
#define AK8974_INT_X_HIGH			0x80 /* Axis over +threshold */
#define AK8974_INT_Y_HIGH			0x40
#define AK8974_INT_Z_HIGH			0x20
#define AK8974_INT_X_LOW			0x10 /* Axis below -threshold	*/
#define AK8974_INT_Y_LOW			0x08
#define AK8974_INT_Z_LOW			0x04
#define AK8974_INT_RANGE			0x02 /* Range overflow (any axis) */

#define AK8974_STATUS_DRDY			0x40 /* Data ready	 */
#define AK8974_STATUS_OVERRUN		0x20 /* Data overrun	 */
#define AK8974_STATUS_INT			0x10 /* Interrupt occurred */

#define AK8974_CTRL1_POWER			0x80 /* 0 = standby; 1 = active */
#define AK8974_CTRL1_RATE			0x10 /* 0 = 10 Hz; 1 = 20 Hz	 */
#define AK8974_CTRL1_FORCE_EN		0x02 /* 0 = normal; 1 = force	 */
#define AK8974_CTRL1_MODE2			0x01 /* 0 */

#define AK8974_CTRL2_INT_EN		0x10 /* 1 = enable interrupts	 */
#define AK8974_CTRL2_DRDY_EN		0x08 /* 1 = enable data ready signal */
#define AK8974_CTRL2_DRDY_POL		0x04 /* 1 = data ready active high */
#define AK8974_CTRL2_RESDEF		(AK8974_CTRL2_DRDY_POL)

#define AK8974_CTRL3_RESET			0x80 /* Software reset	 */
#define AK8974_CTRL3_FORCE			0x40 /* Start forced measurement */
#define AK8974_CTRL3_SELFTEST		0x10 /* Set selftest register	 */
#define AK8974_CTRL3_RESDEF		0x00

#define AK8974_INT_CTRL_XEN		0x80 /* Enable interrupt for this axis */
#define AK8974_INT_CTRL_YEN		0x40
#define AK8974_INT_CTRL_ZEN		0x20
#define AK8974_INT_CTRL_XYZEN		0xE0
#define AK8974_INT_CTRL_POL		0x08 /* 0 = active low; 1 = active high */
#define AK8974_INT_CTRL_PULSE		0x02 /* 0 = latched;	 1 = pulse (50 usec) */
#define AK8974_INT_CTRL_RESDEF	(AK8974_INT_CTRL_XYZEN | AK8974_INT_CTRL_POL)

#define AK8974_INT_SCR_MROI		0x02


typedef struct {
	uint8_t CTRL1;
	uint8_t CTRL2;
	uint8_t CTRL3;
	uint8_t INT_CTRL;
	uint8_t Status;
} PIOS_AK8974_ConfigTypeDef;

/* Local Variables */
volatile bool pios_AK8974_data_ready;

/* static void PIOS_AK8974_Config(PIOS_AK8974_ConfigTypeDef * AK8974_Config_Struct); */
static bool PIOS_AK8974_Read(uint8_t address, uint8_t * buffer, uint8_t len);
static bool PIOS_AK8974_Write(uint8_t address, uint8_t buffer);

/**
  * @brief Initialise the AK8794 sensor
  */
void PIOS_AK8974_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable DRDY GPIO clock */
	RCC_APB2PeriphClockCmd(PIOS_AK8974_DRDY_CLK | RCC_APB2Periph_AFIO, ENABLE);

	/* Configure EOC pin as input floating */
	GPIO_InitStructure.GPIO_Pin = PIOS_AK8974_DRDY_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

	GPIO_Init(PIOS_AK8974_DRDY_GPIO_PORT, &GPIO_InitStructure);

	/* Configure the End Of Conversion (EOC) interrupt */
	GPIO_EXTILineConfig(PIOS_AK8974_DRDY_PORT_SOURCE, PIOS_AK8974_DRDY_PIN_SOURCE);
	EXTI_InitStructure.EXTI_Line = PIOS_AK8974_DRDY_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EOC EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_AK8974_DRDY_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_AK8974_DRDY_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure the AK8974 Sensor */

	/* soft reset */
//	while (!PIOS_AK8974_Write(AK8974_CTRL3, AK8974_CTRL3_RESET)) ;

	while (!PIOS_AK8974_Write(AK8974_CTRL1, AK8974_CTRL1_POWER|AK8974_CTRL1_FORCE_EN ));
	while (!PIOS_AK8974_Write(AK8974_CTRL1, AK8974_CTRL1_POWER|AK8974_CTRL1_RATE ));


	while (!PIOS_AK8974_Write(AK8974_CTRL2, AK8974_CTRL2_DRDY_EN|AK8974_CTRL2_DRDY_POL)) ;

	while (!PIOS_AK8974_Write(AK8974_CTRL3, AK8974_CTRL3_RESDEF)) ;
	while (!PIOS_AK8974_Write(AK8974_INT_CTRL, AK8974_INT_CTRL_PULSE)) ;

	pios_AK8974_data_ready = false;

//	while (!PIOS_AK8974_Write(AK8974_CTRL3, AK8974_CTRL3_FORCE)) ;

}
/*
static void PIOS_AK8974_Config(PIOS_AK8974_ConfigTypeDef * AK8974_Config_Struct)
{


	while (!PIOS_AK8974_Write(AK8974_CTRL1, (uint8_t)AK8974_Config_Struct->CTRL1)) ;

	while (!PIOS_AK8974_Write(AK8974_CTRL2, (uint8_t)AK8974_Config_Struct->CTRL2)) ;

	while (!PIOS_AK8974_Write(AK8974_CTRL3, (uint8_t)AK8974_Config_Struct->CTRL3)) ;


}
*/
/**
* Read the magnetic readings from the sensor
*/
void PIOS_AK8974_ReadMag(int16_t out[3])
{
//	uint8_t buffer[6];

	pios_AK8974_data_ready = false;

/* start reading at AK8974_DATA_X and implicit roll over to AK8974_DATA_Y and AK8974_DATA_Z */

	while (!PIOS_AK8974_Read(AK8974_DATA_X, (uint8_t *)out, 6)) ;

	//    out[0]=buffer[0]|(buffer[1]<<8); 	// X
//    out[1]=buffer[2]|(buffer[3]<<8);	// Y
//    out[2]=buffer[4]|(buffer[5]<<8);	// Z

    //	while (!PIOS_AK8974_Write(AK8974_CTRL3, AK8974_CTRL3_FORCE)) ;

}

/**
* Read the identification bytes from the sensor
*/
void PIOS_AK8974_ReadID(uint8_t out[4])
{
//	while (!PIOS_AK8794_Read(PIOS_AK8794_DATAOUT_IDA_REG, out, 3)) ;
	out[0] = 'N';
	out[1] = 'A';
	out[3] = '\0';
}

bool PIOS_AK8974_NewDataAvailable(void)
{
	return (pios_AK8974_data_ready);
}

/**
* Reads one or more bytes into a buffer
* \param[in] address AK8974 register address
* \param[out] buffer destination buffer
* \param[in] len number of bytes which should be read
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
* \return -4 if invalid length
*/
static bool PIOS_AK8974_Read(uint8_t address, uint8_t * buffer, uint8_t len)
{
	bool retcode;
	uint8_t addr_buffer[] = {
		address,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = AK8974_WHOAMI,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(addr_buffer),
		 .buf = addr_buffer,
		 }
		,
		{
		 .info = __func__,
		 .addr = AK8974_WHOAMI,
		 .rw = PIOS_I2C_TXN_READ,
		 .len = len,
		 .buf = buffer,
		 }
	};
	retcode = PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
	return retcode;
}

/**
* Writes one or more bytes to the AK8794
* \param[in] address Register address
* \param[in] buffer source buffer
* \return 0 if operation was successful
* \return -1 if error during I2C transfer
*/
static bool PIOS_AK8974_Write(uint8_t address, uint8_t buffer)
{
	uint8_t data[] = {
		address,
		buffer,
	};

	const struct pios_i2c_txn txn_list[] = {
		{
		 .info = __func__,
		 .addr = AK8974_WHOAMI,
		 .rw = PIOS_I2C_TXN_WRITE,
		 .len = sizeof(data),
		 .buf = data,
		 }
		,
	};

	return PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
}

void PIOS_AK8974_IRQHandler(void)
{
	pios_AK8974_data_ready = true;
}

#endif /* PIOS_INCLUDE_AK8974 */

/**
 * @}
 * @}
 */
