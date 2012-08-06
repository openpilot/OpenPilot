package com.MobileAnarchy.Android.Widgets.TilesLayout;

import java.util.LinkedList;
import java.util.List;

/**
 * Describes the positioning of a tiles in a 100x100 environment
 */
public class TilesLayoutPreset {

	// =========================================
	// Private Members
	// =========================================

	private List<TilePosition> _positions;

	// =========================================
	// Constructors
	// =========================================

	public TilesLayoutPreset(String name) {
		_positions = new LinkedList<TilePosition>();
	}
	
	// =========================================
	// Public Methods
	// =========================================

	public void add(float x, float y, float width, float height) {
		TilePosition tilePosition = new TilePosition(x, y, height, width);
		add(tilePosition);
	}

	public void add(TilePosition tilePosition) {
		_positions.add(tilePosition);
	}
	
	public Iterable<TilePosition> getTilePositions() {
		return _positions;
	}

	public int getCount() {
		return _positions.size();
	}

	// =========================================
	// Static Presets Factories
	// =========================================

	public static TilesLayoutPreset get1x1() {
		TilesLayoutPreset preset = new TilesLayoutPreset("Default 1X1");
		preset.add(0, 0, 100, 100);
		return preset;
	}

	
	public static TilesLayoutPreset get1x2() {
		TilesLayoutPreset preset = new TilesLayoutPreset("Default 1X2");
		preset.add(0, 0, 50, 100);
		preset.add(0, 50, 50, 100);
		return preset;
	}

	
	public static TilesLayoutPreset get2x1() {
		TilesLayoutPreset preset = new TilesLayoutPreset("Default 2X1");
		preset.add(0, 0, 100, 50);
		preset.add(50, 0, 100, 50);
		return preset;
	}

	
	public static TilesLayoutPreset get2x2() {
		TilesLayoutPreset preset = new TilesLayoutPreset("Default 2X2");
		preset.add(0, 0, 50, 50);
		preset.add(50, 0, 50, 50);
		preset.add(0, 50, 50, 50);
		preset.add(50, 50, 50, 50);
		return preset;
	}

	public static TilesLayoutPreset get3x3() {
		TilesLayoutPreset preset = new TilesLayoutPreset("Default 3X3");
		preset.add(0, 0, 100/3f, 100/3f);
		preset.add(100/3f, 0, 100/3f, 100/3f);
		preset.add(200/3f, 0, 100/3f, 100/3f);
		preset.add(0, 100/3f, 100/3f, 100/3f);
		preset.add(100/3f, 100/3f, 100/3f, 100/3f);
		preset.add(200/3f, 100/3f, 100/3f, 100/3f);
		preset.add(0, 200/3f, 100/3f, 100/3f);
		preset.add(100/3f, 200/3f, 100/3f, 100/3f);
		preset.add(200/3f, 200/3f, 100/3f, 100/3f);
		return preset;
	}

	public static TilesLayoutPreset get3x2() {
		TilesLayoutPreset preset = new TilesLayoutPreset("Default 3X2");
		preset.add(0, 0, 50, 100/3f);
		preset.add(100/3f, 0, 50, 100/3f);
		preset.add(200/3f, 0, 50, 100/3f);
		preset.add(0, 50, 50, 100/3f);
		preset.add(100/3f, 50, 50, 100/3f);
		preset.add(200/3f, 50, 50, 100/3f);
		return preset;
	}

	public static TilesLayoutPreset get2x3() {
		TilesLayoutPreset preset = new TilesLayoutPreset("Default 2X3");
		preset.add(0, 0, 100/3f, 50);
		preset.add(50, 0, 100/3f, 50);
		preset.add(0, 100/3f, 100/3f, 50);
		preset.add(50, 100/3f, 100/3f, 50);
		preset.add(0, 200/3f, 100/3f, 50);
		preset.add(50, 200/3f, 100/3f, 50);
		return preset;
	}

	
	public static TilesLayoutPreset get4x4() {
		TilesLayoutPreset preset = new TilesLayoutPreset("Default 2X2");
		preset.add(0, 0, 25, 25);
		preset.add(25, 0, 25, 25);
		preset.add(50, 0, 25, 25);
		preset.add(75, 0, 25, 25);
		preset.add(0, 25, 25, 25);
		preset.add(25, 25, 25, 25);
		preset.add(50, 25, 25, 25);
		preset.add(75, 25, 25, 25);
		preset.add(0, 50, 25, 25);
		preset.add(25, 50, 25, 25);
		preset.add(50, 50, 25, 25);
		preset.add(75, 50, 25, 25);
		preset.add(0, 75, 25, 25);
		preset.add(25, 75, 25, 25);
		preset.add(50, 75, 25, 25);
		preset.add(75, 75, 25, 25);
		return preset;
	}
	
	public static TilesLayoutPreset get2x3x3() {
		TilesLayoutPreset preset = new TilesLayoutPreset("Custom 2X4X4");
		preset.add(0, 0, 50, 50);
		preset.add(50, 0, 50, 50);
		preset.add(0, 50, 50, 50);

		preset.add(50, 50, 25, 25);
		preset.add(75, 50, 25, 25);
		preset.add(50, 75, 25, 25);
		preset.add(75, 75, 25, 25);

		return preset;
	}

	
}
