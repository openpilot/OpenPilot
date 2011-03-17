package org.openpilot.androidgcs;

import java.io.IOException;
import java.util.Observable;
import java.util.Observer;
import java.util.Set;
import java.util.UUID;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.widget.TextView;
import android.widget.ToggleButton;

import org.openpilot.androidgcs.*;
import org.openpilot.uavtalk.Telemetry;
import org.openpilot.uavtalk.TelemetryMonitor;
import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectManager;
import org.openpilot.uavtalk.UAVTalk;
import org.openpilot.uavtalk.uavobjects.UAVObjectsInitialize;

public class ObjectBrowser extends Activity {
	
	private final String TAG = "ObjectBrower";
	private final String DEVICE_NAME = "RN42-222D";
	private final int REQUEST_ENABLE_BT = 0;
	private UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
	BluetoothAdapter mBluetoothAdapter;
	BluetoothSocket socket;
	boolean connected;
	UAVObjectManager objMngr;
	UAVTalk uavTalk;
	
	final Handler uavobjHandler = new Handler(); 
	final Runnable updateText = new Runnable() {
		public void run() {
			ToggleButton button = (ToggleButton) findViewById(R.id.toggleButton1);
			button.setChecked(!connected);

			Log.d(TAG,"HERE" + connected);
			
			TextView text = (TextView) findViewById(R.id.textView1);
			
			UAVObject obj1 = objMngr.getObject("SystemStats");
			UAVObject obj2 = objMngr.getObject("AttitudeRaw");
			UAVObject obj3 = objMngr.getObject("AttitudeActual");
			UAVObject obj4 = objMngr.getObject("SystemAlarms");

			if(obj1 == null || obj2 == null || obj3 == null || obj4 == null)
				return;

			Log.d(TAG,"And here");
			text.setText(obj1.toString() + "\n" + obj2.toString() + "\n" + obj3.toString() + "\n" + obj4.toString() );
			
		}
	};
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        Log.d(TAG, "Launching Object Browser");

        Log.d(TAG, "Start OP Telemetry Service");
        startService( new Intent( this, OPTelemetryService.class ) );
        
        objMngr = new UAVObjectManager();
		UAVObjectsInitialize.register(objMngr);

		UAVObject obj = objMngr.getObject("SystemStats");
		if(obj != null)
			obj.addUpdatedObserver(new Observer() {
				public void update(Observable observable, Object data) {
					uavobjHandler.post(updateText);
				}				
			});
		obj = objMngr.getObject("AttitudeRaw");
		if(obj != null)
			obj.addUpdatedObserver(new Observer() {
				public void update(Observable observable, Object data) {
					uavobjHandler.post(updateText);
				}				
			});
		obj = objMngr.getObject("AttitudeActual");
		if(obj != null)
			obj.addUpdatedObserver(new Observer() {
				public void update(Observable observable, Object data) {
					uavobjHandler.post(updateText);
				}				
			});
		obj = objMngr.getObject("SystemAlarms");
		if(obj != null)
			obj.addUpdatedObserver(new Observer() {
				public void update(Observable observable, Object data) {
					uavobjHandler.post(updateText);
				}				
			});		
    }
    
}
