package org.openpilot.androidgcs;

import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;
import java.util.Observable;
import java.util.Observer;

import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.database.DataSetObserver;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Adapter;
import android.widget.ArrayAdapter;
import android.widget.ExpandableListAdapter;
import android.widget.ExpandableListView;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.SimpleExpandableListAdapter;
import android.widget.TextView;
import android.widget.Toast;

import org.openpilot.uavtalk.UAVDataObject;
import org.openpilot.uavtalk.UAVObject;

public class ObjectBrowser extends ObjectManagerActivity implements OnSharedPreferenceChangeListener {

	private final String TAG = "ObjectBrower";
	boolean connected;
	SharedPreferences prefs;
	ArrayAdapter<UAVDataObject> adapter;
	
	final Handler uavobjHandler = new Handler(); 
	final Runnable updateText = new Runnable() {
		public void run() {
			Log.d(TAG,"Update");
			update();
		}
	};

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);		
		prefs = PreferenceManager.getDefaultSharedPreferences(this);
		prefs.registerOnSharedPreferenceChangeListener(this);
	}

	@Override
	void onOPConnected() {
		Toast.makeText(this,"Telemetry estabilished",Toast.LENGTH_SHORT);
		Log.d(TAG, "onOPConnected()");
		
		List<List<UAVDataObject>> allobjects = objMngr.getDataObjects();
		List<UAVDataObject> linearized = new ArrayList<UAVDataObject>();
		ListIterator<List<UAVDataObject>> li = allobjects.listIterator();
		while(li.hasNext()) {
			linearized.addAll(li.next());
		}
		
		adapter = new ArrayAdapter<UAVDataObject>(this,R.layout.object_view, linearized);
		ListView objects = (ListView) findViewById(R.id.object_list);
		objects.setAdapter(adapter);

		UAVObject obj = objMngr.getObject("SystemStats");
		if(obj != null)
			obj.addUpdatedObserver(new Observer() {
				public void update(Observable observable, Object data) {
					uavobjHandler.post(updateText);
				}				
			});

	}
	
	public void update() {
		adapter.notifyDataSetChanged();
	}

	public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,
			String key) {
		// TODO Auto-generated method stub
		
	}
}
