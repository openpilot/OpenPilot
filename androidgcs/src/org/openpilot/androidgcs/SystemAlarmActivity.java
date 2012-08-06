package org.openpilot.androidgcs;

import java.util.List;

import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectField;

import android.os.Bundle;
import android.widget.TextView;

public class SystemAlarmActivity extends ObjectManagerActivity {
	/**
	 * Update the UI whenever the attitude is updated
	 */
	protected void objectUpdated(UAVObject obj) {
		if (obj.getName().compareTo("SystemAlarms") != 0)
			return;

		TextView alarms = (TextView) findViewById(R.id.system_alarms_status);
		UAVObjectField a = obj.getField("Alarm");
		List<String> names = a.getElementNames();
		String contents = new String();
		List <String> options = a.getOptions();
		
		// Rank the alarms by order of severity, skip uninitialized
		for (int j = options.size() - 1; j > 0; j--) {
			for (int i = 0; i < names.size(); i++) {
				if(a.getDouble(i) == j)
					contents += names.get(i) + " : " + a.getValue(i).toString() + "\n";
			}
		}
		alarms.setText(contents);
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.system_alarms);
	}	

	@Override
	void onOPConnected() {
		super.onOPConnected();
		
		// Connect the update method to AttitudeActual
		UAVObject obj = objMngr.getObject("SystemAlarms");
		if (obj != null)
			registerObjectUpdates(obj);
		objectUpdated(obj);
	}			
}
