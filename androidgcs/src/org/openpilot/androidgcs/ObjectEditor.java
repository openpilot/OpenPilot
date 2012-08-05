package org.openpilot.androidgcs;

import java.util.List;
import java.util.ListIterator;

import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectField;

import android.os.Bundle;

public class ObjectEditor extends ObjectManagerActivity {

	String objectName;
	int objectID;
	int instID;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.object_editor);

		// TODO: Figure out why this line is required so it doesn't
		// have to be set programmatically
		setTheme(android.R.style.Theme_Holo);
		
	    Bundle extras = getIntent().getExtras();
	    if(extras != null){
	    	objectName = extras.getString("org.openpilot.androidgcs.ObjectName");
	        objectID = extras.getInt("org.openpilot.androidgcs.ObjectId");
	        instID = extras.getInt("org.openpilot.androidgcs.InstId");
	        
	        setTitle(objectName);
	    }
	    
	}	
	
	public void onOPConnected() {
		UAVObject obj = objMngr.getObject(objectID, instID);
		if (obj == null)
			return;

		ObjectEditView editView = (ObjectEditView) findViewById(R.id.object_edit_view);
		editView.setName(obj.getName());
		
		List<UAVObjectField> fields = obj.getFields();
		ListIterator<UAVObjectField> li = fields.listIterator();
		while(li.hasNext()) {
			editView.addField(li.next());
		}
	}

}
