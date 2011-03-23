package org.openpilot.androidgcs;

import java.util.List;
import java.util.Observable;
import java.util.Observer;

import org.openpilot.androidgcs.OPTelemetryService.LocalBinder;
import org.openpilot.androidgcs.OPTelemetryService.TelemTask;
import org.openpilot.uavtalk.UAVDataObject;
import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.UAVObjectManager;

import com.google.android.maps.GeoPoint;
import com.google.android.maps.MapActivity; 
import com.google.android.maps.MapController; 
import com.google.android.maps.MapView; 
import com.google.android.maps.Overlay;
import com.google.android.maps.Projection;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.RectF;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

public class UAVLocation extends MapActivity 
{ 
	private final String TAG = "UAVLocation";
	private static int LOGLEVEL = 0;
//	private static boolean WARN = LOGLEVEL > 1;
	private static boolean DEBUG = LOGLEVEL > 0;

	private MapView mapView; 
	private MapController mapController;

	UAVObjectManager objMngr;
    boolean mBound = false;
    boolean mConnected = false;
    LocalBinder binder;

    GeoPoint homeLocation;
    GeoPoint uavLocation;

    @Override public void onCreate(Bundle icicle) {
		super.onCreate(icicle); 
		setContentView(R.layout.map_layout); 
		mapView = (MapView)findViewById(R.id.map_view);
		mapController = mapView.getController();

		mapView.displayZoomControls(true);
		Double lat = 37.422006*1E6; 
		Double lng = -122.084095*1E6; 
		homeLocation = new GeoPoint(lat.intValue(), lng.intValue());
		uavLocation = homeLocation;
		mapController.setCenter(homeLocation); 
		mapController.setZoom(18);
		
		List<Overlay> overlays = mapView.getOverlays();
		UAVOverlay myOverlay = new UAVOverlay(); 
		overlays.add(myOverlay); 
		mapView.postInvalidate();

		// ObjectManager related stuff (can't inherit standard class)
		BroadcastReceiver connectedReceiver = new BroadcastReceiver() {
			@Override
			public void onReceive(Context context, Intent intent) {
				Log.d(TAG, "Received intent");
				TelemTask task;
				if(intent.getAction().compareTo(OPTelemetryService.INTENT_ACTION_CONNECTED) == 0) {
					
					if(binder  == null)
						return;
					if((task = binder.getTelemTask(0)) == null)
						return;
					objMngr = task.getObjectManager();
					mConnected = true;
					onOPConnected();
					Log.d(TAG, "Connected()");
				} else if (intent.getAction().compareTo(OPTelemetryService.INTENT_ACTION_DISCONNECTED) == 0) {
					objMngr = null;
					mConnected = false;
					onOPDisconnected();
					Log.d(TAG, "Disonnected()");
				}
			}			
		};
		
		IntentFilter filter = new IntentFilter();
		filter.addCategory(OPTelemetryService.INTENT_CATEGORY_GCS);
		filter.addAction(OPTelemetryService.INTENT_ACTION_CONNECTED);
		filter.addAction(OPTelemetryService.INTENT_ACTION_DISCONNECTED);
		registerReceiver(connectedReceiver, filter);
	}

	//@Override 
	protected boolean isRouteDisplayed() {
		// IMPORTANT: This method must return true if your Activity // is displaying driving directions. Otherwise return false. 
		return false;
	}

	public class UAVOverlay extends Overlay {
		@Override 
		public void draw(Canvas canvas, MapView mapView, boolean shadow) {
			
			Projection projection = mapView.getProjection();
			
			if (shadow == false) { 
				Point myPoint = new Point(); 
				projection.toPixels(uavLocation, myPoint);
				
				//// Draw UAV
				// Create and setup your paint brush 
				Paint paint = new Paint(); 
				paint.setARGB(250, 255, 0, 0); 
				paint.setAntiAlias(true); 
				paint.setFakeBoldText(true);
				
				// Create the circle 
				int rad = 5; 
				RectF oval = new RectF(myPoint.x-rad, myPoint.y-rad, myPoint.x+rad, myPoint.y+rad);
				
				// Draw on the canvas 
				canvas.drawOval(oval, paint); 
				canvas.drawText("UAV", myPoint.x+rad, myPoint.y, paint);								

				//// Draw Home
				myPoint = new Point();
				projection.toPixels(homeLocation, myPoint);

				// Create and setup your paint brush 
				paint.setARGB(250, 0, 0, 0); 
				paint.setAntiAlias(true); 
				paint.setFakeBoldText(true);
				
				// Create the circle 
				rad = 5; 
				oval = new RectF(myPoint.x-rad, myPoint.y-rad, myPoint.x+rad, myPoint.y+rad);
				
				// Draw on the canvas 
				canvas.drawOval(oval, paint); 
				canvas.drawText("Home", myPoint.x+rad, myPoint.y, paint);								

			} 
		}

		@Override
		public boolean onTap(GeoPoint point, MapView mapView1) {
			// Return true if screen tap is handled by this overlay
			return false; 
		}
	}
	
	void onOPConnected() {
		UAVObject obj = objMngr.getObject("HomeLocation");
		if(obj != null)
			obj.addUpdatedObserver(new Observer() {
				public void update(Observable observable, Object data) {
					UAVDataObject obj = (UAVDataObject) data;
					Double lat = obj.getField("Latitude").getDouble() / 10;
					Double lon = obj.getField("Longitude").getDouble() / 10;
					homeLocation = new GeoPoint(lat.intValue(), lon.intValue());
					runOnUiThread(new Runnable() {
						public void run() {
							mapController.setCenter(homeLocation);							
						}						
					});
					System.out.println("HomeLocation: " + homeLocation.toString());
				}				
			});
		// Hacky - trigger an update
		obj.updated();
		
		obj = objMngr.getObject("PositionActual");
		if(obj != null)
			obj.addUpdatedObserver(new Observer() {
				public void update(Observable observable, Object data) {
					UAVDataObject obj = (UAVDataObject) data;
					Double north = obj.getField("North").getDouble();
					Double east = obj.getField("East").getDouble();
					// TODO: Correct convertion from NED to LLA.  This is erroneous conversion from cm to deg
					uavLocation = new GeoPoint((int) (homeLocation.getLatitudeE6() + north / 100 * 1e6 / 78847),
							(int) (homeLocation.getLongitudeE6() + east / 100 * 1e6 / 78847));
					runOnUiThread(new Runnable() {
						public void run() {
							mapView.invalidate();
						}
					});
				}				
			});

	}
	
	void onOPDisconnected() {
		
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {
		case R.id.menu_connect:
			binder.openConnection();
			return true;
		case R.id.menu_disconnect:
			binder.stopConnection();
			return true;
		case R.id.menu_settings:
			startActivity(new Intent(this, Preferences.class));
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.options_menu, menu);
		return true;
	}

	@Override
	public void onStart() {
		super.onStart();
		Intent intent = new Intent(this, OPTelemetryService.class);
		bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
	}
	
	public void onBind() {
		
	}

	/** Defines callbacks for service binding, passed to bindService() */
	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName arg0, IBinder service) {
			// We've bound to LocalService, cast the IBinder and attempt to open a connection			
			if (DEBUG) Log.d(TAG,"Service bound");
			mBound = true;
			binder = (LocalBinder) service;
			
			if(binder.isConnected()) {
				TelemTask task;
				if((task = binder.getTelemTask(0)) != null) {
					objMngr = task.getObjectManager();
					mConnected = true;
					onOPConnected();
				}
			
			}
		}

		public void onServiceDisconnected(ComponentName name) {
			mBound = false;
			binder = null;
			mConnected = false;
			objMngr = null;
			objMngr = null;
			mConnected = false;
			onOPDisconnected();
		}
	};
}
