package org.openpilot.androidgcs;

import android.os.Bundle;

/**
 * All the work for this activity is performed by it's fragment
 */
public class SystemAlarmActivity extends ObjectManagerActivity {
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.system_alarms);
	}
}
