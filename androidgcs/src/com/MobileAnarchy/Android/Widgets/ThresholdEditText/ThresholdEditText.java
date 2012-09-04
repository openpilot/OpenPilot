package com.MobileAnarchy.Android.Widgets.ThresholdEditText;

import android.content.Context;
import android.os.Handler;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.widget.EditText;

public class ThresholdEditText extends EditText {

	// =========================================
	// Private members
	// =========================================

	private int threshold;
	private ThresholdTextChanged thresholdTextChanged;
	private Handler handler;
	private Runnable invoker;
	private boolean thresholdDisabledOnEmptyInput;

	
	// =========================================
	// Constructors
	// =========================================

	public ThresholdEditText(Context context) {
		super(context);
		initAttributes(null);
		init();
	}

	public ThresholdEditText(Context context, AttributeSet attrs) {
		super(context, attrs);
		initAttributes(attrs);
		init();
	}

	
	// =========================================
	// Public properties
	// =========================================

	/**
	 * Get the current threshold value
	 */
	public int getThreshold() {
		return threshold;
	}

	/**
	 * Set the threshold value (in milliseconds)
	 * 
	 * @param threshold
	 *            Threshold value
	 */
	public void setThreshold(int threshold) {
		this.threshold = threshold;
	}

	/**
	 * @return True = the callback will fire immediately when the content of the
	 *         EditText is emptied False = The threshold will be used even on
	 *         empty input
	 */
	public boolean getThresholdDisabledOnEmptyInput() {
		return thresholdDisabledOnEmptyInput;
	}

	/**
	 * @param thresholdDisabledOnEmptyInput
	 *            Set to true if you want the callback to fire immediately when
	 *            the content of the EditText is emptied
	 */
	public void setThresholdDisabledOnEmptyInput(
			boolean thresholdDisabledOnEmptyInput) {
		this.thresholdDisabledOnEmptyInput = thresholdDisabledOnEmptyInput;
	}

	/**
	 * Set the callback to the OnThresholdTextChanged event
	 * 
	 * @param listener
	 */
	public void setOnThresholdTextChanged(ThresholdTextChanged listener) {
		this.thresholdTextChanged = listener;
	}

	// =========================================
	// Private / Protected methods
	// =========================================

	/**
	 * Load properties values from xml layout
	 */
	private void initAttributes(AttributeSet attrs) {
		if (attrs != null) {
			String namespace = "http://com.MobileAnarchy.Android.Widgets";

			// Load values to local members
			this.threshold = attrs.getAttributeIntValue(namespace, "threshold",
					500);
			this.thresholdDisabledOnEmptyInput = attrs.getAttributeBooleanValue(
					namespace, "disableThresholdOnEmptyInput", true);
		} else {
			// Default threshold value is 0.5 seconds
			threshold = 500;

			// Default behaviour on emptied text - no threshold
			thresholdDisabledOnEmptyInput = true;
		}
	}

	/**
	 * Initialize the private members with default values
	 */
	private void init() {

		handler = new Handler();

		invoker = new Runnable() {

			@Override
			public void run() {
				invokeCallback();
			}

		};

		this.addTextChangedListener(new TextWatcher() {

			@Override
			public void afterTextChanged(Editable s) {
			}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count,
					int after) {
			}

			@Override
			public void onTextChanged(CharSequence s, int start, int before,
					int count) {

				// Remove any existing pending callbacks
				handler.removeCallbacks(invoker);

				if (s.length() == 0 && thresholdDisabledOnEmptyInput) {
					// The text is empty, so invoke the callback immediately
					invoker.run();
				} else {
					// Post a new delayed callback
					handler.postDelayed(invoker, threshold);
				}
			}

		});
	}

	/**
	 * Invoking the callback on the listener provided (if provided)
	 */
	private void invokeCallback() {
		if (thresholdTextChanged != null) {
			thresholdTextChanged.onThersholdTextChanged(this.getText());
		}
	}

}