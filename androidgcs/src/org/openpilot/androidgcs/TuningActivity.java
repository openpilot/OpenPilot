package org.openpilot.androidgcs;

import org.openpilot.uavtalk.UAVDataObject;
import org.openpilot.uavtalk.UAVObject;

import android.os.Bundle;
import android.util.Log;

public class TuningActivity extends ObjectManagerActivity {
	private final String TAG = TuningActivity.class.getSimpleName();

	private final boolean DEBUG = false;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.tuning);
	}

	@Override
	void onOPConnected() {
		super.onOPConnected();

		if (DEBUG) Log.d(TAG, "onOPConnected()");

		// Subscribe to updates from ManualControlCommand and show the values for crude feedback
		UAVDataObject stabilizationSettings = (UAVDataObject) objMngr.getObject("StabilizationSettings");
		if (stabilizationSettings != null)
			registerObjectUpdates(stabilizationSettings);
	}

	/**
	 * Show the string description of manual control command
	 */
	@Override
	protected void objectUpdated(UAVObject obj) {
		if (obj.getName().compareTo("StabilizationSettings") == 0) {

		}
	}

}
