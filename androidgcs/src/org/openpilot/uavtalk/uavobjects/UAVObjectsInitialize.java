/**
 ******************************************************************************
 *
 *
 * @file       uavobjectsinittemplate.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      the template for the uavobjects init part
 *             $(GENERATEDWARNING)
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

package org.openpilot.uavtalk.uavobjects;

import org.openpilot.uavtalk.uavobjects.*;
import org.openpilot.uavtalk.UAVObjectManager;

public class UAVObjectsInitialize {
	
	public static void register(UAVObjectManager objMngr) {
		try {
			objMngr.registerObject( new Accels() );
			objMngr.registerObject( new AccessoryDesired() );
			objMngr.registerObject( new ActuatorCommand() );
			objMngr.registerObject( new ActuatorDesired() );
			objMngr.registerObject( new ActuatorSettings() );
			objMngr.registerObject( new AltHoldSmoothed() );
			objMngr.registerObject( new AltitudeHoldDesired() );
			objMngr.registerObject( new AltitudeHoldSettings() );
			objMngr.registerObject( new AttitudeActual() );
			objMngr.registerObject( new AttitudeSettings() );
			objMngr.registerObject( new BaroAltitude() );
			objMngr.registerObject( new CameraDesired() );
			objMngr.registerObject( new CameraStabSettings() );
			objMngr.registerObject( new FaultSettings() );
			objMngr.registerObject( new FirmwareIAPObj() );
			objMngr.registerObject( new FlightBatterySettings() );
			objMngr.registerObject( new FlightBatteryState() );
			objMngr.registerObject( new FlightPlanControl() );
			objMngr.registerObject( new FlightPlanSettings() );
			objMngr.registerObject( new FlightPlanStatus() );
			objMngr.registerObject( new FlightStatus() );
			objMngr.registerObject( new FlightTelemetryStats() );
			objMngr.registerObject( new GCSReceiver() );
			objMngr.registerObject( new GCSTelemetryStats() );
			objMngr.registerObject( new GPSPosition() );
			objMngr.registerObject( new GPSSatellites() );
			objMngr.registerObject( new GPSTime() );
			objMngr.registerObject( new GPSVelocity() );
			objMngr.registerObject( new Gyros() );
			objMngr.registerObject( new GyrosBias() );
			objMngr.registerObject( new GuidanceSettings() );
			objMngr.registerObject( new HomeLocation() );
			objMngr.registerObject( new HwSettings() );
			objMngr.registerObject( new I2CStats() );
			objMngr.registerObject( new GPSPosition() );
			objMngr.registerObject( new Magnetometer() );
			objMngr.registerObject( new ManualControlCommand() );
			objMngr.registerObject( new ManualControlSettings() );
			objMngr.registerObject( new MixerSettings() );
			objMngr.registerObject( new MixerStatus() );
			objMngr.registerObject( new NEDPosition() );
			objMngr.registerObject( new NedAccel() );
			objMngr.registerObject( new ObjectPersistence() );
			objMngr.registerObject( new PathDesired() );
			objMngr.registerObject( new PositionActual() );
			objMngr.registerObject( new PositionDesired() );
			objMngr.registerObject( new RateDesired() );
			objMngr.registerObject( new ReceiverActivity() );
			objMngr.registerObject( new SonarAltitude() );
			objMngr.registerObject( new StabilizationDesired() );
			objMngr.registerObject( new StabilizationSettings() );
			objMngr.registerObject( new SystemAlarms() );
			objMngr.registerObject( new SystemSettings() );
			objMngr.registerObject( new SystemStats() );
			objMngr.registerObject( new TxPIDSettings() );
			objMngr.registerObject( new TaskInfo() );
			objMngr.registerObject( new VelocityActual() );
			objMngr.registerObject( new VelocityDesired() );
			objMngr.registerObject( new WatchdogStatus() );
			objMngr.registerObject( new Waypoint() );
			objMngr.registerObject( new WaypointActive() );
	
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}