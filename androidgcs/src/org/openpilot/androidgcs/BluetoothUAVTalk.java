package org.openpilot.androidgcs;

import java.io.IOException;
import java.util.Set;
import java.util.UUID;

import org.openpilot.uavtalk.UAVObjectManager;
import org.openpilot.uavtalk.UAVTalk;

import android.annotation.TargetApi;
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

@TargetApi(10) public class BluetoothUAVTalk {
	private final String TAG = "BluetoothUAVTalk";
	public static int LOGLEVEL = 2;
	public static boolean WARN = LOGLEVEL > 1;
	public static boolean DEBUG = LOGLEVEL > 0;
	
	// Temporarily define fixed device name
	private String device_name = "RN42-222D";
	private final static UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
	
	private BluetoothAdapter mBluetoothAdapter;
	private BluetoothSocket socket;
	private BluetoothDevice device;
	private UAVTalk uavTalk;
	private boolean connected; 
	
	public BluetoothUAVTalk(Context caller) {
		
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(caller);
		device_name = prefs.getString("bluetooth_mac","");

        if (DEBUG) Log.d(TAG, "Trying to open UAVTalk with " + device_name);

        connected = false;
        device = null;
        
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter == null) {
            // Device does not support Bluetooth
        	Log.e(TAG, "Device does not support Bluetooth");
        	return;
        }
        
        if (!mBluetoothAdapter.isEnabled()) {
        	// Enable bluetooth if it isn't already
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            caller.sendOrderedBroadcast(enableBtIntent, "android.permission.BLUETOOTH_ADMIN", new BroadcastReceiver() {
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
    }
	
	public boolean connect(UAVObjectManager objMngr) {
		if( getConnected() ) 
			return true;
		if( !getFoundDevice() )
			return false;		
		if( !openTelemetryBluetooth(objMngr) )
			return false;
		return true;		
	}

	public boolean getConnected() {
		return connected;
	}
    
	public boolean getFoundDevice() {
		return (device != null);
	}
	
	public UAVTalk getUavtalk() {
		return uavTalk;
	}
	
    private void queryDevices() {
    	Log.d(TAG, "Searching for devices");
		Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
		// If there are paired devices
		if (pairedDevices.size() > 0) {
		    // Loop through paired devices
		    for (BluetoothDevice device : pairedDevices) {
		        // Add the name and address to an array adapter to show in a ListView
		        //mArrayAdapter.add(device.getName() + "\n" + device.getAddress());
		    	Log.d(TAG, "Paired device: " + device.getAddress() + " compared to " + device_name);
		    	if(device.getAddress().compareTo(device_name) == 0) {
		    		Log.d(TAG, "Found device: " + device.getName());
		    		this.device = device;
		    		return;
		    	}
		    }
		}
    	
    }

	private boolean openTelemetryBluetooth(UAVObjectManager objMngr) {
		Log.d(TAG, "Opening connection to " + device.getName());
		socket = null;
		connected = false;
		try {
			socket = device.createInsecureRfcommSocketToServiceRecord(MY_UUID);
		} catch (IOException e) {
			Log.e(TAG,"Unable to create Rfcomm socket");
			return false;
			//e.printStackTrace();			
		}
		
		mBluetoothAdapter.cancelDiscovery();
		
		try {
			socket.connect();
		}
		catch (IOException e) {
			Log.e(TAG,"Unable to connect to requested device", e);
            try {
                socket.close();
            } catch (IOException e2) {
                Log.e(TAG, "unable to close() socket during connection failure", e2);
            }
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
