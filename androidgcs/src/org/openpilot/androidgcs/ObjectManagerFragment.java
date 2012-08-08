package org.openpilot.androidgcs;

import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectManager;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
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
        
        ObjectManagerActivity castActivity = null;
        try {
        	castActivity = (ObjectManagerActivity)activity;
        } catch (ClassCastException e) {
        	throw new android.app.Fragment.InstantiationException(
        			"Attaching a ObjectManagerFragment to an activity failed because the parent activity is not a ObjectManagerActivity",
        			e);
        }
        castActivity.addOnConnectionListenerFragment(this);
    }
    
    
	// The below methods should all be called by the parent activity at the appropriate times
    protected void onOPConnected(UAVObjectManager objMngr) {
		this.objMngr = objMngr;
		if (DEBUG) Log.d(TAG,"onOPConnected");
	}
	
	protected void onOPDisconnected() {
		objMngr = null;
		if (DEBUG) Log.d(TAG,"onOPDisconnected");
	}
		
	/**
	 * Called whenever any objects subscribed to via registerObjects 
	 */
	protected void objectUpdated(UAVObject obj) {
		
	}	

	/**
	 * Register on the activities object monitor handler so that updates
	 * occur within that UI thread.  No need to maintain a handler for
	 * each fragment.
	 */
	protected void registerObjectUpdates(UAVObject object) {
		((ObjectManagerActivity) getActivity()).registerObjectUpdates(object, this);
	}
	
}
