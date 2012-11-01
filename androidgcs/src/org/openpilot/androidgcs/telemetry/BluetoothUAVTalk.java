/**
 ******************************************************************************
 * @file       BluetoothUAVTalk.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Telemetry over bluetooth.
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

package org.openpilot.androidgcs.telemetry;

import java.io.IOException;
import java.util.Set;
import java.util.UUID;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

public class BluetoothUAVTalk extends TelemetryTask {

	private final String TAG = "BluetoothUAVTalk";
	public static final int LOGLEVEL = 4;
	public static final boolean DEBUG = LOGLEVEL > 2;
	public static final boolean WARN = LOGLEVEL > 1;
	public static final boolean ERROR = LOGLEVEL > 0;

	// Temporarily define fixed device name
	private String device_name = "RN42-222D";
	private final static UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

	private BluetoothAdapter mBluetoothAdapter;
	private BluetoothSocket socket;
	private BluetoothDevice device;

	public BluetoothUAVTalk(OPTelemetryService caller) {
		super(caller);
	}

	@Override
	boolean attemptConnection() {

		if( getConnected() )
			return true;

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(telemService);
		device_name = prefs.getString("bluetooth_mac","");

        if (DEBUG) Log.d(TAG, "Trying to open UAVTalk with " + device_name);

        device = null;

        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter == null) {
            // Device does not support Bluetooth
        	Log.e(TAG, "Device does not support Bluetooth");
        	return false;
        }

        if (!mBluetoothAdapter.isEnabled()) {
        	// Enable bluetooth if it isn't already
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            telemService.sendOrderedBroadcast(enableBtIntent, "android.permission.BLUETOOTH_ADMIN", new BroadcastReceiver() {
				@Override
				public void onReceive(Context context, Intent intent) {
					Log.e(TAG,"Received " + context + intent);
					//TODO: some logic here to see if it worked
					queryDevices();
				}
            }, null, Activity.RESULT_OK, null, null);
        } else {
        	queryDevices();
        }

		return true;
	}

	@Override
	public void disconnect() {
		super.disconnect();
		if(socket != null) {
			try {
				socket.close();
			} catch (IOException e) {
				if (ERROR) Log.e(TAG, "Unable to close BT socket");
			}
			socket = null;
		}
	}

	public boolean getFoundDevice() {
		return (device != null);
	}


    private void queryDevices() {
    	if (DEBUG) Log.d(TAG, "Searching for devices matching the selected preference");

		Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
		// If there are paired devices
		if (pairedDevices.size() > 0) {
		    // Loop through paired devices
		    for (BluetoothDevice device : pairedDevices) {

		    	if(device.getAddress().compareTo(device_name) == 0) {
		    		if (DEBUG) Log.d(TAG, "Found selected device: " + device.getName());
		    		this.device = device;

		    		openTelemetryBluetooth();
		    		return;
		    	}
		    }
		}

		attemptedFailed();
    }

	private boolean openTelemetryBluetooth() {
		if (DEBUG) Log.d(TAG, "Opening connection to " + device.getName());

		socket = null;

		try {
			socket = device.createRfcommSocketToServiceRecord(MY_UUID);
		} catch (IOException e) {
			if (ERROR) Log.e(TAG,"Unable to create Rfcomm socket");
			return false;
		}

		mBluetoothAdapter.cancelDiscovery();

		try {
			socket.connect();
		}
		catch (IOException e) {
			if (ERROR) Log.e(TAG,"Unable to connect to requested device", e);
            try {
                socket.close();
            } catch (IOException e2) {
                if (ERROR) Log.e(TAG, "unable to close() socket during connection failure", e2);
            }

            attemptedFailed();
			return false;
		}

		try {
			inStream = socket.getInputStream();
			outStream = socket.getOutputStream();
		} catch (IOException e) {
			try {
				socket.close();
			} catch (IOException e2) {

			}
            attemptedFailed();
			return false;
		}

		telemService.toastMessage("Bluetooth device connected");

		// Post message to call attempt succeeded on the parent class
		handler.post(new Runnable() {
			@Override
			public void run() {
				BluetoothUAVTalk.this.attemptSucceeded();
			}
		});

		return true;
	}

}
