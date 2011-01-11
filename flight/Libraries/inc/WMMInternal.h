/**
 ******************************************************************************
 *
 * @file       WMMInternal.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Include file of the WorldMagModel internal functionality.
 *
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

#ifndef WMMINTERNAL_H_
#define WMMINTERNAL_H_

	// internal constants
#define TRUE            ((uint16_t)1)
#define FALSE           ((uint16_t)0)
#define WMM_MAX_MODEL_DEGREES	12
#define WMM_MAX_SECULAR_VARIATION_MODEL_DEGREES	12
#define	NUMTERMS 91		// ((WMM_MAX_MODEL_DEGREES+1)*(WMM_MAX_MODEL_DEGREES+2)/2);
#define NUMPCUP 92		// NUMTERMS +1
#define NUMPCUPS 13		// WMM_MAX_MODEL_DEGREES +1
#define RAD2DEG(rad)    ((rad)*(180.0L/M_PI))
#define DEG2RAD(deg)    ((deg)*(M_PI/180.0L))

	// internal structure definitions
typedef struct {
	float EditionDate;
	float epoch;		//Base time of Geomagnetic model epoch (yrs)
	char ModelName[20];
//	float Main_Field_Coeff_G[NUMTERMS];	// C - Gauss coefficients of main geomagnetic model (nT)
//	float Main_Field_Coeff_H[NUMTERMS];	// C - Gauss coefficients of main geomagnetic model (nT)
//	float Secular_Var_Coeff_G[NUMTERMS];	// CD - Gauss coefficients of secular geomagnetic model (nT/yr)
//	float Secular_Var_Coeff_H[NUMTERMS];	// CD - Gauss coefficients of secular geomagnetic model (nT/yr)
	uint16_t nMax;		// Maximum degree of spherical harmonic model
	uint16_t nMaxSecVar;	// Maxumum degree of spherical harmonic secular model
	uint16_t SecularVariationUsed;	// Whether or not the magnetic secular variation vector will be needed by program
} WMMtype_MagneticModel;

typedef struct {
	float a;		// semi-major axis of the ellipsoid
	float b;		// semi-minor axis of the ellipsoid
	float fla;		// flattening
	float epssq;		// first eccentricity squared
	float eps;		// first eccentricity 
	float re;		// mean radius of  ellipsoid
} WMMtype_Ellipsoid;

typedef struct {
	float lambda;		// longitude
	float phi;		// geodetic latitude
	float HeightAboveEllipsoid;	// height above the ellipsoid (HaE)
} WMMtype_CoordGeodetic;

typedef struct {
	float lambda;		// longitude
	float phig;		// geocentric latitude
	float r;		// distance from the center of the ellipsoid
} WMMtype_CoordSpherical;

typedef struct {
	uint16_t Year;
	uint16_t Month;
	uint16_t Day;
	float DecimalYear;
} WMMtype_Date;

typedef struct {
	float Pcup[NUMPCUP];	// Legendre Function 
	float dPcup[NUMPCUP];	// Derivative of Lagendre fn 
} WMMtype_LegendreFunction;

typedef struct {
	float Bx;		// North
	float By;		// East
	float Bz;		// Down 
} WMMtype_MagneticResults;

typedef struct {

	float RelativeRadiusPower[WMM_MAX_MODEL_DEGREES + 1];	// [earth_reference_radius_km / sph. radius ]^n
	float cos_mlambda[WMM_MAX_MODEL_DEGREES + 1];	// cp(m)  - cosine of (m*spherical coord. longitude
	float sin_mlambda[WMM_MAX_MODEL_DEGREES + 1];	// sp(m)  - sine of (m*spherical coord. longitude)
} WMMtype_SphericalHarmonicVariables;

typedef struct {
	float Decl;		/* 1. Angle between the magnetic field vector and true north, positive east */
	float Incl;		/*2. Angle between the magnetic field vector and the horizontal plane, positive down */
	float F;		/*3. Magnetic Field Strength */
	float H;		/*4. Horizontal Magnetic Field Strength */
	float X;		/*5. Northern component of the magnetic field vector */
	float Y;		/*6. Eastern component of the magnetic field vector */
	float Z;		/*7. Downward component of the magnetic field vector */
	float GV;		/*8. The Grid Variation */
	float Decldot;		/*9. Yearly Rate of change in declination */
	float Incldot;		/*10. Yearly Rate of change in inclination */
	float Fdot;		/*11. Yearly rate of change in Magnetic field strength */
	float Hdot;		/*12. Yearly rate of change in horizontal field strength */
	float Xdot;		/*13. Yearly rate of change in the northern component */
	float Ydot;		/*14. Yearly rate of change in the eastern component */
	float Zdot;		/*15. Yearly rate of change in the downward component */
	float GVdot;		/*16. Yearly rate of chnage in grid variation */
} WMMtype_GeoMagneticElements;

	// Internal Function Prototypes
void WMM_Set_Coeff_Array();
int WMM_GeodeticToSpherical(WMMtype_CoordGeodetic * CoordGeodetic, WMMtype_CoordSpherical * CoordSpherical);
int WMM_DateToYear(uint16_t month, uint16_t day, uint16_t year);
int WMM_Geomag(WMMtype_CoordSpherical * CoordSpherical,
		    WMMtype_CoordGeodetic * CoordGeodetic, WMMtype_GeoMagneticElements * GeoMagneticElements);

int WMM_AssociatedLegendreFunction(WMMtype_CoordSpherical * CoordSpherical, uint16_t nMax, WMMtype_LegendreFunction * LegendreFunction);

int WMM_CalculateGeoMagneticElements(WMMtype_MagneticResults * MagneticResultsGeo, WMMtype_GeoMagneticElements * GeoMagneticElements);

int WMM_CalculateSecularVariation(WMMtype_MagneticResults * MagneticVariation, WMMtype_GeoMagneticElements * MagneticElements);

int WMM_ComputeSphericalHarmonicVariables(WMMtype_CoordSpherical *
					       CoordSpherical, uint16_t nMax, WMMtype_SphericalHarmonicVariables * SphVariables);

int WMM_PcupLow(float *Pcup, float *dPcup, float x, uint16_t nMax);

int WMM_PcupHigh(float *Pcup, float *dPcup, float x, uint16_t nMax);

int WMM_RotateMagneticVector(WMMtype_CoordSpherical *,
				  WMMtype_CoordGeodetic * CoordGeodetic,
				  WMMtype_MagneticResults * MagneticResultsSph, WMMtype_MagneticResults * MagneticResultsGeo);

int WMM_SecVarSummation(WMMtype_LegendreFunction * LegendreFunction,
			     WMMtype_SphericalHarmonicVariables *
			     SphVariables, WMMtype_CoordSpherical * CoordSpherical, WMMtype_MagneticResults * MagneticResults);

int WMM_SecVarSummationSpecial(WMMtype_SphericalHarmonicVariables *
				    SphVariables, WMMtype_CoordSpherical * CoordSpherical, WMMtype_MagneticResults * MagneticResults);

int WMM_Summation(WMMtype_LegendreFunction * LegendreFunction,
		       WMMtype_SphericalHarmonicVariables * SphVariables,
		       WMMtype_CoordSpherical * CoordSpherical, WMMtype_MagneticResults * MagneticResults);

int WMM_SummationSpecial(WMMtype_SphericalHarmonicVariables *
			      SphVariables, WMMtype_CoordSpherical * CoordSpherical, WMMtype_MagneticResults * MagneticResults);

float WMM_get_main_field_coeff_g(uint16_t index);
float WMM_get_main_field_coeff_h(uint16_t index);
float WMM_get_secular_var_coeff_g(uint16_t index);
float WMM_get_secular_var_coeff_h(uint16_t index);

#endif /* WMMINTERNAL_H_ */
