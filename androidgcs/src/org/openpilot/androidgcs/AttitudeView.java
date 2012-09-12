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

    /**
     * @see android.view.View#measure(int, int)
     */
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        setMeasuredDimension(measureWidth(widthMeasureSpec),
                measureHeight(heightMeasureSpec));
    }


    /**
     * Determines the height of this view
     * @param measureSpec A measureSpec packed into an int
     * @return The height of the view, honoring constraints from measureSpec
     */
    private int measureHeight(int measureSpec) {
        int result = 0;
        int specMode = MeasureSpec.getMode(measureSpec);
        int specSize = MeasureSpec.getSize(measureSpec);

        if (specMode == MeasureSpec.EXACTLY) {
            // We were told how big to be
            result = specSize;
        } else {
            // Measure the text (beware: ascent is a negative number)
            result = 1600;
            if (specMode == MeasureSpec.AT_MOST) {
                // Respect AT_MOST value if that was what is called for by measureSpec
                result = Math.min(result, specSize);
            }
        }
        return result;
    }

    /**
     * Determines the width of this view
     * @param measureSpec A measureSpec packed into an int
     * @return The width of the view, honoring constraints from measureSpec
     */
    private int measureWidth(int measureSpec) {
        int result = 0;
        int specMode = MeasureSpec.getMode(measureSpec);
        int specSize = MeasureSpec.getSize(measureSpec);

        if (specMode == MeasureSpec.EXACTLY) {
            // We were told how big to be
            result = specSize;
        } else {
            // Measure the text
            result = 800;
            if (specMode == MeasureSpec.AT_MOST) {
                // Respect AT_MOST value if that was what is called for by measureSpec
                result = Math.min(result, specSize);
            }
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

		final int PX = getMeasuredWidth();
		final int PY = getMeasuredHeight();

		// Want 60 deg to move the horizon all the way off the screen
		final int DEG_TO_PX = (PY/2) / 60; // Magic number for how to scale pitch

		canvas.save();
		canvas.rotate(-roll, PX / 2, PY / 2);
		canvas.save();

		canvas.translate(0, pitch * DEG_TO_PX);
		Drawable horizon = getContext().getResources().getDrawable(
				R.drawable.im_pfd_horizon);
		Drawable reticule = getContext().getResources().getDrawable(
				R.drawable.im_pfd_reticule);
		Drawable fixed = getContext().getResources().getDrawable(
				R.drawable.im_pfd_fixed);

		// Starting with a square image, want to size it equally
		double margin = 0.2;
		int screenSize = Math.min(PX, PY);
		int imageHalfSize = (int) ((screenSize + screenSize * margin) / 2);

		// This puts the image at the center of the PFD canvas (after it was
		// translated)
		horizon.setBounds( PX/2 - imageHalfSize, PY/2 - imageHalfSize, PX/2 + imageHalfSize, PY/2 + imageHalfSize);
		horizon.draw(canvas);
		canvas.restore();

		// Draw the overlay that only rolls
		reticule.setBounds( PX/2 - imageHalfSize, PY/2 - imageHalfSize, PX/2 + imageHalfSize, PY/2 + imageHalfSize);
		reticule.draw(canvas);
		canvas.restore();

		// Draw the overlay that never moves
		fixed.setBounds( PX/2 - imageHalfSize, PY/2 - imageHalfSize, PX/2 + imageHalfSize, PY/2 + imageHalfSize);
		fixed.draw(canvas);
}

}
