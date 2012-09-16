package com.MobileAnarchy.Android.Widgets.DragAndDrop;

import android.view.View;

public class DraggableItem {

	// =========================================
	// Private members
	// =========================================

	private View source;
	private View draggedView;
	private Object tag; 

	// =========================================
	// Constructor
	// =========================================

	public DraggableItem(View source, View draggedItem) {
		this.source = source;
		this.draggedView = draggedItem;
	}

	// =========================================
	// Public properties
	// =========================================

	public Object getTag() {
		return tag;
	}

	public void setTag(Object tag) {
		this.tag = tag;
	}

	public View getSource() {
		return source;
	}

	public View getDraggedView() {
		return draggedView;
	}
	
}
