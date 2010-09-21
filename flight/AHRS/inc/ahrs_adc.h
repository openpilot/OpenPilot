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
#define MAX_SAMPLES (8*50*2)

uint8_t AHRS_ADC_Config(int32_t adc_oversample);
void AHRS_ADC_DMA_Handler(void);

typedef enum { AHRS_IDLE, AHRS_DATA_READY, AHRS_PROCESSING } states;
extern volatile states ahrs_state;
extern volatile int16_t *valid_data_buffer;
//! Counts how many times the EKF wasn't idle when DMA handler called
extern volatile int32_t total_conversion_blocks;
//! Total number of data blocks converted
extern volatile int32_t ekf_too_slow;

#endif
