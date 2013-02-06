/**
 ******************************************************************************
 * @file       OPTelemetryServiceTests.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @brief      Tests for the OPTelemetryService class
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
package org.openpilot.androidgcs.test.telemetryservice;

import org.junit.Ignore;
import org.openpilot.androidgcs.telemetry.HidUAVTalk;
import org.openpilot.androidgcs.telemetry.TcpUAVTalk;
import org.openpilot.androidgcs.telemetry.BluetoothUAVTalk;
import org.openpilot.androidgcs.telemetry.OPTelemetryService;
import org.openpilot.androidgcs.telemetry.OPTelemetryService.LocalBinder;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.test.ServiceTestCase;
import android.test.mock.MockApplication;
import android.test.mock.MockContext;

public class OPTelemetryServiceTests extends
		ServiceTestCase<OPTelemetryService> {

	private Object _syncTelemetryTaskStarted = new Object();
	private Object _syncTelemetryConnected = new Object();
	private BroadcastReceiver _connectBroadcastReceiver;

	public class MockAndroidGCSApplication extends MockApplication {

	}

	class MockAndroidGCSContext extends MockContext {

	}
	
	public OPTelemetryServiceTests() {
		this(OPTelemetryService.class);
	}
	
	public OPTelemetryServiceTests(Class<OPTelemetryService> serviceClass) {
		super(serviceClass);
		_connectBroadcastReceiver = new BroadcastReceiver(){

			@Override
			public void onReceive(Context context, Intent intent) {
				if(intent.getAction().equals(OPTelemetryService.INTENT_ACTION_CONNECTED)) {
					if(_syncTelemetryConnected != null){
						synchronized (_syncTelemetryConnected) {
							_syncTelemetryConnected.notify();
						}
					}
				}else if(intent.getAction().equals(OPTelemetryService.INTENT_ACTION_TELEMETRYTASK_STARTED)) {
					// Not looked into why _syncTelemetryTaskStarted could possibly be null
					// here but sometimes it is...
					if(_syncTelemetryTaskStarted != null){
						synchronized (_syncTelemetryTaskStarted) {
							_syncTelemetryTaskStarted.notify();
						}
					}
				}
			}
			
		};
	}

	@Override
	protected void setUp() throws Exception {
		super.setUp();
		
		IntentFilter filter = new IntentFilter();
		filter.addCategory(OPTelemetryService.INTENT_CATEGORY_GCS);
		filter.addAction(OPTelemetryService.INTENT_ACTION_CONNECTED);
		filter.addAction(OPTelemetryService.INTENT_ACTION_TELEMETRYTASK_STARTED);
		
		getContext().registerReceiver(_connectBroadcastReceiver, filter);
		
//		setApplication(new MockAndroidGCSApplication());
//		setContext(new MockAndroidGCSContext());
	}

	@Override
	protected void tearDown() throws Exception {
		super.tearDown();
//		getContext().unregisterReceiver(_connectBroadcastReceiver);
	}
	
	public void testLifecycleSingleStart(){
		bindService(new Intent(getContext(),
				org.openpilot.androidgcs.telemetry.OPTelemetryService.class));
		
		assertEquals(1, OPTelemetryService.getNumStartupRequests());
		
		shutdownService();
		
		assertEquals(0, OPTelemetryService.getNumStartupRequests());
	}
	
	public void testLifecycleMultiStart(){
		bindService(new Intent(getContext(),
				org.openpilot.androidgcs.telemetry.OPTelemetryService.class));
		bindService(new Intent(getContext(),
				org.openpilot.androidgcs.telemetry.OPTelemetryService.class));
		
		assertEquals(1, OPTelemetryService.getNumStartupRequests());
		
		shutdownService();
		
		assertEquals(0, OPTelemetryService.getNumStartupRequests());
	}

	public void testStartBluetoothTelemetryTask(){
		LocalBinder binder = startTelemetryTask(Integer.valueOf(2));
		
		assertTrue("Started wrong telemetry service type", binder.getTelemetryTask(0) instanceof BluetoothUAVTalk);
	}

	public void testStartBluetoothConnection() throws InterruptedException{
		LocalBinder binder = startTelemetryTask(Integer.valueOf(2));
		
		synchronized (_syncTelemetryConnected) {
			_syncTelemetryConnected.wait(2000);
		}
		
		assertTrue("Failed to connect to telemetry service", binder.isConnected());
		
		assertTrue("Started wrong telemetry service type", binder.getTelemetryTask(0) instanceof BluetoothUAVTalk);
	}
	
	public void testStartTCPTelemetryTask(){
		LocalBinder binder = startTelemetryTask(Integer.valueOf(3));
		
		assertTrue("Started wrong telemetry service type", binder.getTelemetryTask(0) instanceof TcpUAVTalk);
	}
	
	public void testStartUSBTelemetryTask(){
		LocalBinder binder = startTelemetryTask(Integer.valueOf(4));
		
		assertTrue("Started wrong telemetry service type", binder.getTelemetryTask(0) instanceof HidUAVTalk);		
	}

	private LocalBinder startTelemetryTask(Integer telemetryType) {
		LocalBinder binder = (LocalBinder) bindService(new Intent(getContext(),
				org.openpilot.androidgcs.telemetry.OPTelemetryService.class));
		
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getContext());
		SharedPreferences.Editor editor = prefs.edit();
		editor.putString("connection_type", telemetryType.toString());
		editor.apply();
		
		binder.openConnection();
		synchronized (_syncTelemetryTaskStarted) {
			try {
				_syncTelemetryTaskStarted.wait(2000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		return binder;
	}
}
