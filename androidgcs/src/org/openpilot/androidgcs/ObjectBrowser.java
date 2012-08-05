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
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;

import org.openpilot.uavtalk.UAVDataObject;

public class ObjectBrowser extends ObjectManagerActivity implements OnSharedPreferenceChangeListener {

	private final String TAG = "ObjectBrower";
	int selected_index = -1;
	boolean connected;
	SharedPreferences prefs;
	ArrayAdapter<UAVDataObject> adapter;
	List<UAVDataObject> allObjects;
	
	final Handler uavobjHandler = new Handler(); 
	final Runnable updateText = new Runnable() {
		public void run() {
			updateObject();
		}
	};
	
	private final Observer updatedObserver = new Observer() {
		public void update(Observable observable, Object data) {
			uavobjHandler.post(updateText);
		}				
	};

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.object_browser);		
		prefs = PreferenceManager.getDefaultSharedPreferences(this);
		prefs.registerOnSharedPreferenceChangeListener(this);
	}

	@Override
	void onOPConnected() {
		Log.d(TAG, "onOPConnected()");
		
		OnCheckedChangeListener checkListener = new OnCheckedChangeListener() {
			@Override
			public void onCheckedChanged(CompoundButton buttonView,
					boolean isChecked) {
				updateList();
			}
		};
		
		((CheckBox) findViewById(R.id.dataCheck)).setOnCheckedChangeListener(checkListener);
		((CheckBox) findViewById(R.id.settingsCheck)).setOnCheckedChangeListener(checkListener);
		
		((Button) findViewById(R.id.editButton)).setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				if (selected_index > 0) {
					Intent intent = new Intent(ObjectBrowser.this, ObjectEditor.class);
					intent.putExtra("org.openpilot.androidgcs.ObjectName", allObjects.get(selected_index).getName());
					intent.putExtra("org.openpilot.androidgcs.ObjectId", allObjects.get(selected_index).getObjID());
					intent.putExtra("org.openpilot.androidgcs.InstId", allObjects.get(selected_index).getInstID());
					startActivity(intent);
				}
			}
		});

		updateList();
	}

	/**
	 * Populate the list of UAVO objects based on the selected filter
	 */
	private void updateList() {
		// Disconnect any previous signals
		if (selected_index > 0)
			allObjects.get(selected_index).removeUpdatedObserver(updatedObserver);
		selected_index = -1;

		boolean includeData = ((CheckBox) findViewById(R.id.dataCheck)).isChecked();
		boolean includeSettings = ((CheckBox) findViewById(R.id.settingsCheck)).isChecked();

		List<List<UAVDataObject>> allobjects = objMngr.getDataObjects();
		allObjects = new ArrayList<UAVDataObject>();
		ListIterator<List<UAVDataObject>> li = allobjects.listIterator();
		while(li.hasNext()) {
			List<UAVDataObject> objects = li.next();
			if(includeSettings && objects.get(0).isSettings())
				allObjects.addAll(objects);
			else if (includeData && !objects.get(0).isSettings())
				allObjects.addAll(objects);
		}

		adapter = new ArrayAdapter<UAVDataObject>(this,R.layout.object_view, allObjects);
		ListView objects = (ListView) findViewById(R.id.object_list);
		objects.setAdapter(adapter);

		objects.setOnItemClickListener(new OnItemClickListener() {
			public void onItemClick(AdapterView<?> parent, View view,
					int position, long id) {

				if (selected_index > 0)
					allObjects.get(selected_index).removeUpdatedObserver(updatedObserver);

				selected_index = position;
				allObjects.get(position).addUpdatedObserver(updatedObserver);
				allObjects.get(position).updateRequested();
				updateObject();
			}
		});
		
	}
	
	private void updateObject() {
		//adapter.notifyDataSetChanged();
		TextView text = (TextView) findViewById(R.id.object_information);
		if (selected_index >= 0 && selected_index < allObjects.size())
			text.setText(allObjects.get(selected_index).toStringData());
		else
			Log.d(TAG,"Update called but invalid index: " + selected_index);
	}

	public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,
			String key) {
		// TODO Auto-generated method stub

	}
}
