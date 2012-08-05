package com.MobileAnarchy.Android.Widgets.TilesLayout;


public class TilePosition {

	// =========================================
	// Private Members
	// =========================================

	private float x, y, height, width;
	
	// =========================================
	// Constructors
	// =========================================

	public TilePosition(float x, float y, float width, float height) {
		this.x = x;
		this.y = y;
		this.height = height;
		this.width = width;
	}

	// =========================================
	// Public Properties
	// =========================================

	public float getX() {
		return x;
	}

	public float getY() {
		return y;
	}

	public float getHeight() {
		return height;
	}

	public float getWidth() {
		return width;
	}

	
	// =========================================
	// Public Methods
	// =========================================

	public Boolean equals(TilePosition position) {
		if (position == null) 
			return false;
		
		return this.x == position.x &&
		       this.y == position.y &&
		       this.height == position.height &&
		       this.width == position.width;
	}
	
	@Override
	public String toString() {
		return "TilePosition = [X: " + x + ", Y: " + y + ", Height: " + height + ", Width: " + width + "]";
	}
	
}
