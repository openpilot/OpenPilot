package com.MobileAnarchy.Android.Widgets.TilesLayout;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.widget.FrameLayout;

public class SingleTileLayout extends FrameLayout {

	// =========================================
	// Private members
	// =========================================

	private TilePosition position;
	private long timestamp;
	
	// =========================================
	// Constructors
	// =========================================

	public SingleTileLayout(Context context) {
		super(context);
	}

	public SingleTileLayout(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	
	// =========================================
	// Public Methods
	// =========================================

	public TilePosition getPosition() {
		return position;
	}

	public void setPosition(TilePosition position) {
		this.position = position;
	}
	
	public long getTimestamp() {
		return this.timestamp;
	}
	
	// =========================================
	// Overrides
	// =========================================

	@Override
	public void addView(View child) {
		super.addView(child);
		timestamp = java.lang.System.currentTimeMillis();
	}

	@Override
	public void removeAllViews() {
		super.removeAllViews();
		timestamp = 0;
	}

	@Override
	public void removeView(View view) {
		super.removeView(view);
		timestamp = 0;
	}
	
}
