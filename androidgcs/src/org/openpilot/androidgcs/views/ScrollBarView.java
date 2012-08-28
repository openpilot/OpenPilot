package org.openpilot.androidgcs.views;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.EditText;
import android.widget.GridLayout;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

public class ScrollBarView extends GridLayout {

	private final static String TAG = ScrollBarView.class.getSimpleName();

	private final TextView lbl;
	private final EditText edit;
	private final SeekBar bar;
	private double value;

	private final double SCALE = 1000000;

	public ScrollBarView(Context context, AttributeSet attrs) {
		super(context, attrs);

		Log.d(TAG, "Scroll bar init called");

		setOrientation(LinearLayout.VERTICAL);
		setColumnCount(2);

		lbl = new TextView(context);
		lbl.setText("Roll Rate Kp:");
		addView(lbl, new GridLayout.LayoutParams(spec(0), spec(0)));

		edit = new EditText(context);
		addView(edit, new GridLayout.LayoutParams(spec(0), spec(1)));

		bar = new SeekBar(context);
		addView(bar, new GridLayout.LayoutParams(spec(1), spec(0,2)));
		bar.setMax((int) (SCALE * 0.01));

		setPadding(5,5,5,5);

		setValue(0.0035);
	}

	public void setValue(double val)
	{
		value = val;
		edit.setText(Double.toString(val));
		bar.setProgress((int) (SCALE * val));
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);

		// This shouldn't be needed if I could make this scroll bar
		// automagically span both columns
		android.view.ViewGroup.LayoutParams param = bar.getLayoutParams();
		param.width = (int) (getMeasuredWidth() * 0.9);

		// Force the label to half the page width
		param = lbl.getLayoutParams();
		param.width = getMeasuredWidth() / 2;
	}
}
