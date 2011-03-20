package org.openpilot.androidgcs;

import java.util.Observable;
import java.util.Observer;

import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.widget.TextView;
import android.widget.ToggleButton;

import org.openpilot.uavtalk.UAVObject;

public class ObjectBrowser extends ObjectManagerActivity {

	private final String TAG = "ObjectBrower";
	boolean connected;

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
	}

	@Override
	void onOPConnected() {
		//		Toast.makeText(this,"Telemetry estabilished",Toast.LENGTH_SHORT);
		Log.d(TAG, "onOPConnected()");

		UAVObject obj = objMngr.getObject("SystemStats");
		Log.d(TAG, ((Boolean) (obj == null)).toString());
		if(obj != null)
			obj.addUpdatedObserver(new Observer() {
				public void update(Observable observable, Object data) {
					uavobjHandler.post(updateText);
				}				
			});

	}
}
