/**
 ******************************************************************************
 * @file       AttitudeView.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      A view for UAV attitude.
 * @see        The GNU Public License (GPL) Version 3
 *
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
package org.openpilot.androidgcs;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.View;

public class AttitudeView extends View {

	private Paint markerPaint;
	public AttitudeView(Context context) {
		super(context);
		initAttitudeView();
	}

	public AttitudeView(Context context, AttributeSet ats, int defaultStyle) {
		super(context, ats, defaultStyle);
		initAttitudeView();
	}

	public AttitudeView(Context context, AttributeSet ats) {
		super(context, ats);
		initAttitudeView();
	}

	protected void initAttitudeView() {
		setFocusable(true);
		markerPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		markerPaint.setColor(getContext().getResources().getColor(
				R.color.marker_color));

	}

	@Override protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		int measuredWidth = measure(widthMeasureSpec);
		int measuredHeight = measure(heightMeasureSpec);
		int d = Math.min(measuredWidth, measuredHeight);
		setMeasuredDimension(d/2, d/2);
	}

	private int measure(int measureSpec) {
		int result = 0;
		// Decode the measurement specifications.

		int specMode = MeasureSpec.getMode(measureSpec);
		int specSize = MeasureSpec.getSize(measureSpec);

		if (specMode == MeasureSpec.UNSPECIFIED) { // Return a default size of 200 if no bounds are specified.
			result = 200;
		} else {
			// As you want to fill the available space
			// always return the full available bounds.
			result = specSize;
		}
		return result;
	}

	private float roll;
	public void setRoll(double roll) {
		this.roll = (float) roll;
	}

	private float pitch;
	public void setPitch(double d) {
		this.pitch = (float) d;
	}

	@Override
	protected void onDraw(Canvas canvas) {

		final int PX = getMeasuredWidth() / 2;
		final int PY = getMeasuredHeight() / 2;

		// TODO: Figure out why these magic numbers are needed to center it
		final int WIDTH = 600;
		final int HEIGHT = 600;
		final int DEG_TO_PX = 10; // Magic number for how to scale pitch

		canvas.save(0);
		canvas.rotate(-roll, PX, PY);
		canvas.translate(0, pitch * DEG_TO_PX);
		Drawable horizon = getContext().getResources().getDrawable(
				R.drawable.im_pfd_horizon);
		// This puts the image at the center of the PFD canvas (after it was
		// translated)
		horizon.setBounds(-(WIDTH - PX) / 2, -(HEIGHT - PY) / 2, WIDTH, HEIGHT);
		horizon.draw(canvas);
		canvas.restore();

	}

}
