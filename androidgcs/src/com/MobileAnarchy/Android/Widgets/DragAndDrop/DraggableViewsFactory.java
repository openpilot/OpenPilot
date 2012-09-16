package com.MobileAnarchy.Android.Widgets.DragAndDrop;

import android.content.Context;
import android.view.View;
import android.widget.TextView;
import android.widget.TableLayout.LayoutParams;

public class DraggableViewsFactory {

	public static View getLabel(String text) {
		Context context = DragAndDropManager.getInstance().getContext();
		TextView textView = new TextView(context);
		textView.setText(text);
		textView.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
		//textView.setGravity(Gravity.TOP + Gravity.LEFT);
		return textView;
	}
	
}
