/**
 ******************************************************************************
 * @file       PFD.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Shows the PFD activity.
 * @see        The GNU Public License (GPL) Version 3
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

package org.openpilot.androidgcs;

import org.openpilot.uavtalk.UAVObject;

import android.os.Bundle;

public class PFD extends ObjectManagerActivity {

	final long MIN_UPDATE_PERIOD = 50;
	long lastUpdateMs;
	double heading;
	double roll;
	double pitch;

	/**
	 * Update the UI whenever the attitude is updated
	 */
	@Override
	protected void objectUpdated(UAVObject obj) {
		// Throttle the UI redraws.  Eventually this should maybe come from a periodic task
		if ((System.currentTimeMillis() - lastUpdateMs) < MIN_UPDATE_PERIOD)
			return;
		if (obj.getName().compareTo("AttitudeActual") != 0)
			return;

		lastUpdateMs = System.currentTimeMillis();

		heading = obj.getField("Yaw").getDouble();
		pitch = obj.getField("Pitch").getDouble();
		roll = obj.getField("Roll").getDouble();

		CompassView compass = (CompassView) findViewById(R.id.compass_view);
		compass.setBearing((int) heading);
		compass.invalidate();

		AttitudeView attitude = (AttitudeView) findViewById(R.id.attitude_view);
		attitude.setRoll(roll / 180 * Math.PI);
		attitude.setPitch(pitch / 180 * Math.PI);
		attitude.invalidate();
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.pfd);
	}

	@Override
	void onOPConnected() {
		super.onOPConnected();

		// Connect the update method to AttitudeActual
		UAVObject obj = objMngr.getObject("AttitudeActual");
		if (obj != null)
			registerObjectUpdates(obj);
	}
}
