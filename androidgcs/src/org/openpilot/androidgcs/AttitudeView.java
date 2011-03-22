package org.openpilot.androidgcs;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

public class AttitudeView extends View {

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

		circlePaint = new Paint(Paint.ANTI_ALIAS_FLAG); 
		circlePaint.setColor(R.color.background_color); 
		circlePaint.setStrokeWidth(1); 
		circlePaint.setStyle(Paint.Style.FILL_AND_STROKE);
		Resources r = this.getResources(); 
		textPaint = new Paint(Paint.ANTI_ALIAS_FLAG); 
		textPaint.setColor(r.getColor(R.color.text_color));
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

	private double roll;
	public void setRoll(double roll) { 
		this.roll = roll;
	} 
	private double pitch;
	public void setPitch(double d) { 
		this.pitch = d;
	} 

	// Drawing related code
	private Paint markerPaint; 
	private Paint textPaint; 
	private Paint circlePaint; 

	@Override
	protected void onDraw(Canvas canvas) {
		int px = getMeasuredWidth() / 2; 
		int py = getMeasuredHeight() /2 ;
		int radius = Math.min(px, py);
		
		canvas.drawLine(px,py, (int) (px+radius * Math.cos(roll)), (int) (py + radius * Math.sin(roll)), markerPaint);		
		canvas.drawLine(px,py, (int) (px+radius * Math.cos(pitch)), (int) (py + radius * Math.sin(pitch)), markerPaint);
	}

}
