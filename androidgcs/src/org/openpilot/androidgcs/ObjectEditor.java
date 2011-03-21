package org.openpilot.androidgcs;

import java.util.List;
import java.util.ListIterator;

import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectField;

import android.os.Bundle;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

public class ObjectEditor extends ObjectManagerActivity {

	String objectName;
	int objectID;
	int instID;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.object_edit);

		System.out.println("Started. Intent:" + getIntent());
	    Bundle extras = getIntent().getExtras();
	    if(extras != null){
	    	objectName = extras.getString("org.openpilot.androidgcs.ObjectName");
	        objectID = extras.getInt("org.openpilot.androidgcs.ObjectId");
	        instID = extras.getInt("org.openpilot.androidgcs.InstId");
	    }	    	    
	}	
	
	public void onOPConnected() {
		UAVObject obj = objMngr.getObject(objectID, instID);
		Toast.makeText(getApplicationContext(), obj.toString(), Toast.LENGTH_SHORT);
		
		TextView objectName = (TextView) findViewById(R.id.object_edit_name);
		objectName.setText(obj.getName());
		
		LinearLayout fieldViewList = (LinearLayout) findViewById(R.id.object_edit_fields);
		List<UAVObjectField> fields = obj.getFields();
		ListIterator<UAVObjectField> li = fields.listIterator();
		while(li.hasNext()) {
			UAVObjectField field = li.next();
			TextView fieldName = new TextView(this);
			fieldName.setText(field.getName());
			fieldViewList.addView(fieldName);
		}
	}

}
