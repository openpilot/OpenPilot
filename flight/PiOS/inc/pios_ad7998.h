/*
 * pios_ad7998.h
 *
 *  Created on: 29 oct. 2012
 *      Author: fertito
 */

#ifndef PIOS_AD7998_H_
#define PIOS_AD7998_H_

#include <pios.h>

/* AD7998 Addresses */
#define PIOS_AD7998_0_I2C_ADDR			0x21
#define PIOS_AD7998_1_I2C_ADDR			0x23
#define PIOS_AD7998_CONV_REG			0x00
#define PIOS_AD7998_ALERT_STAT			0x01
#define PIOS_AD7998_CONF_REG			0x02
#define PIOS_AD7998_CYCLE_TIM_REG		0x03
#define PIOS_AD7998_DATAL_CH_START		0x04 //each next channel +0x03
#define PIOS_AD7998_DATAH_cH_START		0x05 //each next channel +0x03
#define PIOS_AD7998_DATA_HYS_START		0x06 //each next channel +0x03

#define PIOS_AD7998_CONF_POLARITY        (1 << 0)
#define PIOS_AD7998_CONF_BUSY_ALERT      (1 << 1)
#define PIOS_AD7998_CONF_ALERT_EN        (1 << 2)
#define PIOS_AD7998_CONF_FLTR            (1 << 3)
#define PIOS_AD7998_CONF_CH0             (1 << 4)
#define PIOS_AD7998_CONF_CH1             (1 << 5)
#define PIOS_AD7998_CONF_CH2             (1 << 6)
#define PIOS_AD7998_CONF_CH3             (1 << 7)
#define PIOS_AD7998_CONF_CH4             (1 << 8)
#define PIOS_AD7998_CONF_CH5             (1 << 9)
#define PIOS_AD7998_CONF_CH6             (1 << 10)
#define PIOS_AD7998_CONF_CH7             (1 << 11)
#define PIOS_AD7998_CONF_CH_ALL          (PIOS_AD7998_CONF_CH0 | PIOS_AD7998_CONF_CH1 \
                                        | PIOS_AD7998_CONF_CH2 | PIOS_AD7998_CONF_CH3 \
                                        | PIOS_AD7998_CONF_CH4 | PIOS_AD7998_CONF_CH5 \
                                        | PIOS_AD7998_CONF_CH6 | PIOS_AD7998_CONF_CH7)
struct pios_ad7998_data {
	uint16_t V[8];
	bool Fresh;
};

struct pios_ad7998_cfg {
	//const struct pios_exti_cfg * exti_cfg; /* Pointer to the EXTI configuration */
	int32_t i2c_id;
	
	struct pios_ad7998_data Value;
	
	xTaskHandle TaskHandle;
	
	uint8_t Conf_h;		//0x00
	uint8_t Conf_l;		//0x08 filter on i2c
};

extern int32_t PIOS_AD7998_Init(int32_t i2c);
extern uint16_t PIOS_AD7998_ReadConv(uint8_t channel);
extern uint16_t PIOS_AD7998_GetValue(uint8_t channel);
extern int32_t PIOS_AD7998_Start(int32_t i2c);

#endif /* PIOS_AD7998_H_ */
