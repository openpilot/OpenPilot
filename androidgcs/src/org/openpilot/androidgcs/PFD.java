package org.openpilot.androidgcs;

import java.util.Observable;
import java.util.Observer;

import org.openpilot.uavtalk.UAVDataObject;
import org.openpilot.uavtalk.UAVObject;

import android.os.Bundle;

public class PFD extends ObjectManagerActivity {

	double heading;
	double roll;
	double pitch;

	Runnable update = new Runnable() {
		public void run() {
			CompassView compass = (CompassView) findViewById(R.id.compass_view);
			compass.setBearing((int) heading);
			compass.invalidate();

			AttitudeView attitude = (AttitudeView) findViewById(R.id.attitude_view);
			attitude.setRoll(roll / 180 * Math.PI);
			attitude.setPitch(pitch / 180 * Math.PI);
			attitude.invalidate();
		}		
	};

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.pfd);
	}	

	@Override
	void onOPConnected() {

		UAVObject obj = objMngr.getObject("AttitudeActual");
		if(obj != null)
			obj.addUpdatedObserver(new Observer() {				
				public void update(Observable observable, Object data) {
					UAVDataObject obj = (UAVDataObject) data;
					heading = obj.getField("Yaw").getDouble();
					pitch = obj.getField("Pitch").getDouble();
					roll = obj.getField("Roll").getDouble();
					runOnUiThread(update);
				}
			});
	}				
}
