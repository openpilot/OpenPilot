/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_ADC ADC Functions
 * @brief STM32 ADC PIOS interface
 * @{
 *
 * @file       pios_adc.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      ADC functions header.
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

#ifndef PIOS_ADC_H
#define PIOS_ADC_H

// Maximum of 50 oversampled points
#define PIOS_ADC_MAX_SAMPLES ((((PIOS_ADC_NUM_CHANNELS + PIOS_ADC_USE_ADC2) >> PIOS_ADC_USE_ADC2) << PIOS_ADC_USE_ADC2)* PIOS_ADC_MAX_OVERSAMPLING * 2)

typedef void (*ADCCallback) (float * data);

/* Public Functions */
void PIOS_ADC_Config(uint32_t oversampling);
int32_t PIOS_ADC_PinGet(uint32_t pin);
int16_t * PIOS_ADC_GetRawBuffer(void);
uint8_t PIOS_ADC_GetOverSampling(void);
void PIOS_ADC_SetCallback(ADCCallback new_function);
#if defined(PIOS_INCLUDE_FREERTOS)
void PIOS_ADC_SetQueue(xQueueHandle data_queue);
#endif
extern void PIOS_ADC_DMA_Handler(void);

#endif /* PIOS_ADC_H */

/**
  * @}
  * @}
  */
