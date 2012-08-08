/**
 ******************************************************************************
 *
 * @file       WorldMagModel.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Source file for the World Magnetic Model
 *             This is a port of code available from the US NOAA.
 *
 *             The hard coded coefficients should be valid until 2015.
 *
 *             Updated coeffs from ..
 *             http://www.ngdc.noaa.gov/geomag/WMM/wmm_ddownload.shtml
 *
 *             NASA C source code ..
 *             http://www.ngdc.noaa.gov/geomag/WMM/wmm_wdownload.shtml
 *
 *             Major changes include:
 *                - No geoid model (altitude must be geodetic WGS-84)
 *                - Floating point calculation (not double precision)
 *                - Hard coded coefficients for model
 *                - Elimination of user interface
 *                - Elimination of dynamic memory allocation
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

// I don't want this dependency, but currently using pvPortMalloc
#include "openpilot.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include "WorldMagModel.h"
#include "WMMInternal.h"

#define MALLOC(x) pvPortMalloc(x)
#define FREE(x) vPortFree(x)
//#define MALLOC(x) malloc(x)
//#define FREE(x) free(x)

// const should hopefully keep them in the flash region
static const float CoeffFile[91][6] = { {0, 0, 0, 0, 0, 0},
	{1, 0, -29496.6, 0.0, 11.6, 0.0},
	{1, 1, -1586.3, 4944.4, 16.5, -25.9},
	{2, 0, -2396.6, 0.0, -12.1, 0.0},
	{2, 1, 3026.1, -2707.7, -4.4, -22.5},
	{2, 2, 1668.6, -576.1, 1.9, -11.8},
	{3, 0, 1340.1, 0.0, 0.4, 0.0},
	{3, 1, -2326.2, -160.2, -4.1, 7.3},
	{3, 2, 1231.9, 251.9, -2.9, -3.9},
	{3, 3, 634.0, -536.6, -7.7, -2.6},
	{4, 0, 912.6, 0.0, -1.8, 0.0},
	{4, 1, 808.9, 286.4, 2.3, 1.1},
	{4, 2, 166.7, -211.2, -8.7, 2.7},
	{4, 3, -357.1, 164.3, 4.6, 3.9},
	{4, 4, 89.4, -309.1, -2.1, -0.8},
	{5, 0, -230.9, 0.0, -1.0, 0.0},
	{5, 1, 357.2, 44.6, 0.6, 0.4},
	{5, 2, 200.3, 188.9, -1.8, 1.8},
	{5, 3, -141.1, -118.2, -1.0, 1.2},
	{5, 4, -163.0, 0.0, 0.9, 4.0},
	{5, 5, -7.8, 100.9, 1.0, -0.6},
	{6, 0, 72.8, 0.0, -0.2, 0.0},
	{6, 1, 68.6, -20.8, -0.2, -0.2},
	{6, 2, 76.0, 44.1, -0.1, -2.1},
	{6, 3, -141.4, 61.5, 2.0, -0.4},
	{6, 4, -22.8, -66.3, -1.7, -0.6},
	{6, 5, 13.2, 3.1, -0.3, 0.5},
	{6, 6, -77.9, 55.0, 1.7, 0.9},
	{7, 0, 80.5, 0.0, 0.1, 0.0},
	{7, 1, -75.1, -57.9, -0.1, 0.7},
	{7, 2, -4.7, -21.1, -0.6, 0.3},
	{7, 3, 45.3, 6.5, 1.3, -0.1},
	{7, 4, 13.9, 24.9, 0.4, -0.1},
	{7, 5, 10.4, 7.0, 0.3, -0.8},
	{7, 6, 1.7, -27.7, -0.7, -0.3},
	{7, 7, 4.9, -3.3, 0.6, 0.3},
	{8, 0, 24.4, 0.0, -0.1, 0.0},
	{8, 1, 8.1, 11.0, 0.1, -0.1},
	{8, 2, -14.5, -20.0, -0.6, 0.2},
	{8, 3, -5.6, 11.9, 0.2, 0.4},
	{8, 4, -19.3, -17.4, -0.2, 0.4},
	{8, 5, 11.5, 16.7, 0.3, 0.1},
	{8, 6, 10.9, 7.0, 0.3, -0.1},
	{8, 7, -14.1, -10.8, -0.6, 0.4},
	{8, 8, -3.7, 1.7, 0.2, 0.3},
	{9, 0, 5.4, 0.0, 0.0, 0.0},
	{9, 1, 9.4, -20.5, -0.1, 0.0},
	{9, 2, 3.4, 11.5, 0.0, -0.2},
	{9, 3, -5.2, 12.8, 0.3, 0.0},
	{9, 4, 3.1, -7.2, -0.4, -0.1},
	{9, 5, -12.4, -7.4, -0.3, 0.1},
	{9, 6, -0.7, 8.0, 0.1, 0.0},
	{9, 7, 8.4, 2.1, -0.1, -0.2},
	{9, 8, -8.5, -6.1, -0.4, 0.3},
	{9, 9, -10.1, 7.0, -0.2, 0.2},
	{10, 0, -2.0, 0.0, 0.0, 0.0},
	{10, 1, -6.3, 2.8, 0.0, 0.1},
	{10, 2, 0.9, -0.1, -0.1, -0.1},
	{10, 3, -1.1, 4.7, 0.2, 0.0},
	{10, 4, -0.2, 4.4, 0.0, -0.1},
	{10, 5, 2.5, -7.2, -0.1, -0.1},
	{10, 6, -0.3, -1.0, -0.2, 0.0},
	{10, 7, 2.2, -3.9, 0.0, -0.1},
	{10, 8, 3.1, -2.0, -0.1, -0.2},
	{10, 9, -1.0, -2.0, -0.2, 0.0},
	{10, 10, -2.8, -8.3, -0.2, -0.1},
	{11, 0, 3.0, 0.0, 0.0, 0.0},
	{11, 1, -1.5, 0.2, 0.0, 0.0},
	{11, 2, -2.1, 1.7, 0.0, 0.1},
	{11, 3, 1.7, -0.6, 0.1, 0.0},
	{11, 4, -0.5, -1.8, 0.0, 0.1},
	{11, 5, 0.5, 0.9, 0.0, 0.0},
	{11, 6, -0.8, -0.4, 0.0, 0.1},
	{11, 7, 0.4, -2.5, 0.0, 0.0},
	{11, 8, 1.8, -1.3, 0.0, -0.1},
	{11, 9, 0.1, -2.1, 0.0, -0.1},
	{11, 10, 0.7, -1.9, -0.1, 0.0},
	{11, 11, 3.8, -1.8, 0.0, -0.1},
	{12, 0, -2.2, 0.0, 0.0, 0.0},
	{12, 1, -0.2, -0.9, 0.0, 0.0},
	{12, 2, 0.3, 0.3, 0.1, 0.0},
	{12, 3, 1.0, 2.1, 0.1, 0.0},
	{12, 4, -0.6, -2.5, -0.1, 0.0},
	{12, 5, 0.9, 0.5, 0.0, 0.0},
	{12, 6, -0.1, 0.6, 0.0, 0.1},
	{12, 7, 0.5, 0.0, 0.0, 0.0},
	{12, 8, -0.4, 0.1, 0.0, 0.0},
	{12, 9, -0.4, 0.3, 0.0, 0.0},
	{12, 10, 0.2, -0.9, 0.0, 0.0},
	{12, 11, -0.8, -0.2, -0.1, 0.0},
	{12, 12, 0.0, 0.9, 0.1, 0.0}
};

static WMMtype_Ellipsoid        *Ellip = NULL;
static WMMtype_MagneticModel    *MagneticModel = NULL;
static float                    decimal_date;

/**************************************************************************************
*   Example use - very simple - only two exposed functions
*
*	WMM_Initialize(); // Set default values and constants
*
*	WMM_GetMagVector(float Lat, float Lon, float Alt, uint16_t Month, uint16_t Day, uint16_t Year, float B[3]);
*	e.g. Iceland in may of 2012 = WMM_GetMagVector(65.0, -20.0, 0.0, 5, 5, 2012, B);
*	Alt is above the WGS-84 Ellipsoid
*	B is the NED (XYZ) magnetic vector in nTesla
**************************************************************************************/

int WMM_Initialize()
//      Sets default values for WMM subroutines.
//      UPDATES : Ellip and MagneticModel
{	
	if (!Ellip) return -1;          // invalid pointer
	if (!MagneticModel) return -2;  // invalid pointer
	
	// Sets WGS-84 parameters
	Ellip->a = 6378.137;	// semi-major axis of the ellipsoid in km
	Ellip->b = 6356.7523142;	// semi-minor axis of the ellipsoid in km
	Ellip->fla = 1 / 298.257223563;	// flattening
	Ellip->eps = sqrt(1 - (Ellip->b * Ellip->b) / (Ellip->a * Ellip->a));	// first eccentricity
	Ellip->epssq = (Ellip->eps * Ellip->eps);	// first eccentricity squared
	Ellip->re = 6371.2;	// Earth's radius in km

	// Sets Magnetic Model parameters
	MagneticModel->nMax = WMM_MAX_MODEL_DEGREES;
	MagneticModel->nMaxSecVar = WMM_MAX_SECULAR_VARIATION_MODEL_DEGREES;
	MagneticModel->SecularVariationUsed = 0;

	// Really, Really needs to be read from a file - out of date in 2015 at latest
	MagneticModel->EditionDate = 5.7863328170559505e-307;
	MagneticModel->epoch = 2010.0;
	sprintf(MagneticModel->ModelName, "WMM-2010");

	return 0;                       // OK
}

int WMM_GetMagVector(float Lat, float Lon, float AltEllipsoid, uint16_t Month, uint16_t Day, uint16_t Year, float B[3])
{	
    // return '0' if all appears to be OK
    // return < 0 if error

    int returned = 0;   // default to OK

    // ***********
    // range check supplied params

    if (Lat <  -90) return -1;  // error
    if (Lat >   90) return -2;  // error

    if (Lon < -180) return -3;  // error
    if (Lon >  180) return -4;  // error

    // ***********
    // allocated required memory

//    Ellip = NULL;
//    MagneticModel = NULL;

//    MagneticModel = NULL;
//    CoordGeodetic = NULL;
//    GeoMagneticElements = NULL;

    Ellip = (WMMtype_Ellipsoid *) MALLOC(sizeof(WMMtype_Ellipsoid));
    MagneticModel = (WMMtype_MagneticModel *) MALLOC(sizeof(WMMtype_MagneticModel));

    WMMtype_CoordSpherical *CoordSpherical = (WMMtype_CoordSpherical *) MALLOC(sizeof(WMMtype_CoordSpherical));
    WMMtype_CoordGeodetic *CoordGeodetic = (WMMtype_CoordGeodetic *) MALLOC(sizeof(WMMtype_CoordGeodetic));
    WMMtype_GeoMagneticElements *GeoMagneticElements = (WMMtype_GeoMagneticElements *) MALLOC(sizeof(WMMtype_GeoMagneticElements));

    if (!Ellip || !MagneticModel || !CoordSpherical || !CoordGeodetic || !GeoMagneticElements)
        returned = -5;  // error

    // ***********

    if (returned >= 0)
    {
        if (WMM_Initialize() < 0)
            returned = -6;  // error
    }

    if (returned >= 0)
    {
        CoordGeodetic->lambda = Lon;
        CoordGeodetic->phi = Lat;
        CoordGeodetic->HeightAboveEllipsoid = AltEllipsoid/1000.0; // convert to km

        // Convert from geodeitic to Spherical Equations: 17-18, WMM Technical report
        if (WMM_GeodeticToSpherical(CoordGeodetic, CoordSpherical) < 0)
            returned = -7;  // error
    }


    if (returned >= 0)
    {
        if (WMM_DateToYear(Month, Day, Year) < 0)
            returned = -8;  // error
    }

    if (returned >= 0)
    {
        // Compute the geoMagnetic field elements and their time change
        if (WMM_Geomag(CoordSpherical, CoordGeodetic, GeoMagneticElements) < 0)
            returned = -9;  // error
        else
        {   // set the returned values
            B[0] = GeoMagneticElements->X;
            B[1] = GeoMagneticElements->Y;
            B[2] = GeoMagneticElements->Z;
        }
    }

   // ***********
   // free allocated memory

    if (GeoMagneticElements)
        FREE(GeoMagneticElements);

    if (CoordGeodetic)
        FREE(CoordGeodetic);

    if (CoordSpherical)
        FREE(CoordSpherical);

    if (MagneticModel)
    {
       FREE(MagneticModel);
       MagneticModel = NULL;
    }

    if (Ellip)
    {
        FREE(Ellip);
        Ellip = NULL;
    }

	B[0] = GeoMagneticElements->X * 1e-2;
	B[1] = GeoMagneticElements->Y * 1e-2;
	B[2] = GeoMagneticElements->Z * 1e-2;

    return returned;
}

int WMM_Geomag(WMMtype_CoordSpherical * CoordSpherical, WMMtype_CoordGeodetic * CoordGeodetic, WMMtype_GeoMagneticElements * GeoMagneticElements)
   /*
      The main subroutine that calls a sequence of WMM sub-functions to calculate the magnetic field elements for a single point.
      The function expects the model coefficients and point coordinates as input and returns the magnetic field elements and
      their rate of change. Though, this subroutine can be called successively to calculate a time series, profile or grid
      of magnetic field, these are better achieved by the subroutine WMM_Grid.

      INPUT: Ellip
      CoordSpherical
      CoordGeodetic
      TimedMagneticModel

      OUTPUT : GeoMagneticElements

      CALLS:    WMM_ComputeSphericalHarmonicVariables( Ellip, CoordSpherical, TimedMagneticModel->nMax, &SphVariables); (Compute Spherical Harmonic variables  )
      WMM_AssociatedLegendreFunction(CoordSpherical, TimedMagneticModel->nMax, LegendreFunction);       Compute ALF
      WMM_Summation(LegendreFunction, TimedMagneticModel, SphVariables, CoordSpherical, &MagneticResultsSph);  Accumulate the spherical harmonic coefficients
      WMM_SecVarSummation(LegendreFunction, TimedMagneticModel, SphVariables, CoordSpherical, &MagneticResultsSphVar); Sum the Secular Variation Coefficients
      WMM_RotateMagneticVector(CoordSpherical, CoordGeodetic, MagneticResultsSph, &MagneticResultsGeo); Map the computed Magnetic fields to Geodeitic coordinates
      WMM_RotateMagneticVector(CoordSpherical, CoordGeodetic, MagneticResultsSphVar, &MagneticResultsGeoVar);  Map the secular variation field components to Geodetic coordinates
      WMM_CalculateGeoMagneticElements(&MagneticResultsGeo, GeoMagneticElements);   Calculate the Geomagnetic elements
      WMM_CalculateSecularVariation(MagneticResultsGeoVar, GeoMagneticElements); Calculate the secular variation of each of the Geomagnetic elements

    */
{
    int returned = 0;   // default to OK

    WMMtype_MagneticResults             MagneticResultsSph;
    WMMtype_MagneticResults             MagneticResultsGeo;
    WMMtype_MagneticResults             MagneticResultsSphVar;
    WMMtype_MagneticResults             MagneticResultsGeoVar;

    // ********
    // allocate required memory

    WMMtype_LegendreFunction            *LegendreFunction = (WMMtype_LegendreFunction *) MALLOC(sizeof(WMMtype_LegendreFunction));
    WMMtype_SphericalHarmonicVariables  *SphVariables = (WMMtype_SphericalHarmonicVariables *) MALLOC(sizeof(WMMtype_SphericalHarmonicVariables));

    if (!LegendreFunction || !SphVariables)
        returned = -1;  // memory allocation error

    // ********

    if (returned >= 0)
    {   // Compute Spherical Harmonic variables
        if (WMM_ComputeSphericalHarmonicVariables(CoordSpherical, MagneticModel->nMax, SphVariables) < 0)
            returned = -2;  // error
    }

    if (returned >= 0)
    {   // Compute ALF
        if (WMM_AssociatedLegendreFunction(CoordSpherical, MagneticModel->nMax, LegendreFunction) < 0)
            returned = -3;  // error
    }

    if (returned >= 0)
    {   // Accumulate the spherical harmonic coefficients
        if (WMM_Summation(LegendreFunction, SphVariables, CoordSpherical, &MagneticResultsSph) < 0)
            returned = -4;  // error
    }

    if (returned >= 0)
    {   // Sum the Secular Variation Coefficients
        if (WMM_SecVarSummation(LegendreFunction, SphVariables, CoordSpherical, &MagneticResultsSphVar) < 0)
            returned = -5;  // error
    }

    if (returned >= 0)
    {   // Map the computed Magnetic fields to Geodeitic coordinates
        if (WMM_RotateMagneticVector(CoordSpherical, CoordGeodetic, &MagneticResultsSph, &MagneticResultsGeo) < 0)
            returned = -6;  // error
    }

    if (returned >= 0)
    {   // Map the secular variation field components to Geodetic coordinates
        if (WMM_RotateMagneticVector(CoordSpherical, CoordGeodetic, &MagneticResultsSphVar, &MagneticResultsGeoVar) < 0)
            returned = -7;  // error
    }

    if (returned >= 0)
    {   // Calculate the Geomagnetic elements, Equation 18 , WMM Technical report
        if (WMM_CalculateGeoMagneticElements(&MagneticResultsGeo, GeoMagneticElements) < 0)
            returned = -8;  // error
    }

    if (returned >= 0)
    {   // Calculate the secular variation of each of the Geomagnetic elements
        if (WMM_CalculateSecularVariation(&MagneticResultsGeoVar, GeoMagneticElements) < 0)
            returned = -9;  // error
    }

    // ********
    // free allocated memory

    if (SphVariables)
        FREE(SphVariables);

    if (LegendreFunction)
        FREE(LegendreFunction);

    // ********

    return returned;
}

int WMM_ComputeSphericalHarmonicVariables(WMMtype_CoordSpherical *CoordSpherical, uint16_t nMax, WMMtype_SphericalHarmonicVariables *SphVariables)

   /* Computes Spherical variables
      Variables computed are (a/r)^(n+2), cos_m(lamda) and sin_m(lambda) for spherical harmonic
      summations. (Equations 10-12 in the WMM Technical Report)
      INPUT   Ellip  data  structure with the following elements
      float a; semi-major axis of the ellipsoid
      float b; semi-minor axis of the ellipsoid
      float fla;  flattening
      float epssq; first eccentricity squared
      float eps;  first eccentricity
      float re; mean radius of  ellipsoid
      CoordSpherical    A data structure with the following elements
      float lambda; ( longitude)
      float phig; ( geocentric latitude )
      float r;            ( distance from the center of the ellipsoid)
      nMax   integer     ( Maxumum degree of spherical harmonic secular model)\

      OUTPUT  SphVariables  Pointer to the   data structure with the following elements
      float RelativeRadiusPower[WMM_MAX_MODEL_DEGREES+1];   [earth_reference_radius_km  sph. radius ]^n
      float cos_mlambda[WMM_MAX_MODEL_DEGREES+1]; cp(m)  - cosine of (mspherical coord. longitude)
      float sin_mlambda[WMM_MAX_MODEL_DEGREES+1];  sp(m)  - sine of (mspherical coord. longitude)
      CALLS : none
    */
{
	float cos_lambda, sin_lambda;
	uint16_t m, n;

	cos_lambda = cos(DEG2RAD(CoordSpherical->lambda));
	sin_lambda = sin(DEG2RAD(CoordSpherical->lambda));

	/* for n = 0 ... model_order, compute (Radius of Earth / Spherica radius r)^(n+2)
	   for n  1..nMax-1 (this is much faster than calling pow MAX_N+1 times).      */

	SphVariables->RelativeRadiusPower[0] = (Ellip->re / CoordSpherical->r) * (Ellip->re / CoordSpherical->r);
	for (n = 1; n <= nMax; n++)
		SphVariables->RelativeRadiusPower[n] = SphVariables->RelativeRadiusPower[n - 1] * (Ellip->re / CoordSpherical->r);

	/*
	   Compute cos(m*lambda), sin(m*lambda) for m = 0 ... nMax
	   cos(a + b) = cos(a)*cos(b) - sin(a)*sin(b)
	   sin(a + b) = cos(a)*sin(b) + sin(a)*cos(b)
	 */
	SphVariables->cos_mlambda[0] = 1.0;
	SphVariables->sin_mlambda[0] = 0.0;

	SphVariables->cos_mlambda[1] = cos_lambda;
	SphVariables->sin_mlambda[1] = sin_lambda;
	for (m = 2; m <= nMax; m++)
	{
		SphVariables->cos_mlambda[m] = SphVariables->cos_mlambda[m - 1] * cos_lambda - SphVariables->sin_mlambda[m - 1] * sin_lambda;
		SphVariables->sin_mlambda[m] = SphVariables->cos_mlambda[m - 1] * sin_lambda + SphVariables->sin_mlambda[m - 1] * cos_lambda;
	}

	return 0;   // OK
}

int WMM_AssociatedLegendreFunction(WMMtype_CoordSpherical * CoordSpherical, uint16_t nMax, WMMtype_LegendreFunction * LegendreFunction)

	/* Computes  all of the Schmidt-semi normalized associated Legendre
	   functions up to degree nMax. If nMax <= 16, function WMM_PcupLow is used.
	   Otherwise WMM_PcupHigh is called.
	   INPUT  CoordSpherical        A data structure with the following elements
	   float lambda; ( longitude)
	   float phig; ( geocentric latitude )
	   float r;       ( distance from the center of the ellipsoid)
	   nMax         integer          ( Maxumum degree of spherical harmonic secular model)
	   LegendreFunction Pointer to data structure with the following elements
	   float *Pcup;  (  pointer to store Legendre Function  )
	   float *dPcup; ( pointer to store  Derivative of Lagendre function )

	   OUTPUT  LegendreFunction  Calculated Legendre variables in the data structure

	 */
{
	float sin_phi = sin(DEG2RAD(CoordSpherical->phig));	/* sin  (geocentric latitude) */

	if (nMax <= 16 || (1 - fabs(sin_phi)) < 1.0e-10)	/* If nMax is less tha 16 or at the poles */
	{
		if (WMM_PcupLow(LegendreFunction->Pcup, LegendreFunction->dPcup, sin_phi, nMax) < 0)
		    return -1;  // error
	}
	else
	{
		if (WMM_PcupHigh(LegendreFunction->Pcup, LegendreFunction->dPcup, sin_phi, nMax) < 0)
		    return -2;  // error
	}

	return 0;   // OK
}

int WMM_Summation(WMMtype_LegendreFunction * LegendreFunction,
		       WMMtype_SphericalHarmonicVariables * SphVariables,
		       WMMtype_CoordSpherical * CoordSpherical, WMMtype_MagneticResults * MagneticResults)
{
	/* Computes Geomagnetic Field Elements X, Y and Z in Spherical coordinate system using
	   spherical harmonic summation.

	   The vector Magnetic field is given by -grad V, where V is Geomagnetic scalar potential
	   The gradient in spherical coordinates is given by:

	   dV ^     1 dV ^        1     dV ^
	   grad V = -- r  +  - -- t  +  -------- -- p
	   dr       r dt       r sin(t) dp

	   INPUT :  LegendreFunction
	   MagneticModel
	   SphVariables
	   CoordSpherical
	   OUTPUT : MagneticResults

	   CALLS : WMM_SummationSpecial

	   Manoj Nair, June, 2009 Manoj.C.Nair@Noaa.Gov
	 */

    uint16_t m, n, index;
	float cos_phi;

	MagneticResults->Bz = 0.0;
	MagneticResults->By = 0.0;
	MagneticResults->Bx = 0.0;

	for (n = 1; n <= MagneticModel->nMax; n++)
	{
		for (m = 0; m <= n; m++)
		{
			index = (n * (n + 1) / 2 + m);

/*		    nMax  	(n+2) 	  n     m            m           m
	Bz =   -SUM (a/r)   (n+1) SUM  [g cos(m p) + h sin(m p)] P (sin(phi))
			n=1      	      m=0   n            n           n  */
/* Equation 12 in the WMM Technical report.  Derivative with respect to radius.*/
			MagneticResults->Bz -=
			    SphVariables->RelativeRadiusPower[n] *
			    (WMM_get_main_field_coeff_g(index) *
			     SphVariables->cos_mlambda[m] + WMM_get_main_field_coeff_h(index) * SphVariables->sin_mlambda[m])
			    * (float)(n + 1) * LegendreFunction->Pcup[index];

/*		  1 nMax  (n+2)    n     m            m           m
	By =    SUM (a/r) (m)  SUM  [g cos(m p) + h sin(m p)] dP (sin(phi))
		   n=1             m=0   n            n           n  */
/* Equation 11 in the WMM Technical report. Derivative with respect to longitude, divided by radius. */
			MagneticResults->By +=
			    SphVariables->RelativeRadiusPower[n] *
			    (WMM_get_main_field_coeff_g(index) *
			     SphVariables->sin_mlambda[m] - WMM_get_main_field_coeff_h(index) * SphVariables->cos_mlambda[m])
			    * (float)(m) * LegendreFunction->Pcup[index];
/*		   nMax  (n+2) n     m            m           m
	Bx = - SUM (a/r)   SUM  [g cos(m p) + h sin(m p)] dP (sin(phi))
		   n=1         m=0   n            n           n  */
/* Equation 10  in the WMM Technical report. Derivative with respect to latitude, divided by radius. */

			MagneticResults->Bx -=
			    SphVariables->RelativeRadiusPower[n] *
			    (WMM_get_main_field_coeff_g(index) *
			     SphVariables->cos_mlambda[m] + WMM_get_main_field_coeff_h(index) * SphVariables->sin_mlambda[m])
			    * LegendreFunction->dPcup[index];

		}
	}

	cos_phi = cos(DEG2RAD(CoordSpherical->phig));
	if (fabs(cos_phi) > 1.0e-10)
	{
		MagneticResults->By = MagneticResults->By / cos_phi;
	}
	else
	{
        /* Special calculation for component - By - at Geographic poles.
         * If the user wants to avoid using this function,  please make sure that
         * the latitude is not exactly +/-90. An option is to make use the function
         * WMM_CheckGeographicPoles.
         */
		if (WMM_SummationSpecial(SphVariables, CoordSpherical, MagneticResults) < 0)
		    return -1;  // error
	}

	return 0;   // OK
}

int WMM_SecVarSummation(WMMtype_LegendreFunction * LegendreFunction,
			     WMMtype_SphericalHarmonicVariables *
			     SphVariables, WMMtype_CoordSpherical * CoordSpherical, WMMtype_MagneticResults * MagneticResults)
{
	/*This Function sums the secular variation coefficients to get the secular variation of the Magnetic vector.
	   INPUT :  LegendreFunction
	   MagneticModel
	   SphVariables
	   CoordSpherical
	   OUTPUT : MagneticResults

	   CALLS : WMM_SecVarSummationSpecial

	 */

    uint16_t m, n, index;
	float cos_phi;

	MagneticModel->SecularVariationUsed = TRUE;

	MagneticResults->Bz = 0.0;
	MagneticResults->By = 0.0;
	MagneticResults->Bx = 0.0;

	for (n = 1; n <= MagneticModel->nMaxSecVar; n++)
	{
		for (m = 0; m <= n; m++)
		{
			index = (n * (n + 1) / 2 + m);

/*		    nMax  	(n+2) 	  n     m            m           m
	Bz =   -SUM (a/r)   (n+1) SUM  [g cos(m p) + h sin(m p)] P (sin(phi))
			n=1      	      m=0   n            n           n  */
/*  Derivative with respect to radius.*/
			MagneticResults->Bz -=
			    SphVariables->RelativeRadiusPower[n] *
			    (WMM_get_secular_var_coeff_g(index) *
			     SphVariables->cos_mlambda[m] + WMM_get_secular_var_coeff_h(index) * SphVariables->sin_mlambda[m])
			    * (float)(n + 1) * LegendreFunction->Pcup[index];

/*		  1 nMax  (n+2)    n     m            m           m
	By =    SUM (a/r) (m)  SUM  [g cos(m p) + h sin(m p)] dP (sin(phi))
		   n=1             m=0   n            n           n  */
/* Derivative with respect to longitude, divided by radius. */
			MagneticResults->By +=
			    SphVariables->RelativeRadiusPower[n] *
			    (WMM_get_secular_var_coeff_g(index) *
			     SphVariables->sin_mlambda[m] - WMM_get_secular_var_coeff_h(index) * SphVariables->cos_mlambda[m])
			    * (float)(m) * LegendreFunction->Pcup[index];
/*		   nMax  (n+2) n     m            m           m
	Bx = - SUM (a/r)   SUM  [g cos(m p) + h sin(m p)] dP (sin(phi))
		   n=1         m=0   n            n           n  */
/* Derivative with respect to latitude, divided by radius. */

			MagneticResults->Bx -=
			    SphVariables->RelativeRadiusPower[n] *
			    (WMM_get_secular_var_coeff_g(index) *
			     SphVariables->cos_mlambda[m] + WMM_get_secular_var_coeff_h(index) * SphVariables->sin_mlambda[m])
			    * LegendreFunction->dPcup[index];
		}
	}
	cos_phi = cos(DEG2RAD(CoordSpherical->phig));
	if (fabs(cos_phi) > 1.0e-10)
	{
		MagneticResults->By = MagneticResults->By / cos_phi;
	}
	else
		/* Special calculation for component By at Geographic poles */
	{
		if (WMM_SecVarSummationSpecial(SphVariables, CoordSpherical, MagneticResults) < 0)
		    return -1;  // error
	}

    return 0;   // OK
}

int WMM_RotateMagneticVector(WMMtype_CoordSpherical * CoordSpherical,
				  WMMtype_CoordGeodetic * CoordGeodetic,
				  WMMtype_MagneticResults * MagneticResultsSph, WMMtype_MagneticResults * MagneticResultsGeo)
	/* Rotate the Magnetic Vectors to Geodetic Coordinates
	   Manoj Nair, June, 2009 Manoj.C.Nair@Noaa.Gov
	   Equation 16, WMM Technical report

	   INPUT : CoordSpherical : Data structure WMMtype_CoordSpherical with the following elements
	   float lambda; ( longitude)
	   float phig; ( geocentric latitude )
	   float r;       ( distance from the center of the ellipsoid)

	   CoordGeodetic : Data structure WMMtype_CoordGeodetic with the following elements
	   float lambda; (longitude)
	   float phi; ( geodetic latitude)
	   float HeightAboveEllipsoid; (height above the ellipsoid (HaE) )
	   float HeightAboveGeoid;(height above the Geoid )

	   MagneticResultsSph : Data structure WMMtype_MagneticResults with the following elements
	   float Bx;     North
	   float By;       East
	   float Bz;    Down

	   OUTPUT: MagneticResultsGeo Pointer to the data structure WMMtype_MagneticResults, with the following elements
	   float Bx;     North
	   float By;       East
	   float Bz;    Down

	   CALLS : none

	 */
{
	/* Difference between the spherical and Geodetic latitudes */
	float Psi = (M_PI / 180) * (CoordSpherical->phig - CoordGeodetic->phi);

	/* Rotate spherical field components to the Geodeitic system */
	MagneticResultsGeo->Bz = MagneticResultsSph->Bx * sin(Psi) + MagneticResultsSph->Bz * cos(Psi);
	MagneticResultsGeo->Bx = MagneticResultsSph->Bx * cos(Psi) - MagneticResultsSph->Bz * sin(Psi);
	MagneticResultsGeo->By = MagneticResultsSph->By;

    return 0;
}

int WMM_CalculateGeoMagneticElements(WMMtype_MagneticResults * MagneticResultsGeo, WMMtype_GeoMagneticElements * GeoMagneticElements)

	/* Calculate all the Geomagnetic elements from X,Y and Z components
	   INPUT     MagneticResultsGeo   Pointer to data structure with the following elements
	   float Bx;    ( North )
	   float By;      ( East )
	   float Bz;    ( Down )
	   OUTPUT    GeoMagneticElements    Pointer to data structure with the following elements
	   float Decl; (Angle between the magnetic field vector and true north, positive east)
	   float Incl; Angle between the magnetic field vector and the horizontal plane, positive down
	   float F; Magnetic Field Strength
	   float H; Horizontal Magnetic Field Strength
	   float X; Northern component of the magnetic field vector
	   float Y; Eastern component of the magnetic field vector
	   float Z; Downward component of the magnetic field vector
	   CALLS : none
	 */
{
	GeoMagneticElements->X = MagneticResultsGeo->Bx;
	GeoMagneticElements->Y = MagneticResultsGeo->By;
	GeoMagneticElements->Z = MagneticResultsGeo->Bz;

	GeoMagneticElements->H = sqrt(MagneticResultsGeo->Bx * MagneticResultsGeo->Bx + MagneticResultsGeo->By * MagneticResultsGeo->By);
	GeoMagneticElements->F = sqrt(GeoMagneticElements->H * GeoMagneticElements->H + MagneticResultsGeo->Bz * MagneticResultsGeo->Bz);
	GeoMagneticElements->Decl = RAD2DEG(atan2(GeoMagneticElements->Y, GeoMagneticElements->X));
	GeoMagneticElements->Incl = RAD2DEG(atan2(GeoMagneticElements->Z, GeoMagneticElements->H));

    return 0;   // OK
}

int WMM_CalculateSecularVariation(WMMtype_MagneticResults * MagneticVariation, WMMtype_GeoMagneticElements * MagneticElements)
/*This takes the Magnetic Variation in x, y, and z and uses it to calculate the secular variation of each of the Geomagnetic elements.
	INPUT     MagneticVariation   Data structure with the following elements
				float Bx;    ( North )
				float By;	  ( East )
				float Bz;    ( Down )
	OUTPUT   MagneticElements   Pointer to the data  structure with the following elements updated
			float Decldot; Yearly Rate of change in declination
			float Incldot; Yearly Rate of change in inclination
			float Fdot; Yearly rate of change in Magnetic field strength
			float Hdot; Yearly rate of change in horizontal field strength
			float Xdot; Yearly rate of change in the northern component
			float Ydot; Yearly rate of change in the eastern component
			float Zdot; Yearly rate of change in the downward component
			float GVdot;Yearly rate of chnage in grid variation
	CALLS : none

*/
{
	MagneticElements->Xdot = MagneticVariation->Bx;
	MagneticElements->Ydot = MagneticVariation->By;
	MagneticElements->Zdot = MagneticVariation->Bz;
	MagneticElements->Hdot = (MagneticElements->X * MagneticElements->Xdot + MagneticElements->Y * MagneticElements->Ydot) / MagneticElements->H;	//See equation 19 in the WMM technical report
	MagneticElements->Fdot =
	    (MagneticElements->X * MagneticElements->Xdot +
	     MagneticElements->Y * MagneticElements->Ydot + MagneticElements->Z * MagneticElements->Zdot) / MagneticElements->F;
	MagneticElements->Decldot =
	    180.0 / M_PI * (MagneticElements->X * MagneticElements->Ydot -
			    MagneticElements->Y * MagneticElements->Xdot) / (MagneticElements->H * MagneticElements->H);
	MagneticElements->Incldot =
	    180.0 / M_PI * (MagneticElements->H * MagneticElements->Zdot -
			    MagneticElements->Z * MagneticElements->Hdot) / (MagneticElements->F * MagneticElements->F);
	MagneticElements->GVdot = MagneticElements->Decldot;

    return 0;   // OK
}

int WMM_PcupHigh(float *Pcup, float *dPcup, float x, uint16_t nMax)

/*	This function evaluates all of the Schmidt-semi normalized associated Legendre
	functions up to degree nMax. The functions are initially scaled by
	10^280 sin^m in order to minimize the effects of underflow at large m
	near the poles (see Holmes and Featherstone 2002, J. Geodesy, 76, 279-299).
	Note that this function performs the same operation as WMM_PcupLow.
	However this function also can be used for high degree (large nMax) models.

	Calling Parameters:
		INPUT
			nMax:	 Maximum spherical harmonic degree to compute.
			x:		cos(colatitude) or sin(latitude).

		OUTPUT
			Pcup:	A vector of all associated Legendgre polynomials evaluated at
					x up to nMax. The lenght must by greater or equal to (nMax+1)*(nMax+2)/2.
		  dPcup:   Derivative of Pcup(x) with respect to latitude

		CALLS : none
	Notes:

  Adopted from the FORTRAN code written by Mark Wieczorek September 25, 2005.

  Manoj Nair, Nov, 2009 Manoj.C.Nair@Noaa.Gov

  Change from the previous version
  The prevous version computes the derivatives as
  dP(n,m)(x)/dx, where x = sin(latitude) (or cos(colatitude) ).
  However, the WMM Geomagnetic routines requires dP(n,m)(x)/dlatitude.
  Hence the derivatives are multiplied by sin(latitude).
  Removed the options for CS phase and normalizations.

  Note: In geomagnetism, the derivatives of ALF are usually found with
  respect to the colatitudes. Here the derivatives are found with respect
  to the latitude. The difference is a sign reversal for the derivative of
  the Associated Legendre Functions.

  The derivates can't be computed for latitude = |90| degrees.
	*/
{
    uint16_t    k, kstart, m, n;
    float       pm2, pm1, pmm, plm, rescalem, z, scalef;

	float       *f1 = (float *) MALLOC(sizeof(float) * NUMPCUP);
	float       *f2 = (float *) MALLOC(sizeof(float) * NUMPCUP);
	float       *PreSqr = (float *) MALLOC(sizeof(float) * NUMPCUP);

    if (!PreSqr || !f2 || !f1)
    {   // memory allocation error
        if (PreSqr) FREE(PreSqr);
        if (f2) FREE(f2);
        if (f1) FREE(f1);

        return -1;
    }

	if (fabs(x) == 1.0)
	{
	    FREE(PreSqr);
	    FREE(f2);
	    FREE(f1);

		// printf("Error in PcupHigh: derivative cannot be calculated at poles\n");
		return -2;
	}

	scalef = 1.0e-280;

	for (n = 0; n <= 2 * nMax + 1; ++n)
		PreSqr[n] = sqrt((float)(n));

	k = 2;

	for (n = 2; n <= nMax; n++)
	{
		k = k + 1;
		f1[k] = (float)(2 * n - 1) / (float)(n);
		f2[k] = (float)(n - 1) / (float)(n);
		for (m = 1; m <= n - 2; m++)
		{
			k = k + 1;
			f1[k] = (float)(2 * n - 1) / PreSqr[n + m] / PreSqr[n - m];
			f2[k] = PreSqr[n - m - 1] * PreSqr[n + m - 1] / PreSqr[n + m] / PreSqr[n - m];
		}
		k = k + 2;
	}

	/*z = sin (geocentric latitude) */
	z = sqrt((1.0 - x) * (1.0 + x));
	pm2 = 1.0;
	Pcup[0] = 1.0;
	dPcup[0] = 0.0;
	if (nMax == 0)
    {
        FREE(PreSqr);
        FREE(f2);
        FREE(f1);
        return -3;
    }
	pm1 = x;
	Pcup[1] = pm1;
	dPcup[1] = z;
	k = 1;

	for (n = 2; n <= nMax; n++)
	{
		k = k + n;
		plm = f1[k] * x * pm1 - f2[k] * pm2;
		Pcup[k] = plm;
		dPcup[k] = (float)(n) * (pm1 - x * plm) / z;
		pm2 = pm1;
		pm1 = plm;
	}

	pmm = PreSqr[2] * scalef;
	rescalem = 1.0 / scalef;
	kstart = 0;

	for (m = 1; m <= nMax - 1; ++m)
	{
		rescalem = rescalem * z;

		/* Calculate Pcup(m,m) */
		kstart = kstart + m + 1;
		pmm = pmm * PreSqr[2 * m + 1] / PreSqr[2 * m];
		Pcup[kstart] = pmm * rescalem / PreSqr[2 * m + 1];
		dPcup[kstart] = -((float)(m) * x * Pcup[kstart] / z);
		pm2 = pmm / PreSqr[2 * m + 1];
		/* Calculate Pcup(m+1,m) */
		k = kstart + m + 1;
		pm1 = x * PreSqr[2 * m + 1] * pm2;
		Pcup[k] = pm1 * rescalem;
		dPcup[k] = ((pm2 * rescalem) * PreSqr[2 * m + 1] - x * (float)(m + 1) * Pcup[k]) / z;
		/* Calculate Pcup(n,m) */
		for (n = m + 2; n <= nMax; ++n)
		{
			k = k + n;
			plm = x * f1[k] * pm1 - f2[k] * pm2;
			Pcup[k] = plm * rescalem;
			dPcup[k] = (PreSqr[n + m] * PreSqr[n - m] * (pm1 * rescalem) - (float)(n) * x * Pcup[k]) / z;
			pm2 = pm1;
			pm1 = plm;
		}
	}

	/* Calculate Pcup(nMax,nMax) */
	rescalem = rescalem * z;
	kstart = kstart + m + 1;
	pmm = pmm / PreSqr[2 * nMax];
	Pcup[kstart] = pmm * rescalem;
	dPcup[kstart] = -(float)(nMax) * x * Pcup[kstart] / z;

	// *********
	// free allocated memory

	FREE(PreSqr);
    FREE(f2);
	FREE(f1);

    // *********

	return 0;   // OK
}

int WMM_PcupLow(float *Pcup, float *dPcup, float x, uint16_t nMax)

/*   This function evaluates all of the Schmidt-semi normalized associated Legendre
	functions up to degree nMax.

	Calling Parameters:
		INPUT
			nMax:	 Maximum spherical harmonic degree to compute.
			x:		cos(colatitude) or sin(latitude).

		OUTPUT
			Pcup:	A vector of all associated Legendgre polynomials evaluated at
					x up to nMax.
		   dPcup: Derivative of Pcup(x) with respect to latitude

	Notes: Overflow may occur if nMax > 20 , especially for high-latitudes.
	Use WMM_PcupHigh for large nMax.

   Writted by Manoj Nair, June, 2009 . Manoj.C.Nair@Noaa.Gov.

  Note: In geomagnetism, the derivatives of ALF are usually found with
  respect to the colatitudes. Here the derivatives are found with respect
  to the latitude. The difference is a sign reversal for the derivative of
  the Associated Legendre Functions.
*/
{
    uint16_t    n, m, index, index1, index2;
    float       k, z;

    float       *schmidtQuasiNorm = (float *) MALLOC(sizeof(float) * NUMPCUP);
    if (!schmidtQuasiNorm)
    {   // memory allocation error
        return -1;
    }

	Pcup[0] = 1.0;
	dPcup[0] = 0.0;

	/*sin (geocentric latitude) - sin_phi */
	z = sqrt((1.0 - x) * (1.0 + x));

	/*       First, Compute the Gauss-normalized associated Legendre  functions */
	for (n = 1; n <= nMax; n++)
	{
		for (m = 0; m <= n; m++)
		{
			index = (n * (n + 1) / 2 + m);
			if (n == m)
			{
				index1 = (n - 1) * n / 2 + m - 1;
				Pcup[index] = z * Pcup[index1];
				dPcup[index] = z * dPcup[index1] + x * Pcup[index1];
			}
			else
			if (n == 1 && m == 0)
			{
				index1 = (n - 1) * n / 2 + m;
				Pcup[index] = x * Pcup[index1];
				dPcup[index] = x * dPcup[index1] - z * Pcup[index1];
			}
			else
			if (n > 1 && n != m)
			{
				index1 = (n - 2) * (n - 1) / 2 + m;
				index2 = (n - 1) * n / 2 + m;
				if (m > n - 2)
				{
					Pcup[index] = x * Pcup[index2];
					dPcup[index] = x * dPcup[index2] - z * Pcup[index2];
				}
				else
				{
					k = (float)(((n - 1) * (n - 1)) - (m * m)) / (float)((2 * n - 1)
											     * (2 * n - 3));
					Pcup[index] = x * Pcup[index2] - k * Pcup[index1];
					dPcup[index] = x * dPcup[index2] - z * Pcup[index2] - k * dPcup[index1];
				}
			}
		}
	}
/*Compute the ration between the Gauss-normalized associated Legendre
  functions and the Schmidt quasi-normalized version. This is equivalent to
  sqrt((m==0?1:2)*(n-m)!/(n+m!))*(2n-1)!!/(n-m)!  */

	schmidtQuasiNorm[0] = 1.0;
	for (n = 1; n <= nMax; n++)
	{
		index = (n * (n + 1) / 2);
		index1 = (n - 1) * n / 2;
		/* for m = 0 */
		schmidtQuasiNorm[index] = schmidtQuasiNorm[index1] * (float)(2 * n - 1) / (float)n;

		for (m = 1; m <= n; m++)
		{
			index = (n * (n + 1) / 2 + m);
			index1 = (n * (n + 1) / 2 + m - 1);
			schmidtQuasiNorm[index] = schmidtQuasiNorm[index1] * sqrt((float)((n - m + 1) * (m == 1 ? 2 : 1)) / (float)(n + m));
		}

	}

/* Converts the  Gauss-normalized associated Legendre
	  functions to the Schmidt quasi-normalized version using pre-computed
	  relation stored in the variable schmidtQuasiNorm */

	for (n = 1; n <= nMax; n++)
	{
		for (m = 0; m <= n; m++)
		{
			index = (n * (n + 1) / 2 + m);
			Pcup[index] = Pcup[index] * schmidtQuasiNorm[index];
			dPcup[index] = -dPcup[index] * schmidtQuasiNorm[index];
			/* The sign is changed since the new WMM routines use derivative with respect to latitude
			   insted of co-latitude */
		}
	}

	FREE(schmidtQuasiNorm);

	return 0;   // OK
}

int WMM_SummationSpecial(WMMtype_SphericalHarmonicVariables *
			      SphVariables, WMMtype_CoordSpherical * CoordSpherical, WMMtype_MagneticResults * MagneticResults)
	/* Special calculation for the component By at Geographic poles.
	   Manoj Nair, June, 2009 manoj.c.nair@noaa.gov
	   INPUT: MagneticModel
	   SphVariables
	   CoordSpherical
	   OUTPUT: MagneticResults
	   CALLS : none
	   See Section 1.4, "SINGULARITIES AT THE GEOGRAPHIC POLES", WMM Technical report

	 */
{
    uint16_t    n, index;
    float       k, sin_phi;
    float       schmidtQuasiNorm1;
    float       schmidtQuasiNorm2;
    float       schmidtQuasiNorm3;

    float       *PcupS = (float *) MALLOC(sizeof(float) * NUMPCUPS);
    if (!PcupS)
        return -1;  // memory allocation error

	PcupS[0] = 1;
	schmidtQuasiNorm1 = 1.0;

	MagneticResults->By = 0.0;
	sin_phi = sin(DEG2RAD(CoordSpherical->phig));

	for (n = 1; n <= MagneticModel->nMax; n++)
	{

		/*Compute the ration between the Gauss-normalized associated Legendre
		   functions and the Schmidt quasi-normalized version. This is equivalent to
		   sqrt((m==0?1:2)*(n-m)!/(n+m!))*(2n-1)!!/(n-m)!  */

		index = (n * (n + 1) / 2 + 1);
		schmidtQuasiNorm2 = schmidtQuasiNorm1 * (float)(2 * n - 1) / (float)n;
		schmidtQuasiNorm3 = schmidtQuasiNorm2 * sqrt((float)(n * 2) / (float)(n + 1));
		schmidtQuasiNorm1 = schmidtQuasiNorm2;
		if (n == 1)
		{
			PcupS[n] = PcupS[n - 1];
		}
		else
		{
			k = (float)(((n - 1) * (n - 1)) - 1) / (float)((2 * n - 1) * (2 * n - 3));
			PcupS[n] = sin_phi * PcupS[n - 1] - k * PcupS[n - 2];
		}

/*		  1 nMax  (n+2)    n     m            m           m
	By =    SUM (a/r) (m)  SUM  [g cos(m p) + h sin(m p)] dP (sin(phi))
		   n=1             m=0   n            n           n  */
/* Equation 11 in the WMM Technical report. Derivative with respect to longitude, divided by radius. */

		MagneticResults->By +=
		    SphVariables->RelativeRadiusPower[n] *
		    (WMM_get_main_field_coeff_g(index) *
		     SphVariables->sin_mlambda[1] - WMM_get_main_field_coeff_h(index) * SphVariables->cos_mlambda[1])
		    * PcupS[n] * schmidtQuasiNorm3;
	}

	FREE(PcupS);

	return 0;   // OK
}

int WMM_SecVarSummationSpecial(WMMtype_SphericalHarmonicVariables *
				    SphVariables, WMMtype_CoordSpherical * CoordSpherical, WMMtype_MagneticResults * MagneticResults)
{
	/*Special calculation for the secular variation summation at the poles.

	   INPUT: MagneticModel
	   SphVariables
	   CoordSpherical
	   OUTPUT: MagneticResults
	   CALLS : none

	 */
    uint16_t    n, index;
    float       k, sin_phi;
    float       schmidtQuasiNorm1;
    float       schmidtQuasiNorm2;
    float       schmidtQuasiNorm3;

    float       *PcupS = (float *) MALLOC(sizeof(float) * NUMPCUPS);
    if (!PcupS)
        return -1;  // memory allocation error

	PcupS[0] = 1;
	schmidtQuasiNorm1 = 1.0;

	MagneticResults->By = 0.0;
	sin_phi = sin(DEG2RAD(CoordSpherical->phig));

	for (n = 1; n <= MagneticModel->nMaxSecVar; n++)
	{
		index = (n * (n + 1) / 2 + 1);
		schmidtQuasiNorm2 = schmidtQuasiNorm1 * (float)(2 * n - 1) / (float)n;
		schmidtQuasiNorm3 = schmidtQuasiNorm2 * sqrt((float)(n * 2) / (float)(n + 1));
		schmidtQuasiNorm1 = schmidtQuasiNorm2;
		if (n == 1)
		{
			PcupS[n] = PcupS[n - 1];
		}
		else
		{
			k = (float)(((n - 1) * (n - 1)) - 1) / (float)((2 * n - 1) * (2 * n - 3));
			PcupS[n] = sin_phi * PcupS[n - 1] - k * PcupS[n - 2];
		}

/*		  1 nMax  (n+2)    n     m            m           m
	By =    SUM (a/r) (m)  SUM  [g cos(m p) + h sin(m p)] dP (sin(phi))
		   n=1             m=0   n            n           n  */
/* Derivative with respect to longitude, divided by radius. */

		MagneticResults->By +=
		    SphVariables->RelativeRadiusPower[n] *
		    (WMM_get_secular_var_coeff_g(index) *
		     SphVariables->sin_mlambda[1] - WMM_get_secular_var_coeff_h(index) * SphVariables->cos_mlambda[1])
		    * PcupS[n] * schmidtQuasiNorm3;
	}

	FREE(PcupS);

	return 0;   // OK
}

/**
 * @brief Comput the MainFieldCoeffH accounting for the date
 */
float WMM_get_main_field_coeff_g(uint16_t index) 
{	
	if (index >= NUMTERMS)
		return 0;

    uint16_t n, m, sum_index, a, b;

    float coeff = CoeffFile[index][2];
	
	a = MagneticModel->nMaxSecVar;
	b = (a * (a + 1) / 2 + a);
	for (n = 1; n <= MagneticModel->nMax; n++)
	{
		for (m = 0; m <= n; m++)
		{
			
			sum_index = (n * (n + 1) / 2 + m);
			
			/* Hacky for now, will solve for which conditions need summing analytically */
			if (sum_index != index)
				continue;
			
			if (index <= b) 
				coeff += (decimal_date - MagneticModel->epoch) * WMM_get_secular_var_coeff_g(sum_index);
			
		}
	}
	
	return coeff;
}

float WMM_get_main_field_coeff_h(uint16_t index) 
{	
	if (index >= NUMTERMS)
		return 0;
	
    uint16_t n, m, sum_index, a, b;
	float coeff = CoeffFile[index][3];
	
	a = MagneticModel->nMaxSecVar;
	b = (a * (a + 1) / 2 + a);
	for (n = 1; n <= MagneticModel->nMax; n++)
	{
		for (m = 0; m <= n; m++)
		{
			
			sum_index = (n * (n + 1) / 2 + m);
			
			/* Hacky for now, will solve for which conditions need summing analytically */
			if (sum_index != index)
				continue;
			
			if (index <= b) 
				coeff += (decimal_date - MagneticModel->epoch) * WMM_get_secular_var_coeff_h(sum_index);
		}
	}
	
	return coeff;	
}

float WMM_get_secular_var_coeff_g(uint16_t index) 
{		
	if (index >= NUMTERMS)
		return 0;
	
	return CoeffFile[index][4];
}

float WMM_get_secular_var_coeff_h(uint16_t index) 
{		
	if (index >= NUMTERMS)
		return 0;
	
	return CoeffFile[index][5];
}

int WMM_DateToYear(uint16_t month, uint16_t day, uint16_t year)
// Converts a given calendar date into a decimal year
{
	uint16_t temp = 0;	// Total number of days
	uint16_t MonthDays[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	uint16_t ExtraDay = 0;
	uint16_t i;

	if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
		ExtraDay = 1;
	MonthDays[2] += ExtraDay;

	/******************Validation********************************/

	if (month <= 0 || month > 12)
		return -1;  // error

	if (day <= 0 || day > MonthDays[month])
		return -2;  // error

	/****************Calculation of t***************************/
	for (i = 1; i <= month; i++)
		temp += MonthDays[i - 1];
	temp += day;
	
	decimal_date = year + (temp - 1) / (365.0 + ExtraDay);

	return 0;   // OK
}

int WMM_GeodeticToSpherical(WMMtype_CoordGeodetic * CoordGeodetic, WMMtype_CoordSpherical * CoordSpherical)
// Converts Geodetic coordinates to Spherical coordinates
// Convert geodetic coordinates, (defined by the WGS-84
// reference ellipsoid), to Earth Centered Earth Fixed Cartesian
// coordinates, and then to spherical coordinates.
{
	float CosLat, SinLat, rc, xp, zp;	// all local variables

	CosLat = cos(DEG2RAD(CoordGeodetic->phi));
	SinLat = sin(DEG2RAD(CoordGeodetic->phi));

	// compute the local radius of curvature on the WGS-84 reference ellipsoid
	rc = Ellip->a / sqrt(1.0 - Ellip->epssq * SinLat * SinLat);

	// compute ECEF Cartesian coordinates of specified point (for longitude=0)

	xp = (rc + CoordGeodetic->HeightAboveEllipsoid) * CosLat;
	zp = (rc * (1.0 - Ellip->epssq) + CoordGeodetic->HeightAboveEllipsoid) * SinLat;

	// compute spherical radius and angle lambda and phi of specified point

	CoordSpherical->r = sqrt(xp * xp + zp * zp);
	CoordSpherical->phig = RAD2DEG(asin(zp / CoordSpherical->r));	// geocentric latitude
	CoordSpherical->lambda = CoordGeodetic->lambda;	// longitude

	return 0;   // OK
}
