/**
 ******************************************************************************
 * @file       HomePage.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Main launch page for the Android GCS actitivies
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
package org.openpilot.androidgcs;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class HomePage extends ObjectManagerActivity {
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.gcs_home);

		Button objectBrowser = (Button) findViewById(R.id.launch_object_browser);
		objectBrowser.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, ObjectBrowser.class));
			}
		});

		Button pfd = (Button) findViewById(R.id.launch_pfd);
		pfd.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, PfdActivity.class));
			}
		});

		Button location = (Button) findViewById(R.id.launch_location);
		location.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, UAVLocation.class));
			}
		});

		Button controller = (Button) findViewById(R.id.launch_controller);
		controller.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, Controller.class));
			}
		});

		Button logger = (Button) findViewById(R.id.launch_logger);
		logger.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, Logger.class));
			}
		});

		Button alarms = (Button) findViewById(R.id.launch_alarms);
		alarms.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, SystemAlarmActivity.class));
			}
		});

		Button tester = (Button) findViewById(R.id.launch_tester);
		tester.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, FragmentTester.class));
			}
		});

		Button osgViewer = (Button) findViewById(R.id.launch_osgViewer);
		osgViewer.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, OsgViewer.class));
			}
		});

		Button tuning = (Button) findViewById(R.id.launch_tuning);
		tuning.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, TuningActivity.class));
			}
		});

	}

}
