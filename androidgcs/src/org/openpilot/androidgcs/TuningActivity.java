package org.openpilot.androidgcs;

import org.openpilot.androidgcs.util.SmartSave;
import org.openpilot.androidgcs.views.ScrollBarView;
import org.openpilot.uavtalk.UAVDataObject;

import android.os.Bundle;
import android.util.Log;
import android.widget.Button;

public class TuningActivity extends ObjectManagerActivity {
	private final String TAG = TuningActivity.class.getSimpleName();

	private final boolean DEBUG = false;

	private SmartSave smartSave;

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

		smartSave = new SmartSave(objMngr, stabilizationSettings,
				(Button) findViewById(R.id.saveBtn),
				(Button) findViewById(R.id.applyBtn));

		smartSave.addControlMapping((ScrollBarView) findViewById(R.id.rollRateKp), "RollRatePID", 0);
		smartSave.addControlMapping((ScrollBarView) findViewById(R.id.rollRateKi), "RollRatePID", 1);
		smartSave.addControlMapping((ScrollBarView) findViewById(R.id.pitchRateKp), "PitchRatePID", 0);
		smartSave.addControlMapping((ScrollBarView) findViewById(R.id.pitchRateKi), "PitchRatePID", 1);
		smartSave.addControlMapping((ScrollBarView) findViewById(R.id.rollKp), "RollPI", 0);
		smartSave.addControlMapping((ScrollBarView) findViewById(R.id.pitchKp), "PitchPI", 0);
		smartSave.refreshSettingsDisplay();
	}

}
