/**
 ******************************************************************************
 * @file       MapPositioner.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      A fragmenet to move the UAV around by dragging
 * @see        The GNU Public License (GPL) Version 3
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
 package org.openpilot.androidgcs.fragments;

import org.openpilot.androidgcs.R;
import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectManager;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.SeekBar;

public class MapPositioner extends ObjectManagerFragment {

	private static final String TAG = ObjectManagerFragment.class
			.getSimpleName();
	private static final int LOGLEVEL = 0;
	private static final boolean DEBUG = LOGLEVEL > 0;

	private final float MAX_DISTANCE_M = 50.0f;

	float px_to_m(View v, float px) {
		final float MAX_DISTANCE_PX = Math.max(v.getMeasuredHeight(),
				v.getMeasuredWidth());
		float scale = MAX_DISTANCE_M / MAX_DISTANCE_PX;
		return px * scale;
	}

	float m_to_px(View v, float m) {
		final float MAX_DISTANCE_PX = Math.max(v.getMeasuredHeight(),
				v.getMeasuredWidth());
		float scale = MAX_DISTANCE_M / MAX_DISTANCE_PX;
		return m / scale;
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		return inflater.inflate(R.layout.map_positioner, container, false);
	}

	private float pos_x = 0, pos_y = 0;
	private float desired_x = 0, desired_y = 0, desired_z = 0;
	//private final float poi_x = 1, poi_y = 1;
	private UAVOverlay uav = null;

	public class UAVOverlay extends View {

		Paint paint = null;

		public UAVOverlay(Context context) {
			super(context);
			paint = new Paint();
			paint.setColor(Color.WHITE);
			paint.setTextSize(20);
		}

		private void draw(Canvas canvas, int resource, float x, float y) {
			Drawable icon = getContext().getResources().getDrawable(resource);
			final int offset_x = canvas.getWidth() / 2;
			final int offset_y = canvas.getHeight() / 2;
			final int SIZE = 50 / 2;
			icon.setBounds((int) x - SIZE + offset_x, (int) y - SIZE + offset_y,
					(int) x + SIZE + offset_x, (int) y + SIZE + offset_y);
			icon.draw(canvas);

		}

		@Override
		/**
		 * Draw the UAV, the position desired, and the camera POI
		 */
		protected void onDraw(Canvas canvas) {
			draw(canvas, R.drawable.map_positioner_uav, (int) m_to_px(this, pos_x), (int) m_to_px(this, pos_y));
			draw(canvas, R.drawable.map_positioner_waypoint, (int) m_to_px(this, desired_x), (int) -m_to_px(this, desired_y));
			//draw(canvas, R.drawable.map_positioner_poi, (int) m_to_px(this, poi_x), (int) -m_to_px(this, poi_y));

			// Annotate the desired position
			canvas.drawText("(" + String.format("%.1f",desired_x) + "," + String.format("%.1f",desired_y) + "," + String.format("%.1f",desired_z) + ")",
					m_to_px(this, desired_x) + canvas.getWidth() / 2 + 25,
					-m_to_px(this, desired_y) + canvas.getHeight() / 2, paint);
		}

	};

	public void connectTouch() {
		if (DEBUG)
			Log.d(TAG, "Connected touch listener()");
		View v = getActivity().findViewById(R.id.map_view);
		if (v != null)
			v.setOnTouchListener(new View.OnTouchListener() {
				@Override
				public boolean onTouch(View v, MotionEvent event) {
					switch (event.getActionMasked()) {
					case MotionEvent.ACTION_DOWN:
					case MotionEvent.ACTION_MOVE:
						UAVObject desired = objMngr.getObject("PathDesired");
						if (desired != null) {
							if (DEBUG) Log.d(TAG, "Updating path desired");
							desired.getField("End").setDouble(px_to_m(v, (int) event.getX() - v.getMeasuredWidth() / 2), 1);
							desired.getField("End").setDouble(px_to_m(v, -(event.getY() - v.getMeasuredHeight() / 2)), 0);
							desired.updated();
							if (uav != null)
								uav.invalidate();
						}
						break;
					}
					return true;
				}
			});
		else
			if (DEBUG)
				Log.d(TAG, "No view");

		FrameLayout f = (FrameLayout) getActivity().findViewById(R.id.map_view);
		uav = new UAVOverlay(getActivity().getBaseContext());
		f.addView(uav);
	}

	@Override
	public void onStart() {
		super.onStart();
		if (DEBUG)
			Log.d(TAG, "onStart()");
		connectTouch();

		SeekBar altitudeBar = (SeekBar) getActivity().findViewById(R.id.altitude_slider);
		if(altitudeBar != null)
			altitudeBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

				@Override
				public void onProgressChanged(SeekBar seekBar, int progress,
						boolean fromUser) {
					UAVObject desired = objMngr.getObject("PathDesired");
					if (desired != null) {
						if (DEBUG) Log.d(TAG, "Updating path desired");
						desired.getField("End").setDouble(-progress, 2);
						desired.updated();
						if (uav != null)
							uav.invalidate();
					}

				}

				@Override
				public void onStartTrackingTouch(SeekBar seekBar) {
					// TODO Auto-generated method stub

				}

				@Override
				public void onStopTrackingTouch(SeekBar seekBar) {
					// TODO Auto-generated method stub

				}

			});
	}

	@Override
	public void onOPConnected(UAVObjectManager objMngr) {
		super.onOPConnected(objMngr);
		if (DEBUG)
			Log.d(TAG, "On connected");

		UAVObject obj = objMngr.getObject("PositionActual");
		if (obj != null)
			registerObjectUpdates(obj);
		objectUpdated(obj);

		obj = objMngr.getObject("PathDesired");
		if (obj != null)
			registerObjectUpdates(obj);
		objectUpdated(obj);
	}

	/**
	 * Called whenever any objects subscribed to via registerObjects. Store the
	 * local copy of the information.
	 */
	@Override
	public void objectUpdated(UAVObject obj) {
		if (obj.getName().compareTo("PositionActual") == 0) {
			pos_x = (int) obj.getField("East").getDouble();
			pos_y = (int) obj.getField("North").getDouble();
			uav.invalidate();
		}
		if (obj.getName().compareTo("PathDesired") == 0) {
			desired_x = (float) obj.getField("End").getDouble(1);
			desired_y = (float) obj.getField("End").getDouble(0);
			desired_z = (float) obj.getField("End").getDouble(2);
			uav.invalidate();
		}
	}


}
