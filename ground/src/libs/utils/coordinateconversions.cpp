/**
 ******************************************************************************
 *
 * @file       coordinateconversions.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      General conversions with different coordinate systems.
 *             - all angles in deg
 *             - distances in meters
 *             - altitude above WGS-84 elipsoid
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

#include "coordinateconversions.h"
#include <stdint.h>
#include <QDebug>
#include <math.h>

#define RAD2DEG (180.0/M_PI)
#define DEG2RAD (M_PI/180.0)

namespace Utils {

CoordinateConversions::CoordinateConversions()
{

}

/**
  * Get rotation matrix from ECEF to NED for that LLA
  * @param[in] LLA Longitude latitude altitude for this location
  * @param[out] Rne[3][3] Rotation matrix
  */
void CoordinateConversions::RneFromLLA(double LLA[3], double Rne[3][3]){
    float sinLat, sinLon, cosLat, cosLon;

    sinLat=(float)sin(DEG2RAD*LLA[0]);
    sinLon=(float)sin(DEG2RAD*LLA[1]);
    cosLat=(float)cos(DEG2RAD*LLA[0]);
    cosLon=(float)cos(DEG2RAD*LLA[1]);

    Rne[0][0] = -sinLat*cosLon; Rne[0][1] = -sinLat*sinLon; Rne[0][2] = cosLat;
    Rne[1][0] = -sinLon;        Rne[1][1] = cosLon;         Rne[1][2] = 0;
    Rne[2][0] = -cosLat*cosLon; Rne[2][1] = -cosLat*sinLon; Rne[2][2] = -sinLat;
}

/**
  * Convert from LLA coordinates to ECEF coordinates
  * @param[in] LLA[3] latitude longitude alititude coordinates in
  * @param[out] ECEF[3] location in ECEF coordinates
  */
void CoordinateConversions::LLA2ECEF(double LLA[3], double ECEF[3]){
  const double a = 6378137.0;           // Equatorial Radius
  const double e = 8.1819190842622e-2;  // Eccentricity
  double sinLat, sinLon, cosLat, cosLon;
  double N;

        sinLat=sin(DEG2RAD*LLA[0]);
        sinLon=sin(DEG2RAD*LLA[1]);
        cosLat=cos(DEG2RAD*LLA[0]);
        cosLon=cos(DEG2RAD*LLA[1]);

        N = a / sqrt(1.0 - e*e*sinLat*sinLat);  //prime vertical radius of curvature

        ECEF[0] = (N+LLA[2])*cosLat*cosLon;
        ECEF[1] = (N+LLA[2])*cosLat*sinLon;
        ECEF[2] = ((1-e*e)*N + LLA[2]) * sinLat;
}

/**
  * Convert from ECEF coordinates to LLA coordinates
  * @param[in] ECEF[3] location in ECEF coordinates
  * @param[out] LLA[3] latitude longitude alititude coordinates
  */
int CoordinateConversions::ECEF2LLA(double ECEF[3], double LLA[3])
{
    const double a = 6378137.0;           // Equatorial Radius
    const double e = 8.1819190842622e-2;  // Eccentricity
    double x=ECEF[0], y=ECEF[1], z=ECEF[2];
    double Lat, N, NplusH, delta, esLat;
    uint16_t iter;

    LLA[1] = RAD2DEG*atan2(y,x);
    N = a;
    NplusH = N;
    delta = 1;
    Lat = 1;
    iter=0;

    while (((delta > 1.0e-14)||(delta < -1.0e-14)) && (iter < 100))
    {
        delta = Lat - atan(z / (sqrt(x*x + y*y)*(1-(N*e*e/NplusH))));
        Lat = Lat-delta;
        esLat = e*sin(Lat);
        N = a / sqrt(1 - esLat*esLat);
        NplusH = sqrt(x*x + y*y)/cos(Lat);
        iter += 1;
    }

    LLA[0] = RAD2DEG*Lat;
    LLA[2] = NplusH - N;

    if (iter==500) return (0);
    else return (1);
}

/**
  * Get the current location in Longitude, Latitude Altitude (above WSG-48 ellipsoid)
  * @param[in] BaseECEF the ECEF of the home location (in cm)
  * @param[in] NED the offset from the home location (in m)
  * @param[out] position three element double for position in degrees and meters
  * @returns
  *  @arg 0 success
  *  @arg -1 for failure
  */
int CoordinateConversions::GetLLA(double BaseECEFcm[3], double NED[3], double position[3])
{
    int i;
    // stored value is in cm, convert to m
    double BaseECEFm[3] = {BaseECEFcm[0] / 100, BaseECEFcm[1] / 100, BaseECEFcm[2] / 100};
    double BaseLLA[3];
    double ECEF[3];
    double Rne [3][3];

    // Get LLA address to compute conversion matrix
    ECEF2LLA(BaseECEFm, BaseLLA);
    RneFromLLA(BaseLLA, Rne);

    /* P = ECEF + Rne' * NED */
    for(i = 0; i < 3; i++)
        ECEF[i] = BaseECEFm[i] + Rne[0][i]*NED[0] + Rne[1][i]*NED[1] + Rne[2][i]*NED[2];

    ECEF2LLA(ECEF,position);

    return 0;
}

}
