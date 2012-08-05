package com.MobileAnarchy.Android.Widgets.DragAndDrop;

import android.content.Context;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.MotionEvent;
import android.widget.FrameLayout;

public class DragSurface extends FrameLayout {

	private float draggedViewHalfHeight;
	private float draggedViewHalfWidth;
	private int framesCount;
	
	private Boolean isDragging;
	private DraggableItem draggedItem;
	
	public DragSurface(Context context, AttributeSet attrs) {
		super(context, attrs);
		isDragging = false;
	}

	// =========================================
	// Touch Events Listener
	// =========================================

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		if (isDragging && event.getAction() == MotionEvent.ACTION_UP) {
			// Dragging ended
			removeAllViews();
			isDragging = false;
			
			DragAndDropManager.getInstance().stoppedDragging();
		}

		if (isDragging && event.getAction() == MotionEvent.ACTION_MOVE) {
			// Move the dragged view to it's new position
			repositionView(event.getX(), event.getY());
			
			// Mark this event as handled (so that other UI elements will not intercept it)
			return true;
		}
		
		return false;
	}

	public void startDragging(DraggableItem draggableItem) {
		this.draggedItem = draggableItem;
		this.draggedItem.getDraggedView().setVisibility(INVISIBLE);
		isDragging = true;
		addView(this.draggedItem.getDraggedView());
		//repositionView(x, y);
		framesCount = 0;
	}

	private void repositionView(float x, float y) {
		draggedViewHalfHeight = draggedItem.getDraggedView().getHeight() / 2f;
		draggedViewHalfWidth = draggedItem.getDraggedView().getWidth() / 2f;
		
		// If the dragged view was not drawn yet, skip this phase 
		if (draggedViewHalfHeight == 0 || draggedViewHalfWidth == 0)
			return;
		
		framesCount++;
		
		//Log.d(TAG, "Original = (x=" + x + ", y=" + y + ")");
		//Log.d(TAG, "Size (W=" + draggedViewHalfWidth + ", H=" + draggedViewHalfHeight + ")");

		x = x - draggedViewHalfWidth;
		y = y - draggedViewHalfHeight;
		
		x = Math.max(x, 0);
		x = Math.min(x, getWidth() - draggedViewHalfWidth * 2);

		y = Math.max(y, 0);
		y = Math.min(y, getHeight() - draggedViewHalfHeight * 2);

		//Log.d(TAG, "Moving view to (x=" + x + ", y=" + y + ")");
		FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(LayoutParams.WRAP_CONTENT, 
				LayoutParams.WRAP_CONTENT, Gravity.TOP + Gravity.LEFT);
		
		lp.setMargins((int)x, (int)y, 0, 0);
		draggedItem.getDraggedView().setLayoutParams(lp);

		// hte first couple of dragged frame's positions are not calculated correctly,
		// so we have a threshold before making the dragged view visible again
		if (framesCount < 2)
			return;

		draggedItem.getDraggedView().setVisibility(VISIBLE);
	}
	
}
