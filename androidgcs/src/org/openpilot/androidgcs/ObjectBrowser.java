package org.openpilot.androidgcs;

import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;
import java.util.Observable;
import java.util.Observer;

import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;

import org.openpilot.uavtalk.UAVDataObject;
import org.openpilot.uavtalk.UAVObject;

public class ObjectBrowser extends ObjectManagerActivity implements OnSharedPreferenceChangeListener {

	private final String TAG = "ObjectBrower";
	boolean connected;
	SharedPreferences prefs;
	ArrayAdapter<UAVDataObject> adapter;
	List<UAVDataObject> allObjects;
	
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
		setContentView(R.layout.object_browser);		
		prefs = PreferenceManager.getDefaultSharedPreferences(this);
		prefs.registerOnSharedPreferenceChangeListener(this);
		
		Spinner objectFilter = (Spinner) findViewById(R.id.object_list_filter);
	}

	@Override
	void onOPConnected() {
		Toast.makeText(this,"Telemetry estabilished",Toast.LENGTH_SHORT);
		Log.d(TAG, "onOPConnected()");

		List<List<UAVDataObject>> allobjects = objMngr.getDataObjects();
		allObjects = new ArrayList<UAVDataObject>();
		ListIterator<List<UAVDataObject>> li = allobjects.listIterator();
		while(li.hasNext()) {
			allObjects.addAll(li.next());
		}

		adapter = new ArrayAdapter<UAVDataObject>(this,R.layout.object_view, allObjects);
		ListView objects = (ListView) findViewById(R.id.object_list);
		objects.setAdapter(adapter);

		objects.setOnItemClickListener(new OnItemClickListener() {
			public void onItemClick(AdapterView<?> parent, View view,
					int position, long id) {
			      /*Toast.makeText(getApplicationContext(), ((TextView) view).getText(),
			              Toast.LENGTH_SHORT).show();*/
				Intent intent = new Intent(ObjectBrowser.this, ObjectEditor.class);
				intent.putExtra("org.openpilot.androidgcs.ObjectName", allObjects.get(position).getName());
				intent.putExtra("org.openpilot.androidgcs.ObjectId", allObjects.get(position).getObjID());
				intent.putExtra("org.openpilot.androidgcs.InstId", allObjects.get(position).getInstID());
				startActivity(intent);
			}
		});


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
