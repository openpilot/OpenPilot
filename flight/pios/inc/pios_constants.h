/**
 ******************************************************************************
 *
 * @file       pios_constants.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright_F (Cf) 2013.
 * @brief      Shared phisical constants
 *             --
 * @see        The GNU Public License_F (GPLf) Version 3
 *
 *****************************************************************************/
/* 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 *_F (at your optionf) any later version.
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

#ifndef PIOS_CONSTANTS_H
#define PIOS_CONSTANTS_H

// Constants borrowed straight from GSL http://www.gnu.org/software/gsl/

#define PIOS_CONST_MKS_SPEED_OF_LIGHT_F (2.99792458e8f) /* m / s */
#define PIOS_CONST_MKS_GRAVITATIONAL_CONSTANT_F (6.673e-11f) /* m^3 / kg s^2 */
#define PIOS_CONST_MKS_PLANCKS_CONSTANT_H_F (6.62606896e-34f) /* kg m^2 / s */
#define PIOS_CONST_MKS_PLANCKS_CONSTANT_HBAR_F (1.05457162825e-34f) /* kg m^2 / s */
#define PIOS_CONST_MKS_ASTRONOMICAL_UNIT_F (1.49597870691e11f) /* m */
#define PIOS_CONST_MKS_LIGHT_YEAR_F (9.46053620707e15f) /* m */
#define PIOS_CONST_MKS_PARSEC_F (3.08567758135e16f) /* m */
#define PIOS_CONST_MKS_GRAV_ACCEL_F (9.80665e0f) /* m / s^2 */
#define PIOS_CONST_MKS_ELECTRON_VOLT_F (1.602176487e-19f) /* kg m^2 / s^2 */
#define PIOS_CONST_MKS_MASS_ELECTRON_F (9.10938188e-31f) /* kg */
#define PIOS_CONST_MKS_MASS_MUON_F (1.88353109e-28f) /* kg */
#define PIOS_CONST_MKS_MASS_PROTON_F (1.67262158e-27f) /* kg */
#define PIOS_CONST_MKS_MASS_NEUTRON_F (1.67492716e-27f) /* kg */
#define PIOS_CONST_MKS_RYDBERG_F (2.17987196968e-18f) /* kg m^2 / s^2 */
#define PIOS_CONST_MKS_BOLTZMANN_F (1.3806504e-23f) /* kg m^2 / K s^2 */
#define PIOS_CONST_MKS_MOLAR_GAS_F (8.314472e0f) /* kg m^2 / K mol s^2 */
#define PIOS_CONST_MKS_STANDARD_GAS_VOLUME_F (2.2710981e-2f) /* m^3 / mol */
#define PIOS_CONST_MKS_MINUTE_F (6e1f) /* s */
#define PIOS_CONST_MKS_HOUR_F (3.6e3f) /* s */
#define PIOS_CONST_MKS_DAY_F (8.64e4f) /* s */
#define PIOS_CONST_MKS_WEEK_F (6.048e5f) /* s */
#define PIOS_CONST_MKS_INCH_F (2.54e-2f) /* m */
#define PIOS_CONST_MKS_FOOT_F (3.048e-1f) /* m */
#define PIOS_CONST_MKS_YARD_F (9.144e-1f) /* m */
#define PIOS_CONST_MKS_MILE_F (1.609344e3f) /* m */
#define PIOS_CONST_MKS_NAUTICAL_MILE_F (1.852e3f) /* m */
#define PIOS_CONST_MKS_FATHOM_F (1.8288e0f) /* m */
#define PIOS_CONST_MKS_MIL_F (2.54e-5f) /* m */
#define PIOS_CONST_MKS_POINT_F (3.52777777778e-4f) /* m */
#define PIOS_CONST_MKS_TEXPOINT_F (3.51459803515e-4f) /* m */
#define PIOS_CONST_MKS_MICRON_F (1e-6f) /* m */
#define PIOS_CONST_MKS_ANGSTROM_F (1e-10f) /* m */
#define PIOS_CONST_MKS_HECTARE_F (1e4f) /* m^2 */
#define PIOS_CONST_MKS_ACRE_F (4.04685642241e3f) /* m^2 */
#define PIOS_CONST_MKS_BARN_F (1e-28f) /* m^2 */
#define PIOS_CONST_MKS_LITER_F (1e-3f) /* m^3 */
#define PIOS_CONST_MKS_US_GALLON_F (3.78541178402e-3f) /* m^3 */
#define PIOS_CONST_MKS_QUART_F (9.46352946004e-4f) /* m^3 */
#define PIOS_CONST_MKS_PINT_F (4.73176473002e-4f) /* m^3 */
#define PIOS_CONST_MKS_CUP_F (2.36588236501e-4f) /* m^3 */
#define PIOS_CONST_MKS_FLUID_OUNCE_F (2.95735295626e-5f) /* m^3 */
#define PIOS_CONST_MKS_TABLESPOON_F (1.47867647813e-5f) /* m^3 */
#define PIOS_CONST_MKS_TEASPOON_F (4.92892159375e-6f) /* m^3 */
#define PIOS_CONST_MKS_CANADIAN_GALLON_F (4.54609e-3f) /* m^3 */
#define PIOS_CONST_MKS_UK_GALLON_F (4.546092e-3f) /* m^3 */
#define PIOS_CONST_MKS_MILES_PER_HOUR_F (4.4704e-1f) /* m / s */
#define PIOS_CONST_MKS_KILOMETERS_PER_HOUR_F (2.77777777778e-1f) /* m / s */
#define PIOS_CONST_MKS_KNOT_F (5.14444444444e-1f) /* m / s */
#define PIOS_CONST_MKS_POUND_MASS_F (4.5359237e-1f) /* kg */
#define PIOS_CONST_MKS_OUNCE_MASS_F (2.8349523125e-2f) /* kg */
#define PIOS_CONST_MKS_TON_F (9.0718474e2f) /* kg */
#define PIOS_CONST_MKS_METRIC_TON_F (1e3f) /* kg */
#define PIOS_CONST_MKS_UK_TON_F (1.0160469088e3f) /* kg */
#define PIOS_CONST_MKS_TROY_OUNCE_F (3.1103475e-2f) /* kg */
#define PIOS_CONST_MKS_CARAT_F (2e-4f) /* kg */
#define PIOS_CONST_MKS_UNIFIED_ATOMIC_MASS_F (1.660538782e-27f) /* kg */
#define PIOS_CONST_MKS_GRAM_FORCE_F (9.80665e-3f) /* kg m / s^2 */
#define PIOS_CONST_MKS_POUND_FORCE_F (4.44822161526e0f) /* kg m / s^2 */
#define PIOS_CONST_MKS_KILOPOUND_FORCE_F (4.44822161526e3f) /* kg m / s^2 */
#define PIOS_CONST_MKS_POUNDAL_F (1.38255e-1f) /* kg m / s^2 */
#define PIOS_CONST_MKS_CALORIE_F (4.1868e0f) /* kg m^2 / s^2 */
#define PIOS_CONST_MKS_BTU_F (1.05505585262e3f) /* kg m^2 / s^2 */
#define PIOS_CONST_MKS_THERM_F (1.05506e8f) /* kg m^2 / s^2 */
#define PIOS_CONST_MKS_HORSEPOWER_F (7.457e2f) /* kg m^2 / s^3 */
#define PIOS_CONST_MKS_BAR_F (1e5f) /* kg / m s^2 */
#define PIOS_CONST_MKS_STD_ATMOSPHERE_F (1.01325e5f) /* kg / m s^2 */
#define PIOS_CONST_MKS_TORR_F (1.33322368421e2f) /* kg / m s^2 */
#define PIOS_CONST_MKS_METER_OF_MERCURY_F (1.33322368421e5f) /* kg / m s^2 */
#define PIOS_CONST_MKS_INCH_OF_MERCURY_F (3.38638815789e3f) /* kg / m s^2 */
#define PIOS_CONST_MKS_INCH_OF_WATER_F (2.490889e2f) /* kg / m s^2 */
#define PIOS_CONST_MKS_PSI_F (6.89475729317e3f) /* kg / m s^2 */
#define PIOS_CONST_MKS_POISE_F (1e-1f) /* kg m^-1 s^-1 */
#define PIOS_CONST_MKS_STOKES_F (1e-4f) /* m^2 / s */
#define PIOS_CONST_MKS_STILB_F (1e4f) /* cd / m^2 */
#define PIOS_CONST_MKS_LUMEN_F (1e0f) /* cd sr */
#define PIOS_CONST_MKS_LUX_F (1e0f) /* cd sr / m^2 */
#define PIOS_CONST_MKS_PHOT_F (1e4f) /* cd sr / m^2 */
#define PIOS_CONST_MKS_FOOTCANDLE_F (1.076e1f) /* cd sr / m^2 */
#define PIOS_CONST_MKS_LAMBERT_F (1e4f) /* cd sr / m^2 */
#define PIOS_CONST_MKS_FOOTLAMBERT_F (1.07639104e1f) /* cd sr / m^2 */
#define PIOS_CONST_MKS_CURIE_F (3.7e10f) /* 1 / s */
#define PIOS_CONST_MKS_ROENTGEN_F (2.58e-4f) /* A s / kg */
#define PIOS_CONST_MKS_RAD_F (1e-2f) /* m^2 / s^2 */
#define PIOS_CONST_MKS_SOLAR_MASS_F (1.98892e30f) /* kg */
#define PIOS_CONST_MKS_BOHR_RADIUS_F (5.291772083e-11f) /* m */
#define PIOS_CONST_MKS_NEWTON_F (1e0f) /* kg m / s^2 */
#define PIOS_CONST_MKS_DYNE_F (1e-5f) /* kg m / s^2 */
#define PIOS_CONST_MKS_JOULE_F (1e0f) /* kg m^2 / s^2 */
#define PIOS_CONST_MKS_ERG_F (1e-7f) /* kg m^2 / s^2 */
#define PIOS_CONST_MKS_STEFAN_BOLTZMANN_CONSTANT_F (5.67040047374e-8f) /* kg / K^4 s^3 */
#define PIOS_CONST_MKS_THOMSON_CROSS_SECTION_F (6.65245893699e-29f) /* m^2 */
#define PIOS_CONST_MKS_BOHR_MAGNETON_F (9.27400899e-24f) /* A m^2 */
#define PIOS_CONST_MKS_NUCLEAR_MAGNETON_F (5.05078317e-27f) /* A m^2 */
#define PIOS_CONST_MKS_ELECTRON_MAGNETIC_MOMENT_F (9.28476362e-24f) /* A m^2 */
#define PIOS_CONST_MKS_PROTON_MAGNETIC_MOMENT_F (1.410606633e-26f) /* A m^2 */
#define PIOS_CONST_MKS_FARADAY_F (9.64853429775e4f) /* A s / mol */
#define PIOS_CONST_MKS_ELECTRON_CHARGE_F (1.602176487e-19f) /* A s */
#define PIOS_CONST_MKS_VACUUM_PERMITTIVITY_F (8.854187817e-12f) /* A^2 s^4 / kg m^3 */
#define PIOS_CONST_MKS_VACUUM_PERMEABILITY_F (1.25663706144e-6f) /* kg m / A^2 s^2 */
#define PIOS_CONST_MKS_DEBYE_F (3.33564095198e-30f) /* A s^2 / m^2 */
#define PIOS_CONST_MKS_GAUSS_F (1e-4f) /* kg / A s^2 */

#endif /* PIOS_CONSTANTS_H */
