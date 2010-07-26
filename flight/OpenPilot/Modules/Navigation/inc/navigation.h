/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup NavigationModule Navigation Module
 * @brief Feeds Stabilization with input to fly to a given coordinate in space
 * @note This object updates the @ref AttitudeDesired "Attitude Desired" based on the 
 * comparison on the @ref NavigationDesired "Navigation Desired", @ref AttitudeActual
 * "Attitude Actual" and @ref FlightSituationActual "FlightSituation Actual"
 * @{ 
 *
 * @file       navigation.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Position stabilization module.
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
#ifndef NAVIGATION_H
#define NAVIGATION_H

int32_t NavigationInitialize();

#endif // NAVIGATION_H

/**
  * @}
  * @}
  */
