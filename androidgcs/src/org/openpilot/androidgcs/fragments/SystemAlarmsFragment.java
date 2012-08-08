/**
 ******************************************************************************
 * @file       SystemAlarmsFragment.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      A fragment that will connect to the SystemAlarms and visualize them.
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
package org.openpilot.androidgcs.fragments;

import java.util.List;

import org.openpilot.androidgcs.R;
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
	private static final int LOGLEVEL = 0;
	// private static boolean WARN = LOGLEVEL > 1;
	private static final boolean DEBUG = LOGLEVEL > 0;

	//@Override
    @Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.system_alarms_fragment, container, false);
    }

    @Override
	public void onOPConnected(UAVObjectManager objMngr) {
    	super.onOPConnected(objMngr);
		if (DEBUG)
			Log.d(TAG, "On connected");

    	UAVObject obj = objMngr.getObject("SystemAlarms");
		if (obj != null)
			registerObjectUpdates(obj);
		objectUpdated(obj);
    }

	/**
	 * Called whenever any objects subscribed to via registerObjects
	 */
    @Override
	public void objectUpdated(UAVObject obj) {
		if (DEBUG)
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
