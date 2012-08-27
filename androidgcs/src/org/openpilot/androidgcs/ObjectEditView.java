package org.openpilot.androidgcs;

import java.util.ArrayList;
import java.util.List;

import org.openpilot.uavtalk.UAVObjectField;

import android.content.Context;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.GridLayout;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

public class ObjectEditView extends GridLayout {

	String objectName;
	public List<View> fields;

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
		setColumnCount(2);
		fields = new ArrayList<View>();
	}

	public void setName(String name) {
		objectName = name;
	}

	public void addField(UAVObjectField field) {
		for (int i = 0; i < field.getNumElements(); i++)
			addRow(getContext(), field, i);
	}


	public void addRow(Context context, UAVObjectField field, int idx) {
		int row = getRowCount();

		TextView fieldName = new TextView(context);
		if(field.getNumElements() == 1) {
			fieldName.setText(field.getName());
		} else {
			fieldName.setText(field.getName() + "-" + field.getElementNames().get(idx));
		}
		addView(fieldName, new GridLayout.LayoutParams(spec(row), spec(0)));

		View fieldValue = null;
		switch(field.getType())
		{
		case FLOAT32:
			fieldValue = new EditText(context);
			((EditText)fieldValue).setText(field.getValue(idx).toString());
			((EditText)fieldValue).setInputType(InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_SIGNED | InputType.TYPE_NUMBER_FLAG_DECIMAL);
			break;
		case INT8:
		case INT16:
		case INT32:
			fieldValue = new EditText(context);
			((EditText)fieldValue).setText(field.getValue(idx).toString());
			((EditText)fieldValue).setInputType(InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_SIGNED);
			break;
		case UINT8:
		case UINT16:
		case UINT32:
			fieldValue = new EditText(context);
			((EditText)fieldValue).setText(field.getValue(idx).toString());
			((EditText)fieldValue).setInputType(InputType.TYPE_CLASS_NUMBER);
			break;
		case ENUM:
			fieldValue = new Spinner(context);
			ArrayAdapter<String> adapter = new ArrayAdapter<String>(context, android.R.layout.simple_spinner_dropdown_item);
			adapter.addAll(field.getOptions());
			((Spinner) fieldValue).setAdapter(adapter);
			((Spinner) fieldValue).setSelection((int) field.getDouble(idx));
			break;
		case BITFIELD:
			fieldValue = new EditText(context);
			((EditText)fieldValue).setText(field.getValue(idx).toString());
			((EditText)fieldValue).setInputType(InputType.TYPE_CLASS_NUMBER);
			break;
		case STRING:
			fieldValue = new EditText(context);
			((EditText)fieldValue).setText(field.getValue(idx).toString());
		}

		addView(fieldValue, new GridLayout.LayoutParams(spec(row), spec(1)));
		fields.add(fieldValue);
	}

}
