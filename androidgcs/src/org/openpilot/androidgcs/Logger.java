/**
 ******************************************************************************
 * @file       Logger.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Controller for logging data as well as interface for getting that
 *             data on and off the tablet.
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

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVTalk;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.TextView;


public class Logger extends ObjectManagerActivity {

	final String TAG = "Logger";

	final boolean VERBOSE = false;
	final boolean DEBUG = true;

	private File file;
	private boolean logging;
	private FileOutputStream fileStream;
	private UAVTalk uavTalk;

	private int writtenBytes;
	private int writtenObjects;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.logger);
	}

	private void onStartLogging() {

		File root = Environment.getExternalStorageDirectory();

		Date d = new Date();
		String date = (new SimpleDateFormat("yyyyMMdd_hhmmss")).format(d);
		String fileName = "/logs/logs_" + date + ".opl";

		file = new File(root, fileName);
		if (DEBUG) Log.d(TAG, "Trying for file: " + file.getAbsolutePath());
		try {
			if (root.canWrite()){
				fileStream = new FileOutputStream(file);
				uavTalk = new UAVTalk(null, fileStream, objMngr);
				logging = true;
				writtenBytes = 0;
				writtenObjects = 0;
			} else {
				Log.e(TAG, "Unwriteable address");
			}
		} catch (IOException e) {
			Log.e(TAG, "Could not write file " + e.getMessage());
		}

		// TODO: if logging succeeded then retrieve all settings
	}

	private void onStopLogging() {
		if (DEBUG) Log.d(TAG, "Stop logging");
		logging = false;
		try {
			fileStream.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	@Override
	void onOPConnected() {
		super.onOPConnected();

		if (DEBUG) Log.d(TAG, "onOPConnected()");
		onStartLogging();
		registerObjectUpdates(objMngr.getObjects());
	}

	@Override
	void onOPDisconnected() {
		if (DEBUG) Log.d(TAG, "onOPDisconnected()");
		onStopLogging();
	}

	@Override
	public void onPause()
	{
	    super.onPause();
	    onStopLogging();
	}

	@Override
	public void onResume()
	{
	    super.onResume();
	    onStartLogging();
	}
	/**
	 * Called whenever any objects subscribed to via registerObjects
	 */
	@Override
	protected void objectUpdated(UAVObject obj) {
		if (logging) {
			if (VERBOSE) Log.v(TAG,"Updated: " + obj.toString());
			try {
				long time = System.currentTimeMillis();
				fileStream.write((byte)(time & 0xff));
				fileStream.write((byte)((time & 0x0000ff00) >> 8));
				fileStream.write((byte)((time & 0x00ff0000) >> 16));
				fileStream.write((byte)((time & 0xff000000) >> 24));

				long size = obj.getNumBytes();
				fileStream.write((byte)(size & 0x00000000000000ffl) >> 0);
				fileStream.write((byte)(size & 0x000000000000ff00l) >> 8);
				fileStream.write((byte)(size & 0x0000000000ff0000l) >> 16);
				fileStream.write((byte)(size & 0x00000000ff000000l) >> 24);
				fileStream.write((byte)(size & 0x000000ff00000000l) >> 32);
				fileStream.write((byte)(size & 0x0000ff0000000000l) >> 40);
				fileStream.write((byte)(size & 0x00ff000000000000l) >> 48);
				fileStream.write((byte)(size & 0xff00000000000000l) >> 56);

				uavTalk.sendObject(obj, false, false);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			writtenBytes += obj.getNumBytes();
			writtenObjects ++;

			((TextView) findViewById(R.id.logger_number_of_bytes)).setText(Integer.valueOf(writtenBytes).toString());
			((TextView) findViewById(R.id.logger_number_of_objects)).setText(Integer.valueOf(writtenObjects).toString());
		}
	}
}
