package com.MobileAnarchy.Android.Widgets.DragAndDrop;

import java.util.ArrayList;

import android.content.Context;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;

public class DragAndDropManager {

	// =========================================
	// Private members
	// =========================================

	protected static final String TAG = "DragAndDropManager";
	private static DragAndDropManager instance = null;
	private ArrayList<DropZone> dropZones;
	private OnTouchListener originalTouchListener;
	private DragSurface dragSurface;
	private DraggableItem draggedItem;
	private DropZone activeDropZone;

	// =========================================
	// Protected Constructor
	// =========================================

	protected DragAndDropManager() {
		// Exists only to defeat instantiation.
		dropZones = new ArrayList<DropZone>();
	}

	// =========================================
	// Public Properties
	// =========================================

	public static DragAndDropManager getInstance() {
		if (instance == null) {
			instance = new DragAndDropManager();
		}
		return instance;
	}

	public Context getContext() {
		if (dragSurface == null) 
			return null;
		
		return dragSurface.getContext();
	}
	
	// =========================================
	// Public Methods
	// =========================================

	public void init(DragSurface surface) {
		dragSurface = surface;
		clearDropZones();
	}
	
	public void clearDropZones() {
		dropZones.clear();
	}
	
	public void addDropZone(DropZone dropZone) {
		dropZones.add(dropZone);
	}

	
	public void startDragging(OnTouchListener originalListener, DraggableItem draggedItem) {
		originalTouchListener = originalListener;
		this.draggedItem = draggedItem;
		draggedItem.getSource().setOnTouchListener(new OnTouchListener() {
			
			@Override
			public boolean onTouch(View v, MotionEvent event) {
				int[] location = new int[2]; 
				v.getLocationOnScreen(location);
				event.offsetLocation(location[0], location[1]);
				invalidateDropZones((int)event.getX(), (int)event.getY());
				return dragSurface.onTouchEvent(event);
			}
		});

		dragSurface.startDragging(draggedItem);
	}
	

	// =========================================
	// Protected Methods
	// =========================================

	protected void invalidateDropZones(int x, int y) {
		if (activeDropZone != null) {
			if (!activeDropZone.isOver(x, y)) {
				activeDropZone.getListener().OnDragZoneLeft(activeDropZone, draggedItem);
				activeDropZone = null;
			}
			else {
				// we are still over the same drop zone, no need to check other drop zones
				return;
			}
		}
		
		for (DropZone dropZone : dropZones) {
			if (dropZone.isOver(x, y)) {
				activeDropZone = dropZone;
				dropZone.getListener().OnDragZoneEntered(activeDropZone, draggedItem);
				break;
			}
		}
	}
	
	protected void stoppedDragging() {
		if (activeDropZone != null) {
			activeDropZone.getListener().OnDropped(activeDropZone, draggedItem);
		}
		
		// Registering the "old" listener to the view that initiated this drag session
		draggedItem.getSource().setOnTouchListener(originalTouchListener);
		draggedItem = null;
		activeDropZone = null;
	}
	
}
