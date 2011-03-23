package org.openpilot.androidgcs;

import java.util.List;
import java.util.ListIterator;

import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectField;

import android.os.Bundle;
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
		
		ObjectEditView editView = (ObjectEditView) findViewById(R.id.object_edit_view);
		editView.setName(obj.getName());
		
		List<UAVObjectField> fields = obj.getFields();
		ListIterator<UAVObjectField> li = fields.listIterator();
		while(li.hasNext()) {
			editView.addField(li.next());
		}
	}

}
