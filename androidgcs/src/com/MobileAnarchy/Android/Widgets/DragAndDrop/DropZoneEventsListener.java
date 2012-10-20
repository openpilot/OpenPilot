package com.MobileAnarchy.Android.Widgets.DragAndDrop;

public interface DropZoneEventsListener {

	void OnDragZoneEntered(DropZone zone, DraggableItem item);
	void OnDragZoneLeft(DropZone zone, DraggableItem item); 
	void OnDropped(DropZone zone, DraggableItem item);
	
}
