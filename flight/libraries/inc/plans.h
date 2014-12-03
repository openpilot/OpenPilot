/**
 ******************************************************************************
 * @addtogroup OpenPilotLibraries OpenPilot Libraries
 * @{
 * @addtogroup Navigation
 * @brief setups RTH/PH and other pathfollower/pathplanner status
 * @{
 *
 * @file       plan.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************/
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

#ifndef PLANS_H_
#define PLANS_H_
#include <pios.h>
#include <pios_math.h>

/** \page standard Plans
   How to use this library
   \li Call plan_initialize() to init all needed struct and uavos at startup.<br>
   It may be safely called more than once.<br>

   \li Functions called plan_setup_* needs to be called once, every time the requested function is engaged.<br>

   \li Functions called plan_run_* are to be periodically called while the requested mode is engaged.<br>
 */

/**
 * @brief initialize UAVOs and structs used by this library
 */
void plan_initialize();

/**
 * @brief setup pathplanner/pathfollower for positionhold
 */
void plan_setup_positionHold();

/**
 * @brief setup pathplanner/pathfollower for return to base
 */
void plan_setup_returnToBase();

/**
 * @brief setup pathplanner/pathfollower for land
 */
void plan_setup_land();

/**
 * @brief execute land
 */
void plan_run_land();

/**
 * @brief setup pathplanner/pathfollower for braking
 */
void plan_setup_assistedcontrol(uint8_t timeout_occurred);

/**
 * @brief setup pathfollower for positionvario
 */
void plan_setup_PositionVarioFPV();
void plan_setup_PositionVarioLOS();
void plan_setup_PositionVarioNSEW();

/**
 * @brief run for positionvario
 */
void plan_run_PositionVarioFPV();
void plan_run_PositionVarioLOS();
void plan_run_PositionVarioNSEW();

/**
 * @brief setup pathplanner/pathfollower for AutoCruise
 */
void plan_setup_AutoCruise();

/**
 * @brief execute AutoCruise
 */
void plan_run_AutoCruise();

#endif /* PLANS_H_ */
