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
