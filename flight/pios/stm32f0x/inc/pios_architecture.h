/**
 ******************************************************************************
 *
 * @file       pios_architecture.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Architecture specific macros and definitions
 *             --
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

#ifndef PIOS_ARCHITECTURE_H
#define PIOS_ARCHITECTURE_H

// defines for adc
#define PIOS_ADC_VOLTAGE_SCALE        3.30f / 4096.0f

// defines for Temp measurements
#define PIOS_ADC_STM32_TEMP_V25       1.43f /* V */
#define PIOS_ADC_STM32_TEMP_AVG_SLOPE 4.3f /* mV/C */
#define PIOS_CONVERT_VOLT_TO_CPU_TEMP(x) ((PIOS_ADC_STM32_TEMP_V25 - x) * 1000.0f / PIOS_ADC_STM32_TEMP_AVG_SLOPE + 25.0f)


#endif /* PIOS_ARCHITECTURE_H */
