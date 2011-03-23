package org.openpilot.androidgcs;

import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;

import org.openpilot.uavtalk.UAVObjectField;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

public class ObjectEditView extends LinearLayout {

	TextView objectName;
	List<View> fields;

	public ObjectEditView(Context context) {
		super(context);		
		initObjectEditView();
	}

	public ObjectEditView(Context context, AttributeSet ats, int defaultStyle) { 
		super(context, ats);
		initObjectEditView();
	}

	public ObjectEditView(Context context, AttributeSet ats) { 
		super(context, ats);
		initObjectEditView();
	}

	public void initObjectEditView() {
		// Set orientation of layout to vertical 
		setOrientation(LinearLayout.VERTICAL);

		objectName = new TextView(getContext());
		objectName.setText("");
		objectName.setTextSize(14);

		// Lay them out in the compound control. 
		int lHeight = LayoutParams.WRAP_CONTENT; 
		int lWidth = LayoutParams.FILL_PARENT;
		addView(objectName, new LinearLayout.LayoutParams(lWidth, lHeight));

		fields = new ArrayList<View>();
	}

	public void setName(String name) {
		objectName.setText(name);
	}

	public void addField(UAVObjectField field) {
		if(field.getNumElements() == 1) {
			FieldValue fieldView = new FieldValue(getContext());
			fieldView.setName(field.getName());
			if(field.isNumeric()) {
				fieldView.setValue(new Double(field.getDouble()).toString());
			} else {
				fieldView.setValue(field.getValue().toString());
			}
			fields.add(fieldView);
			addView(fieldView, new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.FILL_PARENT));
		} 
		else {
			ListIterator<String> names = field.getElementNames().listIterator();
			int i = 0;
			while(names.hasNext()) {
				FieldValue fieldView = new FieldValue(getContext());
				fieldView.setName(field.getName() + "-" + names.next());
				if(field.isNumeric()) {
					fieldView.setValue(new Double(field.getDouble(i)).toString());
				} else {
					fieldView.setValue(field.getValue(i).toString());
				}
				i++;
				fields.add(fieldView);
				addView(fieldView, new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.FILL_PARENT));

			}
		}
	}

	public class FieldValue extends LinearLayout {

		TextView fieldName;
		EditText fieldValue;

		public FieldValue(Context context) {
			super(context);
			setOrientation(LinearLayout.HORIZONTAL);

			fieldName = new TextView(getContext());
			fieldValue = new EditText(getContext());

			// Lay them out in the compound control.
			addView(fieldName, new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.FILL_PARENT));
			fieldValue.setWidth(300);
			addView(fieldValue, new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
		}

		public void setName(String name) {
			fieldName.setText(name);
		}

		public void setValue(String value) {
			fieldValue.setText(value);
		}		
	}

}
