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

#include "openpilot.h"
#include "navigation.h"
#include "navigationsettings.h"
#include "navigationdesired.h"
#include "attitudeactual.h"
#include "attitudedesired.h"
#include "flightsituationactual.h"
#include "manualcontrolcommand.h"
#include "systemsettings.h"


// Private constants
#define STACK_SIZE configMINIMAL_STACK_SIZE
#define TASK_PRIORITY (tskIDLE_PRIORITY+3)
#define DEG2RAD ( M_PI / 180.0 )
#define RAD2DEG ( 180.0 / M_PI )
#define MIN(a,b) ((a<b)?a:b)
#define MAX(a,b) ((a>b)?a:b)
#define SSQRT(x) ((x)>=0?sqrt(x):-sqrt(-x)) // signed square root
#define EARTHRAD 6378137
#define GRAVITY 9.81

// Private types

// Private variables
static xTaskHandle taskHandle;

// Private functions
static void navigationTask(void* parameters);
static float bound(float val, float min, float max);
static float angleDifference(float val);
static float fixAngle(float val,float min, float max);
static float sphereDistance(float lat1,float long1,float lat2,float long2);
static float sphereCourse(float lat1,float long1,float lat2,float long2);

/**
 * Module initialization
 */
int32_t NavigationInitialize()
{
	// Initialize variables

	// Start main task
	xTaskCreate(navigationTask, (signed char*)"Navigation", STACK_SIZE, NULL, TASK_PRIORITY, &taskHandle);

	return 0;
}

/**
 * Module task
 */
static void navigationTask(void* parameters)
{
	NavigationSettingsData navSettings;
	NavigationDesiredData navigationDesired;
	AttitudeActualData attitudeActual;
	AttitudeDesiredData attitudeDesired;
	FlightSituationActualData situationActual;
	ManualControlCommandData manualControl;
	SystemSettingsData systemSettings;
	portTickType lastSysTime;

	// flight safety values
	float maxPitch;
	float minPitch;
	float maxAccel;

	// helper variables
	float safeAccel;
	float safeDistance;
	float aoa,saoa;

	// intended flight direction
	float targetDistance;
	float targetVspeed;
	float targetTruePitch;
	float targetHeading;
	float targetPitch;
	float targetYaw;

	// turn direction
	float turnDirection;
	float turnSpeed;
	float turnAccel;


	// Main task loop
	lastSysTime = xTaskGetTickCount();
	while (1)
	{
		// Read settings and other objects
		NavigationSettingsGet(&navSettings);
		SystemSettingsGet(&systemSettings);
		ManualControlCommandGet(&manualControl);
		FlightSituationActualGet(&situationActual);
		NavigationDesiredGet(&navigationDesired);
		AttitudeActualGet(&attitudeActual);

		// horizontal distance to waypoint
		// (not 100% exact since the earth is not a perfect sphere, but
		// close enough for our purpose...)
		targetDistance = DEG2RAD * EARTHRAD * sphereDistance(
				situationActual.Latitude,situationActual.Longitude,
				navigationDesired.Latitude,navigationDesired.Longitude
			);

		// pitch to climb/sink to target altitude (attempts to reach
		// target altitude withing SettleTime seconds
		targetVspeed= (navigationDesired.Altitude - situationActual.Altitude)/navSettings.SettleTime;
		if ( targetVspeed >= situationActual.Airspeed )
		{
			targetTruePitch = 90.0;
		}
		else if ( targetVspeed <= -situationActual.Airspeed )
		{
			targetTruePitch = -90.0;
		}
		else
		{
			targetTruePitch = RAD2DEG * asin( targetVspeed / situationActual.Airspeed );
		}

		// course to target coordinate
		targetHeading = sphereCourse(
				situationActual.Latitude,situationActual.Longitude,
				navigationDesired.Latitude,navigationDesired.Longitude
			);

//printf("\n\n\nTarget: Heading: %2f Distance: %2f climbing %2f°\n",targetHeading,targetDistance,targetTruePitch);
//printf("distance: %f\n",sphereDistance(situationActual.Latitude,situationActual.Longitude,navigationDesired.Latitude,navigationDesired.Longitude));
		/**
		 * navigation for fixed wing planes
		 */
		if (systemSettings.AirframeType != SYSTEMSETTINGS_AIRFRAMETYPE_VTOL)
		{

			/**
			 * Fixed wing planes create lift through their airspeed
			 * therefore most maneuver limits depend on the current
			 * speed and situation.
			 */

			// acceleration:
			maxAccel=navSettings.AccelerationMax;
			/**
			 * Idea: at minSpeed, the plane can JUST compensate 1g
			 * safely.  The StallSpeed at any load factor is
			 * Vstall=Vs(0) / sqrt(load) so the maximum load for
			 * the current speed is maxLoad = (Vs(0)/V)²
			 */
			maxAccel = MIN( maxAccel , 
					GRAVITY * (situationActual.Airspeed*situationActual.Airspeed)
					/ (navSettings.SpeedMin * navSettings.SpeedMin)		
				);

			// maximum pitch:
			maxPitch=85.0;

			/**
			 * Idea: any climb decreases speed. Aside from drag
			 * speed decrease is vertical speed times g (9.81m/s²)
			 * make sure speed stays above min speed within
			 * SettleTime
			 */
			safeAccel = ( navSettings.SpeedSafe-situationActual.Airspeed ) / navSettings.SettleTime;
			if (safeAccel>=0)
			{
				// speed is below minimum speed. Do not allow any climbs!
				maxPitch = MIN(maxPitch,0);
			}
			else if (safeAccel>-GRAVITY)
			{
				// speed is getting low. Limit maximum climb!
				maxPitch = MIN(maxPitch,RAD2DEG*asin(-safeAccel/GRAVITY));
			}

			// minimum pitch:
			minPitch=-85.0;

			/**
			 * Idea: any dive increases speed. Aside from drag
			 * speed increase is vertical speed times g (9.81m/s²)
			 * make sure speed stays below max speed within
			 * SettleTime
			 */
			safeAccel = ( navSettings.SpeedMax-situationActual.Airspeed ) / navSettings.SettleTime;
			if (safeAccel<=0)
			{
				// speed is above maximum speed. Do not allow any dives!
				minPitch = MAX(minPitch,0);
			}
			else if (safeAccel<GRAVITY)
			{
				// speed is getting high. Limit maximum dive!
				minPitch = MAX(minPitch,-1.*RAD2DEG*asin(safeAccel/GRAVITY));
			}

			/**
			 * Idea: To change pitch, acceleration must be
			 * applied.  A pull up at maximum allowed pitch has a certain radius.
			 * If we are lower than that we are not allowed to 
			 */
			safeDistance = situationActual.Airspeed * situationActual.Airspeed / (maxAccel+GRAVITY);
//printf("safe distance: %f\n",safeDistance);
			if (safeDistance <= 0 || situationActual.Altitude<=navigationDesired.Altitude)
			{
				// altitude is below wanted altitude. Do not allow any dive at all.
				minPitch = MAX(minPitch,0);
			}
			else if ( safeDistance >= situationActual.Altitude - navigationDesired.Altitude)
			{
				// altitude is close to target altitude, limit negative pitch
				minPitch = MAX(minPitch,-1 * RAD2DEG * asin( (situationActual.Altitude - navigationDesired.Altitude) / safeDistance ));
			}

//printf("Safety: Pitch: %2f %2f Accel: %2fm/s²\n",minPitch,maxPitch,maxAccel);

			/**
			 * Base anted yaw and pitch on calculated angle of
			 * attack and side slip effects
			 *
			 * Note: This can lead to an oszillation between a
			 * flight with positive and negative angle of attack.
			 * Without this correction however, the plane will
			 * always "lag behind" it's desired altitude.
			 */

			// angle of attack in X and Y
			aoa = attitudeActual.Pitch - (RAD2DEG * atan2( situationActual.Climbrate, situationActual.Groundspeed));
			saoa = attitudeActual.Yaw-situationActual.Course;

			targetPitch = bound(targetTruePitch+aoa,minPitch,maxPitch);
			targetYaw = fixAngle(targetHeading+saoa,0.,360.);
//printf("Target                 Pitch: %2f Yaw: %2f \n",targetPitch,targetYaw);
//printf("Current Roll: %2f Pitch: %2f Yaw: %2f \n",attitudeActual.Roll,attitudeActual.Pitch,attitudeActual.Yaw);

			/**
			 * Now we have the current Pitch Yaw and Roll in
			 * AttitudeActual and the wanted course in targetPitch
			 * and targetYaw. We also have the safety limits. Now
			 * make a smooth and save transition.
			 * including all safety limits! Make a nice transition.
			 */

			// turn vector (Euler)
			/**
			 * The naive approach wants to fly along an Orthodrome - the quickest way to get from one angle to another
			 */
			turnDirection = sphereCourse(attitudeActual.Pitch,attitudeActual.Yaw,targetPitch,targetYaw);

			/**
			 * However if the target yaw is more than 90° away, this orthodrome would lead through a minimum or maximum point that
			 * could be beyond safe min/max pitch values!
			 */
			if (fabs(angleDifference(attitudeActual.Yaw-targetYaw))>90)
			{
				if (fabs(turnDirection)<90)
				{
					/**
					 * Positive turns (upward) can conflict with maxPitch
					 */
					if (RAD2DEG*acos(sin(fabs(DEG2RAD*turnDirection))*cos(DEG2RAD*attitudeActual.Pitch))>maxPitch)
					{
						/**
						 * In this case the turn has to be adjusted to a circle that won't conflict with maxPitch
						 */
//printf("maximum is %2f which is higher than %2f\n",RAD2DEG*acos(sin(DEG2RAD*turnDirection)*cos(DEG2RAD*attitudeActual.Pitch)),maxPitch);
						if (cos(DEG2RAD*attitudeActual.Pitch)>cos(DEG2RAD*maxPitch))
						{
							if (turnDirection>0)
							{
								turnDirection = RAD2DEG * asin( cos(DEG2RAD*maxPitch)/cos(DEG2RAD*attitudeActual.Pitch));
							}
							else
							{
								turnDirection = -RAD2DEG * asin( cos(DEG2RAD*maxPitch)/cos(DEG2RAD*attitudeActual.Pitch));
							}
						}
						else
						{
							/**
							 * something went wrong (attitude outside safe bounds?)- compensate safely
							 */
							turnDirection=(turnDirection>0?110:-110);
						}
//printf("limiting because of max!\n");
					}
				} 
				else
				{
					/**
					 * Downward turns can conflict with minPitch
					 */
					if (RAD2DEG*acos(sin(fabs(DEG2RAD*turnDirection))*cos(DEG2RAD*attitudeActual.Pitch))>-minPitch)
					{
//printf("minimum is %2f which is less than %2f\n",-1*RAD2DEG*acos(sin(fabs(DEG2RAD*turnDirection))*cos(DEG2RAD*attitudeActual.Pitch)),minPitch);
						/**
						 * In this case the turn has to be adjusted to a circle that won't conflict with minPitch
						 */
						if (cos(DEG2RAD*attitudeActual.Pitch)>cos(DEG2RAD*minPitch))
						{
							if (turnDirection>0) {
								turnDirection = 180 - RAD2DEG * asin( cos(DEG2RAD*minPitch)/cos(DEG2RAD*attitudeActual.Pitch));
							}
							else
							{
								turnDirection = -180 + RAD2DEG * asin( cos(DEG2RAD*minPitch)/cos(DEG2RAD*attitudeActual.Pitch));
							}
						}
						else
						{
							/**
							 * something went wrong (attitude outside safe bounds?)- compensate safely
							 */
							turnDirection=(turnDirection>0?45:-45);
						}
//printf("limiting because of min!\n");
					}
				}
			}

			/**
			 * Turn speed in °/s is the minimum of
			 * 	TurnSpeedFactor * degrees to turn
			 * and
			 * 	the maximum angular velocity achievable with
			 * 	less than maxAccel centripetal acceleration
			 * 	(taking gravity effect into account)
			 * but not less than 0
			 * Formula: acceleration = (DEG2RAD*turnSpeed)
			 *                         * airspeed
			 */
			turnSpeed = MAX(0,MIN(
					navSettings.TurnSpeedFactor * sphereDistance(
									attitudeActual.Pitch,attitudeActual.Yaw,
									targetPitch,targetYaw)
					,
						RAD2DEG * (maxAccel-( MIN(0,cos(DEG2RAD*turnDirection))*GRAVITY )) / situationActual.Airspeed
					));
//printf("turn distance is %2f °\n", sphereDistance(attitudeActual.Pitch,attitudeActual.Yaw,targetPitch,targetYaw));
//printf("safe speed is %2f °/s\n",RAD2DEG * (maxAccel-( MIN(0,cos(DEG2RAD*turnDirection))*GRAVITY )) / situationActual.Airspeed);
//printf("turn speed is %2f °/s\n",turnSpeed);
//printf("Airspeed is %2f °/s\n",situationActual.Airspeed);

			/**
			 * Compute wanted centripetal acceleration
			 */
			turnAccel = DEG2RAD* turnSpeed * situationActual.Airspeed;
//printf("Turn with: %2f at %2fm/s²\n",turnDirection,turnAccel);


			// Desired Attitude
			/**
			 * Now that we know which way the plane is supposed to
			 * go and at what acceleration, calculate the required
			 * roll angle to also compensate gravity during the
			 * turn
			 */
			attitudeDesired.Roll = RAD2DEG*atan2( 
							turnAccel * sin(DEG2RAD*turnDirection),
							turnAccel * cos(DEG2RAD*turnDirection) + cos(DEG2RAD*attitudeActual.Pitch)*GRAVITY
							);
			/**
			 * However this can also result in an upright flight
			 * with a load of less than 1g
			 * In this case, the effect on yaw is reversed.
			 * (Imagine rolling 45° right, then push the stick
			 * slightly. You will go left and down!)
			 * In this case we reverse the roll to descent into the
			 * right direction!
			 */
			if (fabs(angleDifference(attitudeDesired.Roll-turnDirection))>90) {
				attitudeDesired.Roll=-attitudeDesired.Roll;
			}
//printf("FINALS: roll %2f ",attitudeDesired.Roll);

			/**
			 * The desired Yaw and Pitch values are tricky.
			 * The current Stabilization module doesn't allow us to
			 * specify an angular speed or even centripetal
			 * acceleration instead we have to specify an
			 * orientation.
			 * According to tests, a formula of
			 * 	sqrt(turnAccel) * StabilizationForceFactor
			 * degrees lookahead gives reasonable results.
			 */
			attitudeDesired.Pitch = bound(
				attitudeActual.Pitch +  navSettings.StabilizationForceFactor * SSQRT(turnAccel * cos(DEG2RAD*turnDirection)),
				minPitch,
				maxPitch);
			attitudeDesired.Yaw = attitudeActual.Yaw + navSettings.StabilizationForceFactor * SSQRT(turnAccel * sin(DEG2RAD*turnDirection)) ;
//printf("pitch %2f  yaw %2f\n",attitudeDesired.Pitch-attitudeActual.Pitch,attitudeDesired.Yaw-attitudeActual.Yaw);


			/**
			 * TODO: Make a real PID loop for the throttle.
			 */
			attitudeDesired.Throttle=0.8;
			

		}

		// Write actuator desired (if not in manual mode)
		if ( manualControl.FlightMode == MANUALCONTROLCOMMAND_FLIGHTMODE_AUTO )
		{
			AttitudeDesiredSet(&attitudeDesired);
		}

		// Wait until next update
		vTaskDelayUntil(&lastSysTime, navSettings.UpdatePeriod / portTICK_RATE_MS );
	}
}

/**
 * Bound input value between limits
 */
static float bound(float val, float min, float max)
{
	if ( val < min )
	{
		val = min;
	}
	else if ( val > max )
	{
		val = max;
	}
	return val;
}

/**
 * Fix result of angular differences
 */
static float angleDifference(float val)
{
	while ( val < -180.0 )
	{
		val += 360.0;
	}
	while ( val > 180.0 )
	{
		val -= 360.0;
	}
	return val;
}

/**
 * Make sure an angular value stays between correct bounds
 */
static float fixAngle(float val,float min, float max)
{
	while ( val < min )
	{
		val += 360.0;
	}
	while ( val >= max )
	{
		val -= 360.0;
	}
	return val;
}

/**
 * calculate spherical distance and course between two coordinate pairs
 * see http://de.wikipedia.org/wiki/Orthodrome for details
 */
static float sphereDistance(float lat1, float long1, float lat2, float long2)
{
	float zeta=(RAD2DEG * acos (
		sin(DEG2RAD*lat1) * sin(DEG2RAD*lat2)
		+ cos(DEG2RAD*lat1) * cos(DEG2RAD*lat2) * cos(DEG2RAD*(long2-long1))
	));
	if (isnan(zeta)) {
		zeta=0;
	}
	return zeta;

}
static float sphereCourse(float lat1, float long1, float lat2, float long2)
{
	float zeta = sphereDistance(lat1,long1,lat2,long2);
	float angle = RAD2DEG * acos(
			( sin(DEG2RAD*lat2) - sin(DEG2RAD*lat1) * cos(DEG2RAD*zeta) )
			/ ( cos(DEG2RAD*lat1) * sin(DEG2RAD*zeta) )
		);
	if (isnan(angle)) angle=0;
	if (angleDifference(long2-long1)>=0) {
		return angle;
	} else {
		return -angle;
	}
}

/** 
  * @}
  * @}
  */
