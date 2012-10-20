/**
 ******************************************************************************
 * @file       CompassView.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      A view of the compass heading.
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
import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

public class CompassView extends View {

	public CompassView(Context context) {
		super(context);
		initCompassView();
	}

	public CompassView(Context context, AttributeSet ats, int defaultStyle) {
		super(context, ats, defaultStyle);
		initCompassView();
	}

	public CompassView(Context context, AttributeSet ats) {
		super(context, ats);
		initCompassView();
	}

	protected void initCompassView() {
		setFocusable(true);

		circlePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		circlePaint.setColor(getResources().getColor(R.color.background_color));
		circlePaint.setStrokeWidth(1);
		circlePaint.setStyle(Paint.Style.FILL_AND_STROKE);
		Resources r = this.getResources();
		northString = r.getString(R.string.cardinal_north);
		eastString = r.getString(R.string.cardinal_east);
		southString = r.getString(R.string.cardinal_south);
		westString = r.getString(R.string.cardinal_west);
		textPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		textPaint.setColor(r.getColor(R.color.text_color));
		textHeight = (int)textPaint.measureText("yY");
		markerPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		markerPaint.setColor(r.getColor(R.color.marker_color));
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

	private double bearing;
	public void setBearing(double bearing) {
		this.bearing = bearing;
	}

	// Drawing related code
	private Paint markerPaint;
	private Paint textPaint;
	private Paint circlePaint;
	private String northString;
	private String eastString;
	private String southString;
	private String westString;
	private int textHeight;

	@Override
	protected void onDraw(Canvas canvas) {
		int px = getMeasuredWidth() / 2;
		int py = getMeasuredHeight() /2 ;
		int radius = Math.min(px, py);
		// Draw the background
		canvas.drawCircle(px, py, radius, circlePaint);

		// Rotate our perspective so that the "top" is
		// facing the current bearing.
		canvas.save();
		canvas.rotate((float) -bearing, px, py);

		int textWidth = (int)textPaint.measureText("W");
		int cardinalX = px-textWidth/2;
		int cardinalY = py-radius+textHeight;

		// Draw the marker every 15 degrees and text every 45.
		for (int i = 0; i < 24; i++) {
			// Draw a marker.
			canvas.drawLine(px, py-radius, px, py-radius+10, markerPaint);
			canvas.save();
			canvas.translate(0, textHeight);
			// Draw the cardinal points
			if (i % 6 == 0) {
				String dirString = null;
				switch (i) {
				case 0 : {
					dirString = northString;
					int arrowY = 2*textHeight;
					canvas.drawLine(px, arrowY, px-5, 3*textHeight, markerPaint);
					canvas.drawLine(px, arrowY, px+5, 3*textHeight, markerPaint);
					break;
				}
				case 6: dirString = eastString; break;
				case 12: dirString = southString; break;
				case 18: dirString = westString; break;
				}
				canvas.drawText(dirString, cardinalX, cardinalY, textPaint);
			}
			else if (i % 3 == 0) {
				// Draw the text every alternate 45deg
				String angle = String.valueOf(i*15);
				float angleTextWidth = textPaint.measureText(angle);

				int angleTextX = (int)(px-angleTextWidth/2);
				int angleTextY = py-radius+textHeight;
				canvas.drawText(angle, angleTextX, angleTextY, textPaint);
			}
			canvas.restore();
			canvas.rotate(15, px, py);
		}
		canvas.restore();
	}
}
