package org.openpilot.androidgcs;

import java.util.List;

import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectField;
import org.openpilot.uavtalk.UAVObjectManager;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

public class SystemAlarmsFragment extends ObjectManagerFragment {
	
	private static final String TAG = SystemAlarmsFragment.class.getSimpleName();

	//@Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.system_alarms_fragment, container, false);
    }
    
    public void onOPConnected(UAVObjectManager objMngr) {
    	super.onOPConnected(objMngr);
    	Log.d(TAG,"On connected");
    	
    	UAVObject obj = objMngr.getObject("SystemAlarms");
		if (obj != null)
			registerObjectUpdates(obj);
		objectUpdated(obj);
    }
    
	/**
	 * Called whenever any objects subscribed to via registerObjects 
	 */
    @Override
	protected void objectUpdated(UAVObject obj) {
    	Log.d(TAG, "Updated");
		if (obj.getName().compareTo("SystemAlarms") == 0) {
			TextView alarms = (TextView) getActivity().findViewById(R.id.system_alarms_fragment_field);
			UAVObjectField a = obj.getField("Alarm");
			List<String> names = a.getElementNames();
			String contents = new String();
			List <String> options = a.getOptions();
			
			// Rank the alarms by order of severity, skip uninitialized
			for (int j = options.size() - 1; j > 0; j--) {
				for (int i = 0; i < names.size(); i++) {
					if(a.getDouble(i) == j)
						contents += names.get(i) + " : " + a.getValue(i).toString() + "\n";
				}
			}
			alarms.setText(contents);
		}
	}
	

}
