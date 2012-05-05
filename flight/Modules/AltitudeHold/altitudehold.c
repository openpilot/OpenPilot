/**
 ******************************************************************************
 *
 * @file       guidance.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      This module compared @ref PositionActuatl to @ref ActiveWaypoint 
 * and sets @ref AttitudeDesired.  It only does this when the FlightMode field
 * of @ref ManualControlCommand is Auto.
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

/**
 * Input object: ActiveWaypoint
 * Input object: PositionActual
 * Input object: ManualControlCommand
 * Output object: AttitudeDesired
 *
 * This module will periodically update the value of the AttitudeDesired object.
 *
 * The module executes in its own thread in this example.
 *
 * Modules have no API, all communication to other modules is done through UAVObjects.
 * However modules may use the API exposed by shared libraries.
 * See the OpenPilot wiki for more details.
 * http://www.openpilot.org/OpenPilot_Application_Architecture
 *
 */

#include "openpilot.h"
#include <math.h>
#include "CoordinateConversions.h"
#include "altholdsmoothed.h"
#include "attitudeactual.h"
#include "altitudeholdsettings.h"
#include "altitudeholddesired.h"	// object that will be updated by the module
#include "baroaltitude.h"
#include "positionactual.h"
#include "flightstatus.h"
#include "stabilizationdesired.h"
#include "accels.h"

// Private constants
#define MAX_QUEUE_SIZE 2
#define STACK_SIZE_BYTES 1024
#define TASK_PRIORITY (tskIDLE_PRIORITY+1)
// Private types

// Private variables
static xTaskHandle altitudeHoldTaskHandle;
static xQueueHandle queue;
static AltitudeHoldSettingsData altitudeHoldSettings;

// Private functions
static void altitudeHoldTask(void *parameters);
static void SettingsUpdatedCb(UAVObjEvent * ev);

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AltitudeHoldStart()
{
	// Start main task
	xTaskCreate(altitudeHoldTask, (signed char *)"AltitudeHold", STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &altitudeHoldTaskHandle);
	TaskMonitorAdd(TASKINFO_RUNNING_ALTITUDEHOLD, altitudeHoldTaskHandle);

	return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AltitudeHoldInitialize()
{
	AltitudeHoldSettingsInitialize();
	AltitudeHoldDesiredInitialize();
	AltHoldSmoothedInitialize();

	// Create object queue
	queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));

	AltitudeHoldSettingsConnectCallback(&SettingsUpdatedCb);

	return 0;
}
MODULE_INITCALL(AltitudeHoldInitialize, AltitudeHoldStart)

float tau;
float throttleIntegral;
float velocity;
float decay;
float velocity_decay;
bool running = false;
float error;
float switchThrottle;
float smoothed_altitude;
float starting_altitude;

/**
 * Module thread, should not return.
 */
static void altitudeHoldTask(void *parameters)
{
	AltitudeHoldDesiredData altitudeHoldDesired;
	StabilizationDesiredData stabilizationDesired;

	portTickType thisTime, lastUpdateTime;
	UAVObjEvent ev;
	
	// Force update of the settings
	SettingsUpdatedCb(&ev);

	// Listen for updates.
	AltitudeHoldDesiredConnectQueue(queue);
	BaroAltitudeConnectQueue(queue);
	FlightStatusConnectQueue(queue);
	AccelsConnectQueue(queue);
	
	BaroAltitudeAltitudeGet(&smoothed_altitude);
	running = false;
	enum init_state {WAITING_BARO, WAITIING_INIT, INITED} init = WAITING_BARO;

	// Main task loop
	bool baro_updated = false;
	while (1) {		
		// Wait until the AttitudeRaw object is updated, if a timeout then go to failsafe
		if ( xQueueReceive(queue, &ev, 100 / portTICK_RATE_MS) != pdTRUE )
		{
			if(!running)
				throttleIntegral = 0;

			// Todo: Add alarm if it should be running
			continue;
		} else if (ev.obj == BaroAltitudeHandle()) {
			baro_updated = true;
			
			init = (init == WAITING_BARO) ? WAITIING_INIT : init;
		} else if (ev.obj == FlightStatusHandle()) {
			FlightStatusData flightStatus;
			FlightStatusGet(&flightStatus);

			if(flightStatus.FlightMode == FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD && !running) {
				// Copy the current throttle as a starting point for integral
				StabilizationDesiredThrottleGet(&throttleIntegral);
				switchThrottle = throttleIntegral;
				error = 0;
				velocity = 0;
				running = true;

				AltHoldSmoothedData altHold;
				AltHoldSmoothedGet(&altHold);
				starting_altitude = altHold.Altitude;
			} else if (flightStatus.FlightMode != FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD)
				running = false;
		} else if (ev.obj == AccelsHandle()) {
			static uint32_t timeval;
			
			static float z[4] = {0, 0, 0, 0};
			float z_new[4];
			float P[4][4], K[4][2], x[2];
			float G[4] = {1.0e-15f, 1.0e-15f, 1.0e-3f, 1.0e-7};
			static float V[4][4] = {{10.0f, 0, 0, 0}, {0, 100.0f, 0, 0}, {0, 0, 100.0f, 0}, {0, 0, 0, 1000.0f}};

			float dT;
			static float S[2] = {1.0f,10.0f};

			thisTime = xTaskGetTickCount();

			/* Somehow this always assigns to zero.  Compiler bug? Race condition? */
			S[0] = altitudeHoldSettings.PressureNoise;
			S[1] = altitudeHoldSettings.AccelNoise;
			G[2] = altitudeHoldSettings.AccelDrift; 

			AccelsData accels;
			AccelsGet(&accels);
			AttitudeActualData attitudeActual;
			AttitudeActualGet(&attitudeActual);
			BaroAltitudeData baro;
			BaroAltitudeGet(&baro);

			if (init == WAITIING_INIT) {
				z[0] = baro.Altitude;
				z[1] = 0;
				z[2] = accels.z;
				z[3] = 0;
				init = INITED;
			} else if (init == WAITING_BARO)
				continue;

			x[0] = baro.Altitude;
			//rotate avg accels into earth frame and store it
			if(1) {
				float q[4], Rbe[3][3];
				q[0]=attitudeActual.q1;
				q[1]=attitudeActual.q2;
				q[2]=attitudeActual.q3;
				q[3]=attitudeActual.q4;
				Quaternion2R(q, Rbe);
				x[1] = -(Rbe[0][2]*accels.x+ Rbe[1][2]*accels.y + Rbe[2][2]*accels.z + 9.81f);
			} else {
				x[1] = -accels.z + 9.81f;
			}

			dT = PIOS_DELAY_DiffuS(timeval) / 1.0e6f;
			timeval = PIOS_DELAY_GetRaw();

			P[0][0] = dT*(V[0][1]+dT*V[1][1])+V[0][0]+G[0]+dT*V[1][0];
			P[0][1] = dT*(V[0][2]+dT*V[1][2])+V[0][1]+dT*V[1][1];
			P[0][2] = V[0][2]+dT*V[1][2];
			P[0][3] = V[0][3]+dT*V[1][3];
			P[1][0] = dT*(V[1][1]+dT*V[2][1])+V[1][0]+dT*V[2][0];
			P[1][1] = dT*(V[1][2]+dT*V[2][2])+V[1][1]+G[1]+dT*V[2][1];
			P[1][2] = V[1][2]+dT*V[2][2];
			P[1][3] = V[1][3]+dT*V[2][3];
			P[2][0] = V[2][0]+dT*V[2][1];
			P[2][1] = V[2][1]+dT*V[2][2];
			P[2][2] = V[2][2]+G[2];
			P[2][3] = V[2][3];
			P[3][0] = V[3][0]+dT*V[3][1];
			P[3][1] = V[3][1]+dT*V[3][2];
			P[3][2] = V[3][2];
			P[3][3] = V[3][3]+G[3];

			if (baro_updated) {
				K[0][0] = -(V[2][2]*S[0]+V[2][3]*S[0]+V[3][2]*S[0]+V[3][3]*S[0]+G[2]*S[0]+G[3]*S[0]+S[0]*S[1])/(V[0][0]*G[2]+V[0][0]*G[3]+V[2][2]*G[0]+V[2][3]*G[0]+V[3][2]*G[0]+V[3][3]*G[0]+V[0][0]*S[1]+V[2][2]*S[0]+V[2][3]*S[0]+V[3][2]*S[0]+V[3][3]*S[0]+V[0][0]*V[2][2]-V[0][2]*V[2][0]+V[0][0]*V[2][3]+V[0][0]*V[3][2]-V[0][2]*V[3][0]-V[2][0]*V[0][3]+V[0][0]*V[3][3]-V[0][3]*V[3][0]+G[0]*G[2]+G[0]*G[3]+G[0]*S[1]+G[2]*S[0]+G[3]*S[0]+S[0]*S[1]+(dT*dT)*V[1][1]*V[2][2]-(dT*dT)*V[1][2]*V[2][1]+(dT*dT)*V[1][1]*V[2][3]+(dT*dT)*V[1][1]*V[3][2]-(dT*dT)*V[1][2]*V[3][1]-(dT*dT)*V[2][1]*V[1][3]+(dT*dT)*V[1][1]*V[3][3]-(dT*dT)*V[1][3]*V[3][1]+dT*V[0][1]*G[2]+dT*V[1][0]*G[2]+dT*V[0][1]*G[3]+dT*V[1][0]*G[3]+dT*V[0][1]*S[1]+dT*V[1][0]*S[1]+(dT*dT)*V[1][1]*G[2]+(dT*dT)*V[1][1]*G[3]+(dT*dT)*V[1][1]*S[1]+dT*V[0][1]*V[2][2]+dT*V[1][0]*V[2][2]-dT*V[0][2]*V[2][1]-dT*V[2][0]*V[1][2]+dT*V[0][1]*V[2][3]+dT*V[0][1]*V[3][2]+dT*V[1][0]*V[2][3]+dT*V[1][0]*V[3][2]-dT*V[0][2]*V[3][1]-dT*V[2][0]*V[1][3]-dT*V[0][3]*V[2][1]-dT*V[1][2]*V[3][0]+dT*V[0][1]*V[3][3]+dT*V[1][0]*V[3][3]-dT*V[0][3]*V[3][1]-dT*V[3][0]*V[1][3])+1.0f;
				K[0][1] = ((V[0][2]+V[0][3])*S[0]+dT*(V[1][2]+V[1][3])*S[0])/(V[0][0]*G[2]+V[0][0]*G[3]+V[2][2]*G[0]+V[2][3]*G[0]+V[3][2]*G[0]+V[3][3]*G[0]+V[0][0]*S[1]+V[2][2]*S[0]+V[2][3]*S[0]+V[3][2]*S[0]+V[3][3]*S[0]+V[0][0]*V[2][2]-V[0][2]*V[2][0]+V[0][0]*V[2][3]+V[0][0]*V[3][2]-V[0][2]*V[3][0]-V[2][0]*V[0][3]+V[0][0]*V[3][3]-V[0][3]*V[3][0]+G[0]*G[2]+G[0]*G[3]+G[0]*S[1]+G[2]*S[0]+G[3]*S[0]+S[0]*S[1]+(dT*dT)*V[1][1]*V[2][2]-(dT*dT)*V[1][2]*V[2][1]+(dT*dT)*V[1][1]*V[2][3]+(dT*dT)*V[1][1]*V[3][2]-(dT*dT)*V[1][2]*V[3][1]-(dT*dT)*V[2][1]*V[1][3]+(dT*dT)*V[1][1]*V[3][3]-(dT*dT)*V[1][3]*V[3][1]+dT*V[0][1]*G[2]+dT*V[1][0]*G[2]+dT*V[0][1]*G[3]+dT*V[1][0]*G[3]+dT*V[0][1]*S[1]+dT*V[1][0]*S[1]+(dT*dT)*V[1][1]*G[2]+(dT*dT)*V[1][1]*G[3]+(dT*dT)*V[1][1]*S[1]+dT*V[0][1]*V[2][2]+dT*V[1][0]*V[2][2]-dT*V[0][2]*V[2][1]-dT*V[2][0]*V[1][2]+dT*V[0][1]*V[2][3]+dT*V[0][1]*V[3][2]+dT*V[1][0]*V[2][3]+dT*V[1][0]*V[3][2]-dT*V[0][2]*V[3][1]-dT*V[2][0]*V[1][3]-dT*V[0][3]*V[2][1]-dT*V[1][2]*V[3][0]+dT*V[0][1]*V[3][3]+dT*V[1][0]*V[3][3]-dT*V[0][3]*V[3][1]-dT*V[3][0]*V[1][3]);
				K[1][0] = (V[1][0]*G[2]+V[1][0]*G[3]+V[1][0]*S[1]+V[1][0]*V[2][2]-V[2][0]*V[1][2]+V[1][0]*V[2][3]+V[1][0]*V[3][2]-V[2][0]*V[1][3]-V[1][2]*V[3][0]+V[1][0]*V[3][3]-V[3][0]*V[1][3]+(dT*dT)*V[2][1]*V[3][2]-(dT*dT)*V[2][2]*V[3][1]+(dT*dT)*V[2][1]*V[3][3]-(dT*dT)*V[3][1]*V[2][3]+dT*V[1][1]*G[2]+dT*V[2][0]*G[2]+dT*V[1][1]*G[3]+dT*V[2][0]*G[3]+dT*V[1][1]*S[1]+dT*V[2][0]*S[1]+(dT*dT)*V[2][1]*G[2]+(dT*dT)*V[2][1]*G[3]+(dT*dT)*V[2][1]*S[1]+dT*V[1][1]*V[2][2]-dT*V[1][2]*V[2][1]+dT*V[1][1]*V[2][3]+dT*V[1][1]*V[3][2]+dT*V[2][0]*V[3][2]-dT*V[1][2]*V[3][1]-dT*V[2][1]*V[1][3]-dT*V[3][0]*V[2][2]+dT*V[1][1]*V[3][3]+dT*V[2][0]*V[3][3]-dT*V[3][0]*V[2][3]-dT*V[1][3]*V[3][1])/(V[0][0]*G[2]+V[0][0]*G[3]+V[2][2]*G[0]+V[2][3]*G[0]+V[3][2]*G[0]+V[3][3]*G[0]+V[0][0]*S[1]+V[2][2]*S[0]+V[2][3]*S[0]+V[3][2]*S[0]+V[3][3]*S[0]+V[0][0]*V[2][2]-V[0][2]*V[2][0]+V[0][0]*V[2][3]+V[0][0]*V[3][2]-V[0][2]*V[3][0]-V[2][0]*V[0][3]+V[0][0]*V[3][3]-V[0][3]*V[3][0]+G[0]*G[2]+G[0]*G[3]+G[0]*S[1]+G[2]*S[0]+G[3]*S[0]+S[0]*S[1]+(dT*dT)*V[1][1]*V[2][2]-(dT*dT)*V[1][2]*V[2][1]+(dT*dT)*V[1][1]*V[2][3]+(dT*dT)*V[1][1]*V[3][2]-(dT*dT)*V[1][2]*V[3][1]-(dT*dT)*V[2][1]*V[1][3]+(dT*dT)*V[1][1]*V[3][3]-(dT*dT)*V[1][3]*V[3][1]+dT*V[0][1]*G[2]+dT*V[1][0]*G[2]+dT*V[0][1]*G[3]+dT*V[1][0]*G[3]+dT*V[0][1]*S[1]+dT*V[1][0]*S[1]+(dT*dT)*V[1][1]*G[2]+(dT*dT)*V[1][1]*G[3]+(dT*dT)*V[1][1]*S[1]+dT*V[0][1]*V[2][2]+dT*V[1][0]*V[2][2]-dT*V[0][2]*V[2][1]-dT*V[2][0]*V[1][2]+dT*V[0][1]*V[2][3]+dT*V[0][1]*V[3][2]+dT*V[1][0]*V[2][3]+dT*V[1][0]*V[3][2]-dT*V[0][2]*V[3][1]-dT*V[2][0]*V[1][3]-dT*V[0][3]*V[2][1]-dT*V[1][2]*V[3][0]+dT*V[0][1]*V[3][3]+dT*V[1][0]*V[3][3]-dT*V[0][3]*V[3][1]-dT*V[3][0]*V[1][3]);
				K[1][1] = (V[1][2]*G[0]+V[1][3]*G[0]+V[1][2]*S[0]+V[1][3]*S[0]+V[0][0]*V[1][2]-V[1][0]*V[0][2]+V[0][0]*V[1][3]-V[1][0]*V[0][3]+(dT*dT)*V[0][1]*V[2][2]+(dT*dT)*V[1][0]*V[2][2]-(dT*dT)*V[0][2]*V[2][1]-(dT*dT)*V[2][0]*V[1][2]+(dT*dT)*V[0][1]*V[2][3]+(dT*dT)*V[1][0]*V[2][3]-(dT*dT)*V[2][0]*V[1][3]-(dT*dT)*V[0][3]*V[2][1]+(dT*dT*dT)*V[1][1]*V[2][2]-(dT*dT*dT)*V[1][2]*V[2][1]+(dT*dT*dT)*V[1][1]*V[2][3]-(dT*dT*dT)*V[2][1]*V[1][3]+dT*V[2][2]*G[0]+dT*V[2][3]*G[0]+dT*V[2][2]*S[0]+dT*V[2][3]*S[0]+dT*V[0][0]*V[2][2]+dT*V[0][1]*V[1][2]-dT*V[0][2]*V[1][1]-dT*V[0][2]*V[2][0]+dT*V[0][0]*V[2][3]+dT*V[0][1]*V[1][3]-dT*V[1][1]*V[0][3]-dT*V[2][0]*V[0][3])/(V[0][0]*G[2]+V[0][0]*G[3]+V[2][2]*G[0]+V[2][3]*G[0]+V[3][2]*G[0]+V[3][3]*G[0]+V[0][0]*S[1]+V[2][2]*S[0]+V[2][3]*S[0]+V[3][2]*S[0]+V[3][3]*S[0]+V[0][0]*V[2][2]-V[0][2]*V[2][0]+V[0][0]*V[2][3]+V[0][0]*V[3][2]-V[0][2]*V[3][0]-V[2][0]*V[0][3]+V[0][0]*V[3][3]-V[0][3]*V[3][0]+G[0]*G[2]+G[0]*G[3]+G[0]*S[1]+G[2]*S[0]+G[3]*S[0]+S[0]*S[1]+(dT*dT)*V[1][1]*V[2][2]-(dT*dT)*V[1][2]*V[2][1]+(dT*dT)*V[1][1]*V[2][3]+(dT*dT)*V[1][1]*V[3][2]-(dT*dT)*V[1][2]*V[3][1]-(dT*dT)*V[2][1]*V[1][3]+(dT*dT)*V[1][1]*V[3][3]-(dT*dT)*V[1][3]*V[3][1]+dT*V[0][1]*G[2]+dT*V[1][0]*G[2]+dT*V[0][1]*G[3]+dT*V[1][0]*G[3]+dT*V[0][1]*S[1]+dT*V[1][0]*S[1]+(dT*dT)*V[1][1]*G[2]+(dT*dT)*V[1][1]*G[3]+(dT*dT)*V[1][1]*S[1]+dT*V[0][1]*V[2][2]+dT*V[1][0]*V[2][2]-dT*V[0][2]*V[2][1]-dT*V[2][0]*V[1][2]+dT*V[0][1]*V[2][3]+dT*V[0][1]*V[3][2]+dT*V[1][0]*V[2][3]+dT*V[1][0]*V[3][2]-dT*V[0][2]*V[3][1]-dT*V[2][0]*V[1][3]-dT*V[0][3]*V[2][1]-dT*V[1][2]*V[3][0]+dT*V[0][1]*V[3][3]+dT*V[1][0]*V[3][3]-dT*V[0][3]*V[3][1]-dT*V[3][0]*V[1][3]);
				K[2][0] = (V[2][0]*G[3]-V[3][0]*G[2]+V[2][0]*S[1]+V[2][0]*V[3][2]-V[3][0]*V[2][2]+V[2][0]*V[3][3]-V[3][0]*V[2][3]+dT*V[2][1]*G[3]-dT*V[3][1]*G[2]+dT*V[2][1]*S[1]+dT*V[2][1]*V[3][2]-dT*V[2][2]*V[3][1]+dT*V[2][1]*V[3][3]-dT*V[3][1]*V[2][3])/(V[0][0]*G[2]+V[0][0]*G[3]+V[2][2]*G[0]+V[2][3]*G[0]+V[3][2]*G[0]+V[3][3]*G[0]+V[0][0]*S[1]+V[2][2]*S[0]+V[2][3]*S[0]+V[3][2]*S[0]+V[3][3]*S[0]+V[0][0]*V[2][2]-V[0][2]*V[2][0]+V[0][0]*V[2][3]+V[0][0]*V[3][2]-V[0][2]*V[3][0]-V[2][0]*V[0][3]+V[0][0]*V[3][3]-V[0][3]*V[3][0]+G[0]*G[2]+G[0]*G[3]+G[0]*S[1]+G[2]*S[0]+G[3]*S[0]+S[0]*S[1]+(dT*dT)*V[1][1]*V[2][2]-(dT*dT)*V[1][2]*V[2][1]+(dT*dT)*V[1][1]*V[2][3]+(dT*dT)*V[1][1]*V[3][2]-(dT*dT)*V[1][2]*V[3][1]-(dT*dT)*V[2][1]*V[1][3]+(dT*dT)*V[1][1]*V[3][3]-(dT*dT)*V[1][3]*V[3][1]+dT*V[0][1]*G[2]+dT*V[1][0]*G[2]+dT*V[0][1]*G[3]+dT*V[1][0]*G[3]+dT*V[0][1]*S[1]+dT*V[1][0]*S[1]+(dT*dT)*V[1][1]*G[2]+(dT*dT)*V[1][1]*G[3]+(dT*dT)*V[1][1]*S[1]+dT*V[0][1]*V[2][2]+dT*V[1][0]*V[2][2]-dT*V[0][2]*V[2][1]-dT*V[2][0]*V[1][2]+dT*V[0][1]*V[2][3]+dT*V[0][1]*V[3][2]+dT*V[1][0]*V[2][3]+dT*V[1][0]*V[3][2]-dT*V[0][2]*V[3][1]-dT*V[2][0]*V[1][3]-dT*V[0][3]*V[2][1]-dT*V[1][2]*V[3][0]+dT*V[0][1]*V[3][3]+dT*V[1][0]*V[3][3]-dT*V[0][3]*V[3][1]-dT*V[3][0]*V[1][3]);
				K[2][1] = (V[0][0]*G[2]+V[2][2]*G[0]+V[2][3]*G[0]+V[2][2]*S[0]+V[2][3]*S[0]+V[0][0]*V[2][2]-V[0][2]*V[2][0]+V[0][0]*V[2][3]-V[2][0]*V[0][3]+G[0]*G[2]+G[2]*S[0]+(dT*dT)*V[1][1]*V[2][2]-(dT*dT)*V[1][2]*V[2][1]+(dT*dT)*V[1][1]*V[2][3]-(dT*dT)*V[2][1]*V[1][3]+dT*V[0][1]*G[2]+dT*V[1][0]*G[2]+(dT*dT)*V[1][1]*G[2]+dT*V[0][1]*V[2][2]+dT*V[1][0]*V[2][2]-dT*V[0][2]*V[2][1]-dT*V[2][0]*V[1][2]+dT*V[0][1]*V[2][3]+dT*V[1][0]*V[2][3]-dT*V[2][0]*V[1][3]-dT*V[0][3]*V[2][1])/(V[0][0]*G[2]+V[0][0]*G[3]+V[2][2]*G[0]+V[2][3]*G[0]+V[3][2]*G[0]+V[3][3]*G[0]+V[0][0]*S[1]+V[2][2]*S[0]+V[2][3]*S[0]+V[3][2]*S[0]+V[3][3]*S[0]+V[0][0]*V[2][2]-V[0][2]*V[2][0]+V[0][0]*V[2][3]+V[0][0]*V[3][2]-V[0][2]*V[3][0]-V[2][0]*V[0][3]+V[0][0]*V[3][3]-V[0][3]*V[3][0]+G[0]*G[2]+G[0]*G[3]+G[0]*S[1]+G[2]*S[0]+G[3]*S[0]+S[0]*S[1]+(dT*dT)*V[1][1]*V[2][2]-(dT*dT)*V[1][2]*V[2][1]+(dT*dT)*V[1][1]*V[2][3]+(dT*dT)*V[1][1]*V[3][2]-(dT*dT)*V[1][2]*V[3][1]-(dT*dT)*V[2][1]*V[1][3]+(dT*dT)*V[1][1]*V[3][3]-(dT*dT)*V[1][3]*V[3][1]+dT*V[0][1]*G[2]+dT*V[1][0]*G[2]+dT*V[0][1]*G[3]+dT*V[1][0]*G[3]+dT*V[0][1]*S[1]+dT*V[1][0]*S[1]+(dT*dT)*V[1][1]*G[2]+(dT*dT)*V[1][1]*G[3]+(dT*dT)*V[1][1]*S[1]+dT*V[0][1]*V[2][2]+dT*V[1][0]*V[2][2]-dT*V[0][2]*V[2][1]-dT*V[2][0]*V[1][2]+dT*V[0][1]*V[2][3]+dT*V[0][1]*V[3][2]+dT*V[1][0]*V[2][3]+dT*V[1][0]*V[3][2]-dT*V[0][2]*V[3][1]-dT*V[2][0]*V[1][3]-dT*V[0][3]*V[2][1]-dT*V[1][2]*V[3][0]+dT*V[0][1]*V[3][3]+dT*V[1][0]*V[3][3]-dT*V[0][3]*V[3][1]-dT*V[3][0]*V[1][3]);
				K[3][0] = (-V[2][0]*G[3]+V[3][0]*G[2]+V[3][0]*S[1]-V[2][0]*V[3][2]+V[3][0]*V[2][2]-V[2][0]*V[3][3]+V[3][0]*V[2][3]-dT*V[2][1]*G[3]+dT*V[3][1]*G[2]+dT*V[3][1]*S[1]-dT*V[2][1]*V[3][2]+dT*V[2][2]*V[3][1]-dT*V[2][1]*V[3][3]+dT*V[3][1]*V[2][3])/(V[0][0]*G[2]+V[0][0]*G[3]+V[2][2]*G[0]+V[2][3]*G[0]+V[3][2]*G[0]+V[3][3]*G[0]+V[0][0]*S[1]+V[2][2]*S[0]+V[2][3]*S[0]+V[3][2]*S[0]+V[3][3]*S[0]+V[0][0]*V[2][2]-V[0][2]*V[2][0]+V[0][0]*V[2][3]+V[0][0]*V[3][2]-V[0][2]*V[3][0]-V[2][0]*V[0][3]+V[0][0]*V[3][3]-V[0][3]*V[3][0]+G[0]*G[2]+G[0]*G[3]+G[0]*S[1]+G[2]*S[0]+G[3]*S[0]+S[0]*S[1]+(dT*dT)*V[1][1]*V[2][2]-(dT*dT)*V[1][2]*V[2][1]+(dT*dT)*V[1][1]*V[2][3]+(dT*dT)*V[1][1]*V[3][2]-(dT*dT)*V[1][2]*V[3][1]-(dT*dT)*V[2][1]*V[1][3]+(dT*dT)*V[1][1]*V[3][3]-(dT*dT)*V[1][3]*V[3][1]+dT*V[0][1]*G[2]+dT*V[1][0]*G[2]+dT*V[0][1]*G[3]+dT*V[1][0]*G[3]+dT*V[0][1]*S[1]+dT*V[1][0]*S[1]+(dT*dT)*V[1][1]*G[2]+(dT*dT)*V[1][1]*G[3]+(dT*dT)*V[1][1]*S[1]+dT*V[0][1]*V[2][2]+dT*V[1][0]*V[2][2]-dT*V[0][2]*V[2][1]-dT*V[2][0]*V[1][2]+dT*V[0][1]*V[2][3]+dT*V[0][1]*V[3][2]+dT*V[1][0]*V[2][3]+dT*V[1][0]*V[3][2]-dT*V[0][2]*V[3][1]-dT*V[2][0]*V[1][3]-dT*V[0][3]*V[2][1]-dT*V[1][2]*V[3][0]+dT*V[0][1]*V[3][3]+dT*V[1][0]*V[3][3]-dT*V[0][3]*V[3][1]-dT*V[3][0]*V[1][3]);
				K[3][1] = (V[0][0]*G[3]+V[3][2]*G[0]+V[3][3]*G[0]+V[3][2]*S[0]+V[3][3]*S[0]+V[0][0]*V[3][2]-V[0][2]*V[3][0]+V[0][0]*V[3][3]-V[0][3]*V[3][0]+G[0]*G[3]+G[3]*S[0]+(dT*dT)*V[1][1]*V[3][2]-(dT*dT)*V[1][2]*V[3][1]+(dT*dT)*V[1][1]*V[3][3]-(dT*dT)*V[1][3]*V[3][1]+dT*V[0][1]*G[3]+dT*V[1][0]*G[3]+(dT*dT)*V[1][1]*G[3]+dT*V[0][1]*V[3][2]+dT*V[1][0]*V[3][2]-dT*V[0][2]*V[3][1]-dT*V[1][2]*V[3][0]+dT*V[0][1]*V[3][3]+dT*V[1][0]*V[3][3]-dT*V[0][3]*V[3][1]-dT*V[3][0]*V[1][3])/(V[0][0]*G[2]+V[0][0]*G[3]+V[2][2]*G[0]+V[2][3]*G[0]+V[3][2]*G[0]+V[3][3]*G[0]+V[0][0]*S[1]+V[2][2]*S[0]+V[2][3]*S[0]+V[3][2]*S[0]+V[3][3]*S[0]+V[0][0]*V[2][2]-V[0][2]*V[2][0]+V[0][0]*V[2][3]+V[0][0]*V[3][2]-V[0][2]*V[3][0]-V[2][0]*V[0][3]+V[0][0]*V[3][3]-V[0][3]*V[3][0]+G[0]*G[2]+G[0]*G[3]+G[0]*S[1]+G[2]*S[0]+G[3]*S[0]+S[0]*S[1]+(dT*dT)*V[1][1]*V[2][2]-(dT*dT)*V[1][2]*V[2][1]+(dT*dT)*V[1][1]*V[2][3]+(dT*dT)*V[1][1]*V[3][2]-(dT*dT)*V[1][2]*V[3][1]-(dT*dT)*V[2][1]*V[1][3]+(dT*dT)*V[1][1]*V[3][3]-(dT*dT)*V[1][3]*V[3][1]+dT*V[0][1]*G[2]+dT*V[1][0]*G[2]+dT*V[0][1]*G[3]+dT*V[1][0]*G[3]+dT*V[0][1]*S[1]+dT*V[1][0]*S[1]+(dT*dT)*V[1][1]*G[2]+(dT*dT)*V[1][1]*G[3]+(dT*dT)*V[1][1]*S[1]+dT*V[0][1]*V[2][2]+dT*V[1][0]*V[2][2]-dT*V[0][2]*V[2][1]-dT*V[2][0]*V[1][2]+dT*V[0][1]*V[2][3]+dT*V[0][1]*V[3][2]+dT*V[1][0]*V[2][3]+dT*V[1][0]*V[3][2]-dT*V[0][2]*V[3][1]-dT*V[2][0]*V[1][3]-dT*V[0][3]*V[2][1]-dT*V[1][2]*V[3][0]+dT*V[0][1]*V[3][3]+dT*V[1][0]*V[3][3]-dT*V[0][3]*V[3][1]-dT*V[3][0]*V[1][3]);
				
				z_new[0] = -K[0][0]*(dT*z[1]-x[0]+z[0])+dT*z[1]-K[0][1]*(-x[1]+z[2]+z[3])+z[0];
				z_new[1] = -K[1][0]*(dT*z[1]-x[0]+z[0])+dT*z[2]-K[1][1]*(-x[1]+z[2]+z[3])+z[1];
				z_new[2] = -K[2][0]*(dT*z[1]-x[0]+z[0])-K[2][1]*(-x[1]+z[2]+z[3])+z[2];
				z_new[3] = -K[3][0]*(dT*z[1]-x[0]+z[0])-K[3][1]*(-x[1]+z[2]+z[3])+z[3];

				memcpy(z, z_new, sizeof(z_new));
				
				V[0][0] = -K[0][1]*P[2][0]-K[0][1]*P[3][0]-P[0][0]*(K[0][0]-1.0f);
				V[0][1] = -K[0][1]*P[2][1]-K[0][1]*P[3][2]-P[0][1]*(K[0][0]-1.0f);
				V[0][2] = -K[0][1]*P[2][2]-K[0][1]*P[3][2]-P[0][2]*(K[0][0]-1.0f);
				V[0][3] = -K[0][1]*P[2][3]-K[0][1]*P[3][3]-P[0][3]*(K[0][0]-1.0f);
				V[1][0] = P[1][0]-K[1][0]*P[0][0]-K[1][1]*P[2][0]-K[1][1]*P[3][0];
				V[1][1] = P[1][1]-K[1][0]*P[0][1]-K[1][1]*P[2][1]-K[1][1]*P[3][2];
				V[1][2] = P[1][2]-K[1][0]*P[0][2]-K[1][1]*P[2][2]-K[1][1]*P[3][2];
				V[1][3] = P[1][3]-K[1][0]*P[0][3]-K[1][1]*P[2][3]-K[1][1]*P[3][3];
				V[2][0] = -K[2][0]*P[0][0]-K[2][1]*P[3][0]-P[2][0]*(K[2][1]-1.0f);
				V[2][1] = -K[2][0]*P[0][1]-K[2][1]*P[3][2]-P[2][1]*(K[2][1]-1.0f);
				V[2][2] = -K[2][0]*P[0][2]-K[2][1]*P[3][2]-P[2][2]*(K[2][1]-1.0f);
				V[2][3] = -K[2][0]*P[0][3]-K[2][1]*P[3][3]-P[2][3]*(K[2][1]-1.0f);
				V[3][0] = -K[3][0]*P[0][0]-K[3][1]*P[2][0]-P[3][0]*(K[3][1]-1.0f);
				V[3][1] = -K[3][0]*P[0][1]-K[3][1]*P[2][1]-P[3][2]*(K[3][1]-1.0f);
				V[3][2] = -K[3][0]*P[0][2]-K[3][1]*P[2][2]-P[3][2]*(K[3][1]-1.0f);
				V[3][3] = -K[3][0]*P[0][3]-K[3][1]*P[2][3]-P[3][3]*(K[3][1]-1.0f);
				

				baro_updated = false;
			} else {
				K[0][0] = (V[0][2]+V[0][3]+dT*V[1][2]+dT*V[1][3])/(V[2][2]+V[2][3]+V[3][2]+V[3][3]+G[2]+G[3]+S[1]);
				K[1][0] = (V[1][2]+V[1][3]+dT*V[2][2]+dT*V[2][3])/(V[2][2]+V[2][3]+V[3][2]+V[3][3]+G[2]+G[3]+S[1]);
				K[2][0] = (V[2][2]+V[2][3]+G[2])/(V[2][2]+V[2][3]+V[3][2]+V[3][3]+G[2]+G[3]+S[1]);
				K[3][0] = (V[3][2]+V[3][3]+G[3])/(V[2][2]+V[2][3]+V[3][2]+V[3][3]+G[2]+G[3]+S[1]);
				

				z_new[0] = dT*z[1]-K[0][0]*(-x[1]+z[2]+z[3])+z[0];
				z_new[1] = dT*z[2]-K[1][0]*(-x[1]+z[2]+z[3])+z[1];
				z_new[2] = -K[2][0]*(-x[1]+z[2]+z[3])+z[2];
				z_new[3] = -K[3][0]*(-x[1]+z[2]+z[3])+z[3];

				memcpy(z, z_new, sizeof(z_new));

				V[0][0] = P[0][0]-K[0][0]*P[2][0]-K[0][0]*P[3][0];
				V[0][1] = P[0][1]-K[0][0]*P[2][1]-K[0][0]*P[3][2];
				V[0][2] = P[0][2]-K[0][0]*P[2][2]-K[0][0]*P[3][2];
				V[0][3] = P[0][3]-K[0][0]*P[2][3]-K[0][0]*P[3][3];
				V[1][0] = P[1][0]-K[1][0]*P[2][0]-K[1][0]*P[3][0];
				V[1][1] = P[1][1]-K[1][0]*P[2][1]-K[1][0]*P[3][2];
				V[1][2] = P[1][2]-K[1][0]*P[2][2]-K[1][0]*P[3][2];
				V[1][3] = P[1][3]-K[1][0]*P[2][3]-K[1][0]*P[3][3];
				V[2][0] = -K[2][0]*P[3][0]-P[2][0]*(K[2][0]-1.0f);
				V[2][1] = -K[2][0]*P[3][2]-P[2][1]*(K[2][0]-1.0f);
				V[2][2] = -K[2][0]*P[3][2]-P[2][2]*(K[2][0]-1.0f);
				V[2][3] = -K[2][0]*P[3][3]-P[2][3]*(K[2][0]-1.0f);
				V[3][0] = -K[3][0]*P[2][0]-P[3][0]*(K[3][0]-1.0f);
				V[3][1] = -K[3][0]*P[2][1]-P[3][2]*(K[3][0]-1.0f);
				V[3][2] = -K[3][0]*P[2][2]-P[3][2]*(K[3][0]-1.0f);
				V[3][3] = -K[3][0]*P[2][3]-P[3][3]*(K[3][0]-1.0f);
			}

			AltHoldSmoothedData altHold;
			AltHoldSmoothedGet(&altHold);
			altHold.Altitude = z[0];
			altHold.Velocity = z[1];
			altHold.Accel = z[2];
			AltHoldSmoothedSet(&altHold);

			AltHoldSmoothedGet(&altHold);

			// Verify that we are  in altitude hold mode
			FlightStatusData flightStatus;
			FlightStatusGet(&flightStatus);
			if(flightStatus.FlightMode != FLIGHTSTATUS_FLIGHTMODE_ALTITUDEHOLD) {
				running = false;
			}

			if (!running)
				continue;

			// Compute the altitude error
			error = (starting_altitude + altitudeHoldDesired.Altitude) - altHold.Altitude;

			// Compute integral off altitude error
			throttleIntegral += error * altitudeHoldSettings.Ki * dT;

			// Only update stabilizationDesired less frequently
			if((thisTime - lastUpdateTime) < 20)
				continue;

			lastUpdateTime = thisTime;

			// Instead of explicit limit on integral you output limit feedback
			StabilizationDesiredGet(&stabilizationDesired);
			stabilizationDesired.Throttle = error * altitudeHoldSettings.Kp + throttleIntegral -
			altHold.Velocity * altitudeHoldSettings.Kd - altHold.Accel * altitudeHoldSettings.Ka;
			if(stabilizationDesired.Throttle > 1) {
				throttleIntegral -= (stabilizationDesired.Throttle - 1);
				stabilizationDesired.Throttle = 1;
			}
			else if (stabilizationDesired.Throttle < 0) {
				throttleIntegral -= stabilizationDesired.Throttle;
				stabilizationDesired.Throttle = 0;
			}

			stabilizationDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_ROLL] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
			stabilizationDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_PITCH] = STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE;
			stabilizationDesired.StabilizationMode[STABILIZATIONDESIRED_STABILIZATIONMODE_YAW] = STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK;		
			stabilizationDesired.Roll = altitudeHoldDesired.Roll;
			stabilizationDesired.Pitch = altitudeHoldDesired.Pitch;
			stabilizationDesired.Yaw = altitudeHoldDesired.Yaw;
			StabilizationDesiredSet(&stabilizationDesired);

		} else if (ev.obj == AltitudeHoldDesiredHandle()) {
			AltitudeHoldDesiredGet(&altitudeHoldDesired);
		}

	}
}

static void SettingsUpdatedCb(UAVObjEvent * ev)
{
	AltitudeHoldSettingsGet(&altitudeHoldSettings);
}
