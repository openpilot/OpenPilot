package org.openpilot.androidgcs;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import org.openpilot.uavtalk.UAVObjectManager;
import org.openpilot.uavtalk.UAVTalk;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

public class TcpUAVTalk {
	private final String TAG = "TcpUAVTalk";
	public static int LOGLEVEL = 2;
	public static boolean WARN = LOGLEVEL > 1;
	public static boolean DEBUG = LOGLEVEL > 0;
	
	// Temporarily define fixed device name
	private String ip_address = "1";
	private int port = 9001;
	
	private UAVTalk uavTalk;
	private boolean connected; 
	private Socket socket;
	
	/**
	 * Construct a TcpUAVTalk object attached to the OPTelemetryService.  Gets the 
	 * connection settings from the preferences.
	 */
	public TcpUAVTalk(Context caller) {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(caller);
		ip_address = prefs.getString("ip_address","127.0.0.1");
		try {
			port = Integer.decode(prefs.getString("port", ""));
		} catch (NumberFormatException e) {
			//TODO: Handle this exception
		}

		if (DEBUG) Log.d(TAG, "Trying to open UAVTalk with " + ip_address);

        connected = false;         
    }
	
	/**
	 * Connect a TCP object to an object manager.  Returns true if already
	 * connected, otherwise returns true if managed a successful socket.
	 */
	public boolean connect(UAVObjectManager objMngr) {
		if( getConnected() ) 
			return true;
		if( !openTelemetryTcp(objMngr) )
			return false;
		return true;		
	}
	
	public void disconnect() {
		try {
			socket.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		socket = null;
	}

	public boolean getConnected() {
		return connected;
	}
    	
	public UAVTalk getUavtalk() {
		return uavTalk;
	}
	
	/**
	 * Opens a TCP socket to the address determined on construction.  If successful
	 * creates a UAVTalk stream connection this socket to the passed in object manager
	 */
	private boolean openTelemetryTcp(UAVObjectManager objMngr) {
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

		connected = true;
		
		try {
			uavTalk = new UAVTalk(socket.getInputStream(), socket.getOutputStream(), objMngr);
		} catch (IOException e) {
			Log.e(TAG,"Error starting UAVTalk");
			// TODO Auto-generated catch block
			//e.printStackTrace();
			return false;
		}
				
		return true;
	}

}
