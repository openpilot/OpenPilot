package com.MobileAnarchy.Android.Widgets.DragAndDrop;

import android.view.View;

public class DropZone {

	// =========================================
	// Private members
	// =========================================

	private View view;
	private DropZoneEventsListener listener;
	private int left, top, width, height;
	private Boolean dimansionsCalculated;
	
	// =========================================
	// Constructor
	// =========================================

	public DropZone(View view, DropZoneEventsListener listener) {
		this.view = view;
		this.listener = listener;
		dimansionsCalculated = false;
	}

	// =========================================
	// Public properties
	// =========================================
	
	public View getView() {
		return view;
	}
	
	// =========================================
	// Public methods
	// =========================================
	
	public Boolean isOver(int x, int y) {
		if (!dimansionsCalculated) 
			calculateDimensions();
		
		Boolean isOver = (x >= left && x <= (left + width)) &&
		       			 (y >= top && y <= (top + height));

		//Log.d("DragZone", "x=" +x + ", left=" + left + ", y=" + y + ", top=" + top + " width=" + width + ", height=" + height + ", isover=" + isOver);

		return isOver;
	}
	
	// =========================================
	// Protected & Private methods
	// =========================================
	
	protected DropZoneEventsListener getListener() {
		return listener;
	}
	
	private void calculateDimensions() {
		int[] location = new int[2];
		view.getLocationOnScreen(location);
		left = location[0];
		top = location[1];
		width = view.getWidth();
		height = view.getHeight();
		dimansionsCalculated = true;
	}
}
