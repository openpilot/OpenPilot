package org.openpilot.androidgcs;

import java.util.List;
import java.util.ListIterator;
import java.util.Observable;
import java.util.Observer;

import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectManager;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;

public class ObjectManagerFragment extends Fragment {
	
	private static final String TAG = ObjectManagerFragment.class.getSimpleName();
	private static int LOGLEVEL = 1;
//	private static boolean WARN = LOGLEVEL > 1;
	private static boolean DEBUG = LOGLEVEL > 0;
    
	UAVObjectManager objMngr;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		if (DEBUG) Log.d(TAG, "Created an ObjectManagerFragment");
		// For an activity this registers against the telemetry service intents.  Fragments must be notified by their
		// parent activity
	}
    
	/**
	 * Attach to the parent activity so it can notify us when the connection
	 * changed
	 */
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        if (DEBUG) Log.d(TAG,"onAttach");
        
        ((ObjectManagerActivity)activity).addOnConnectionListenerFragment(this);
    }
    
	// The below methods should all be called by the parent activity at the appropriate times
	void onOPConnected(UAVObjectManager objMngr) {
		this.objMngr = objMngr;
		if (DEBUG) Log.d(TAG,"onOPConnected");
	}
	
	void onOPDisconnected() {
		objMngr = null;
		if (DEBUG) Log.d(TAG,"onOPDisconnected");
	}
		
	public void onBind() {
		
	}

	
	/**
	 * Called whenever any objects subscribed to via registerObjects 
	 */
	protected void objectUpdated(UAVObject obj) {
		
	}
	
	/**
	 * A message handler and a custom Observer to use it which calls
	 * objectUpdated with the right object type
	 */
	final Handler uavobjHandler = new Handler(); 	
	private class UpdatedObserver implements Observer  {
		UAVObject obj;
		UpdatedObserver(UAVObject obj) { this.obj = obj; };
		public void update(Observable observable, Object data) {
			uavobjHandler.post(new Runnable() {
				@Override
				public void run() { objectUpdated(obj); }
			});
		}
	};

	/**
	 * Register an activity to receive updates from this object
	 * 
	 * the objectUpdated() method will be called in the original UI thread
	 */
	protected void registerObjectUpdates(UAVObject object) {
		object.addUpdatedObserver(new UpdatedObserver(object));
	}
	protected void registerObjectUpdates(List<List<UAVObject>> objects) {
		ListIterator<List<UAVObject>> li = objects.listIterator();
		while(li.hasNext()) {
			ListIterator<UAVObject> li2 = li.next().listIterator();
			while(li2.hasNext())
				registerObjectUpdates(li2.next());
		}
	}
	
}
