/**
 ******************************************************************************
 * @file       TcpUAVTalk.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      UAVTalk over TCP.
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
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

public class TcpUAVTalk extends TelemetryTask {
	private final String TAG = "TcpUAVTalk";
	public static int LOGLEVEL = 2;
	public static boolean WARN = LOGLEVEL > 1;
	public static boolean DEBUG = LOGLEVEL > 0;

	// Temporarily define fixed device name
	private String ip_address = "1";
	private int port = 9001;

	private Socket socket;

	/**
	 * Construct a TcpUAVTalk object attached to the OPTelemetryService.  Gets the
	 * connection settings from the preferences.
	 */
	public TcpUAVTalk(OPTelemetryService caller) {
		super(caller);
	}

	@Override
	boolean attemptConnection() {

		if( getConnected() )
			return true;

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(telemService);
		ip_address = prefs.getString("ip_address","127.0.0.1");
		try {
			port = Integer.decode(prefs.getString("port", ""));
		} catch (NumberFormatException e) {
			//TODO: Handle this exception
		}

		if (DEBUG) Log.d(TAG, "Trying to open UAVTalk with " + ip_address);

		Log.d(TAG, "Opening connection to " + ip_address + " at address " + port);

		InetAddress serverAddr = null;
		try {
			serverAddr = InetAddress.getByName(ip_address);
		} catch (UnknownHostException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
			return false;
		}

		socket = null;
		try {
			socket = new Socket(serverAddr,port);
		} catch (IOException e1) {
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
			return false;
		}


		// Post message to call attempt succeeded on the parent class
		handler.post(new Runnable() {
			@Override
			public void run() {
				TcpUAVTalk.this.attemptSucceeded();
			}
		});

		return true;
	}


	@Override
	public void disconnect() {
		super.disconnect();
		if (socket != null) {
			try {
				socket.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			socket = null;
		}
	}

}
