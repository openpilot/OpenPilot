package org.openpilot.androidgcs;

import java.util.List;
import java.util.ListIterator;

import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectField;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

public class ObjectEditor extends ObjectManagerActivity {

	static final String TAG = "ObjectEditor"; 
	String objectName;
	long objectID;
	long instID;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.object_editor);

		// TODO: Figure out why this line is required so it doesn't
		// have to be set programmatically
		setTheme(android.R.style.Theme_Holo);

		Bundle extras = getIntent().getExtras();
		if (extras == null)
			return;

		objectName = extras.getString("org.openpilot.androidgcs.ObjectName");
		objectID = extras.getLong("org.openpilot.androidgcs.ObjectId");
		instID = extras.getLong("org.openpilot.androidgcs.InstId");

		setTitle(objectName);

		Button sendButton = (Button) findViewById(R.id.object_edit_send_button);
		sendButton.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				updateObject();
			}
		});

		Button saveButton = (Button) findViewById(R.id.object_edit_save_button);
		saveButton.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				saveObject();
			}
		});
	}

	public void onOPConnected() {
		UAVObject obj = objMngr.getObject(objectID, instID);
		if (obj == null) {
			Log.d(TAG, "Object not found:" + objectID);
			return;
		}

		ObjectEditView editView = (ObjectEditView) findViewById(R.id.object_edit_view);
		editView.setName(obj.getName());

		List<UAVObjectField> fields = obj.getFields();
		ListIterator<UAVObjectField> li = fields.listIterator();
		while (li.hasNext()) {
			editView.addField(li.next());
		}
	}

	/**
	 * Fetch the data back from the view and then send it to the UAV
	 */
	private void saveObject() {

		UAVObject objPer = objMngr.getObject("ObjectPersistence");

		if( !updateObject()  || objPer == null) {
			Toast.makeText(this, "Save failed", Toast.LENGTH_LONG).show();
			return;
		}
		
		long thisId = objectID < 0 ? ((Long) 0x100000000l) + objectID : objectID; 
		objPer.getField("Operation").setValue("Save");
		objPer.getField("Selection").setValue("SingleObject");
		Log.d(TAG,"Saving with object id: " + objectID + " swapped to " + thisId);
		objPer.getField("ObjectID").setValue(thisId);
		objPer.getField("InstanceID").setValue(instID);
		objPer.updated();
		
		Toast.makeText(this, "Save succeeded", Toast.LENGTH_LONG).show();
	}

	/**
	 * Fetch the data back from the view and then send it to the UAV
	 */
	private boolean updateObject() {
		UAVObject obj = objMngr.getObject(objectID, instID);
		if (obj == null)
			return false;

		Log.d(TAG, "Updating object id " + obj.getObjID());
		ObjectEditView editView = (ObjectEditView) findViewById(R.id.object_edit_view);

		int field_idx = 0;

		List<UAVObjectField> fields = obj.getFields();
		ListIterator<UAVObjectField> li = fields.listIterator();
		while (li.hasNext()) {
			UAVObjectField field = li.next();
			int num_fields = field.getNumElements();
			for (int i = 0; i < num_fields; i++) {
				switch (field.getType()) {
				case ENUM:
					int selected = ((Spinner)editView.fields.get(field_idx)).getSelectedItemPosition();
					field.setValue(selected, i);
					break;
				default:
					String val = ((EditText) editView.fields.get(field_idx)).getText().toString();
					Double num = Double.parseDouble(val);
					
					Log.e(TAG, "Updating field: " + field.getName() + " value: " + num);
					field.setValue(num, i);
					break;
				}

				field_idx++;
			}
		}
		obj.updated();

		return true;
	}

}
