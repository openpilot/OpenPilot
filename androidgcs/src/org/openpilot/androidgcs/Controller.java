package org.openpilot.androidgcs;

import java.util.Observable;
import java.util.Observer;
import java.util.Timer;
import java.util.TimerTask;

import org.openpilot.uavtalk.UAVDataObject;
import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectField;

import com.MobileAnarchy.Android.Widgets.Joystick.DualJoystickView;

import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

public class Controller extends ObjectManagerActivity {
	private final String TAG = "Controller";

	private final int THROTTLE_CHANNEL = 0;
	private final int ROLL_CHANNEL = 1;
	private final int PITCH_CHANNEL = 2;
	private final int YAW_CHANNEL = 3;
	private final int FLIGHTMODE_CHANNEL = 4;
	
	private final int CHANNEL_MIN = 1000;
	private final int CHANNEL_MAX = 2000;
	private final int CHANNEL_NEUTRAL = 1500;
	private final int CHANNEL_NEUTRAL_THROTTLE = 1100;
	
	private double throttle = 0.1, roll = 0.1, pitch = -0.1, yaw = 0;
	private boolean updated;
	
	Timer sendTimer = new Timer();
	
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.controller);
		TextView manualView = (TextView) findViewById(R.id.manualControlValues);
		if(manualView != null)
			manualView.setText("Hello");
	}

	
	@Override
	void onOPConnected() {
		Log.d(TAG, "onOPConnected()");
		
		// Subsribe to updates from ManualControlCommand and show the values for crude feedback
		UAVDataObject manualControl = (UAVDataObject) objMngr.getObject("ManualControlCommand");
		if(manualControl != null) {
			manualControl.addUpdatedObserver(updatedObserver);
		}
		
		activateGcsReceiver();
		
		TimerTask controllerTask = new TimerTask() {
			public void run() {
				uavobjHandler.post(new Runnable() {
					@Override
					public void run() {
						//DualJoystickView joystick = (DualJoystickView) findViewById(R.id.dualjoystickView);
						
						UAVObject gcsReceiver = objMngr.getObject("GCSReceiver");
						if (gcsReceiver == null) {
							Log.e(TAG, "No GCS Receiver object found");
							return;
						}
						
						UAVObjectField channels = gcsReceiver.getField("Channel");
						if(channels == null) {
							Log.e(TAG, "GCS Receiver object ill formatted");
							return;
						}
						
						channels.setValue(scaleChannel(throttle, CHANNEL_NEUTRAL_THROTTLE), THROTTLE_CHANNEL);
						channels.setValue(scaleChannel(roll, CHANNEL_NEUTRAL), ROLL_CHANNEL);
						channels.setValue(scaleChannel(pitch, CHANNEL_NEUTRAL), PITCH_CHANNEL);
						channels.setValue(scaleChannel(yaw, CHANNEL_NEUTRAL), YAW_CHANNEL);
						channels.setValue(scaleChannel(0, CHANNEL_NEUTRAL), FLIGHTMODE_CHANNEL);
						
						gcsReceiver.updated();
						
						Log.d(TAG, "Send update");
						updated = false;
					}
				});
			}
		};
		sendTimer.schedule(controllerTask, 500, 50);
	}

	/**
	 * The callbacks from the UAVOs must run in the correct thread to update the
	 * UI.  This is what using a runnable does.
	 */
	final Handler uavobjHandler = new Handler(); 
	final Runnable updateText = new Runnable() {
		public void run() {
			updateManualControl();
		}
	};
	
	private final Observer updatedObserver = new Observer() {
		public void update(Observable observable, Object data) {
			uavobjHandler.post(updateText);
		}				
	};

	/**
	 * Show the string description of manual control command
	 */
	private void updateManualControl() {
		UAVDataObject manualControl = (UAVDataObject) objMngr.getObject("ManualControlCommand");
		TextView manualView = (TextView) findViewById(R.id.manualControlValues);
		if (manualView != null && manualControl != null)
			manualView.setText(manualControl.toStringData());
	}
	
	/**
	 * Active GCS receiver mode
	 */
	private void activateGcsReceiver() {
		UAVObject manualControlSettings = objMngr.getObject("ManualControlSettings");
		if (manualControlSettings == null) {
			Toast.makeText(this, "Failed to get manual control settings", Toast.LENGTH_SHORT).show();
			return;
		}
		
		UAVObjectField channelGroups = manualControlSettings.getField("ChannelGroups");
		UAVObjectField channelNumber = manualControlSettings.getField("ChannelNumber");
		UAVObjectField channelMax = manualControlSettings.getField("ChannelMax");
		UAVObjectField channelNeutral = manualControlSettings.getField("ChannelNeutral");
		UAVObjectField channelMin = manualControlSettings.getField("ChannelMin");
		if (channelGroups == null || channelMax == null || channelNeutral == null || 
				channelMin == null || channelNumber == null) {
			Toast.makeText(this,  "Manual control settings not formatted correctly", Toast.LENGTH_SHORT).show();
			return;
		}
		
		/* Configure the manual control module how the GCS controller expects
		 * This order MUST correspond to the enumeration order of ChannelNumber in 
		 * ManualControlSettings.
		 */ 
		int channels[] = { THROTTLE_CHANNEL, ROLL_CHANNEL, PITCH_CHANNEL, YAW_CHANNEL, FLIGHTMODE_CHANNEL };
		for (int i = 0; i < channels.length; i++) {
			channelGroups.setValue("GCS", channels[i]);
			channelNumber.setValue(1 + channels[i], i); // Add 1 because this uses 0 for "NONE"
			channelMin.setValue(CHANNEL_MIN, channels[i]);
			channelMax.setValue(CHANNEL_MAX, channels[i]);
			switch(channels[i]) {
			case THROTTLE_CHANNEL:
				channelNeutral.setValue(CHANNEL_NEUTRAL_THROTTLE, channels[i]);
				break;
			default:
				channelNeutral.setValue(CHANNEL_NEUTRAL, channels[i]);
			break;
			}
		}
		
		// Send settings to the UAV
		manualControlSettings.updated();
		
		
	}
	
	/**
	 * Scale the channels to the output range the flight controller expects
	 */
	private float scaleChannel(double in, double neutral) {	
		// Check bounds
		if (in > 1) 
			in = 1;
		if (in < -1)
			in = -1;
		
		if (in >= 0)
			return (float) (neutral + (CHANNEL_MAX - neutral) * in);
		return (float) (neutral + (neutral - CHANNEL_MIN) * in);
	}

}
