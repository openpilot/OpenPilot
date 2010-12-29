/**
 ******************************************************************************
 * @addtogroup AHRS AHRS Control
 * @brief The AHRS Modules perform
 *
 * @{ 
 * @addtogroup AHRS_ADC
 * @brief Specialized ADC code for double buffered DMA for AHRS
 * @{ 
 *
 *
 * @file       ahrs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      INSGPS Test Program
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

#ifndef AHRS_ADC
#define AHRS_ADC

#include <pios.h>

// Maximum of 50 oversampled points
#define MAX_OVERSAMPLING 50    /* cannot have more than 50 samples      */
#define MAX_SAMPLES (PIOS_ADC_NUM_CHANNELS*MAX_OVERSAMPLING*2)

typedef void (*ADCCallback) (float * data);

// Public API:
uint8_t AHRS_ADC_Config(int32_t adc_oversample);
void AHRS_ADC_DMA_Handler(void);
void AHRS_ADC_SetCallback(ADCCallback);
void AHRS_ADC_SetFIRCoefficients(float * new_filter);
float * AHRS_ADC_GetBuffer(void);
int16_t * AHRS_ADC_GetRawBuffer(void);
uint8_t AHRS_ADC_GetOverSampling(void);

#endif
