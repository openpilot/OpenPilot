package org.openpilot.androidgcs;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import org.openpilot.uavtalk.UAVObjectManager;
import org.openpilot.uavtalk.UAVTalk;

import android.content.Context;
import android.util.Log;

public class TcpUAVTalk {
	private final String TAG = "TcpUAVTalk";
	public static int LOGLEVEL = 2;
	public static boolean WARN = LOGLEVEL > 1;
	public static boolean DEBUG = LOGLEVEL > 0;
	
	// Temporarily define fixed device name
	public final static String IP_ADDRESS = "127.0.0.1";
	public final static int PORT = 9001;
	
	private UAVTalk uavTalk;
	private boolean connected; 
	
	public TcpUAVTalk(Context caller) {
        if (DEBUG) Log.d(TAG, "Trying to open UAVTalk with " + IP_ADDRESS);

        connected = false;         
    }
	
	public boolean connect(UAVObjectManager objMngr) {
		if( getConnected() ) 
			return true;
		if( !openTelemetryTcp(objMngr) )
			return false;
		return true;		
	}

	public boolean getConnected() {
		return connected;
	}
    	
	public UAVTalk getUavtalk() {
		return uavTalk;
	}
	
 
	private boolean openTelemetryTcp(UAVObjectManager objMngr) {
		Log.d(TAG, "Opening conncetion to " + IP_ADDRESS);
		
		InetAddress serverAddr = null;
		try {
			serverAddr = InetAddress.getByName(IP_ADDRESS);
		} catch (UnknownHostException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
			return false;
		}
		
		Socket socket = null;
		try {
			socket = new Socket(serverAddr,PORT);
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
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
