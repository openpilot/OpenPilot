package org.openpilot.androidgcs;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class HomePage extends ObjectManagerActivity {
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);		
		setContentView(R.layout.gcs_home);		
		
		Button objectBrowser = (Button) findViewById(R.id.launch_object_browser);
		objectBrowser.setOnClickListener(new OnClickListener() {
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, ObjectBrowser.class));	
			}			
		});
		
		Button pfd = (Button) findViewById(R.id.launch_pfd);
		pfd.setOnClickListener(new OnClickListener() {
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, PFD.class));	
			}			
		});

		Button location = (Button) findViewById(R.id.launch_location);
		location.setOnClickListener(new OnClickListener() {
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, UAVLocation.class));	
			}			
		});

		Button controller = (Button) findViewById(R.id.launch_controller);
		controller.setOnClickListener(new OnClickListener() {
			public void onClick(View arg0) {
				startActivity(new Intent(HomePage.this, Controller.class));	
			}			
		});

	}

}
