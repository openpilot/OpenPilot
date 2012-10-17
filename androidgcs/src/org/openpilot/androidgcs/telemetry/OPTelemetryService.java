/**
 ******************************************************************************
 * @file       OPTelemetryService.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Provides UAVTalk telemetry over multiple physical links.  The
 *             details of each of these are in their respective connection
 *             classes.  This mostly creates those threads based on the selected
 *             preferences.
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
package org.openpilot.androidgcs.telemetry;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import org.openpilot.uavtalk.UAVObjectManager;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.preference.PreferenceManager;
import android.util.Log;
import android.widget.Toast;
import dalvik.system.DexClassLoader;

public class OPTelemetryService extends Service {

	// Logging settings
	private final String TAG = OPTelemetryService.class.getSimpleName();
	public static int LOGLEVEL = 2;
	public static boolean DEBUG = LOGLEVEL > 1;
	public static boolean WARN = LOGLEVEL > 0;

	// Intent category
	public final static String INTENT_CATEGORY_GCS        = "org.openpilot.intent.category.GCS";

	// Intent actions
	public final static String INTENT_ACTION_CONNECTED    = "org.openpilot.intent.action.CONNECTED";
	public final static String INTENT_ACTION_DISCONNECTED = "org.openpilot.intent.action.DISCONNECTED";

	// Variables for local message handler thread
	private Looper mServiceLooper;
	private ServiceHandler mServiceHandler;
	private static HandlerThread thread;

	// Message ids
	static final int MSG_START        = 0;
	static final int MSG_CONNECT      = 1;
	static final int MSG_DISCONNECT   = 3;
	static final int MSG_TOAST        = 100;

	private Thread activeTelem;
	private TelemetryTask telemTask;

	private final IBinder mBinder = new LocalBinder();

	static class ServiceHandler extends Handler {
	    private final WeakReference<OPTelemetryService> mService;

	    ServiceHandler(OPTelemetryService service, Looper looper) {
	    	super(looper);
	        mService = new WeakReference<OPTelemetryService>(service);
	    }

	    @Override
	    public void handleMessage(Message msg)
	    {
	    	OPTelemetryService service = mService.get();
	         if (service != null) {
	              service.handleMessage(msg);
	         }
	    }
	}

	void handleMessage(Message msg) {
		switch(msg.arg1) {
		case MSG_START:
			stopSelf(msg.arg2);
			break;
		case MSG_CONNECT:
			int connection_type;
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(OPTelemetryService.this);
			try {
				connection_type = Integer.decode(prefs.getString("connection_type", ""));
			} catch (NumberFormatException e) {
				connection_type = 0;
			}

			switch(connection_type) {
			case 0: // No connection
				return;
			case 2:
				Toast.makeText(getApplicationContext(), "Attempting BT connection", Toast.LENGTH_SHORT).show();
				telemTask = new BluetoothUAVTalk(this);
				activeTelem = new Thread(telemTask, "Bluetooth telemetry thread");
				break;
			case 3:
				Toast.makeText(getApplicationContext(), "Attempting TCP connection", Toast.LENGTH_SHORT).show();
				telemTask = new TcpUAVTalk(this);
				activeTelem = new Thread(telemTask, "Tcp telemetry thread");
				break;
			case 4:
				Toast.makeText(getApplicationContext(), "Attempting USB HID connection", Toast.LENGTH_SHORT).show();
				telemTask = new HidUAVTalk(this);
				activeTelem = new Thread(telemTask, "Hid telemetry thread");
				break;
			default:
				throw new Error("Unsupported");
			}
			activeTelem.start();
			break;
		case MSG_DISCONNECT:
			Toast.makeText(getApplicationContext(), "Disconnect requested", Toast.LENGTH_SHORT).show();
			if (DEBUG) Log.d(TAG, "Calling disconnect");
			if (telemTask != null) {
				telemTask.disconnect();
				telemTask = null;

				try {
					activeTelem.join();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			else  if (activeTelem != null) {
				activeTelem.interrupt();
				try {
					activeTelem.join();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				activeTelem = null;
			}
			if (DEBUG) Log.d(TAG, "Telemetry thread terminated");
			Intent intent = new Intent();
			intent.setAction(INTENT_ACTION_DISCONNECTED);
			sendBroadcast(intent,null);

			stopSelf();

			break;
		case MSG_TOAST:
			Toast.makeText(this, (String) msg.obj, Toast.LENGTH_SHORT).show();
			break;
		default:
			System.out.println(msg.toString());
			throw new Error("Invalid message");
		}
	}

	/**
	 * Called when the service starts.  It creates a thread to handle messages (e.g. connect and disconnect)
	 * and based on the stored preference will send itself a connect signal if needed.
	 */
	public void startup() {
		Toast.makeText(getApplicationContext(), "Telemetry service starting", Toast.LENGTH_SHORT).show();

		thread = new HandlerThread("TelemetryServiceHandler", Process.THREAD_PRIORITY_BACKGROUND);
		thread.start();

		// Get the HandlerThread's Looper and use it for our Handler
		mServiceLooper = thread.getLooper();
		mServiceHandler = new ServiceHandler(this, mServiceLooper);

		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(OPTelemetryService.this);
		if(prefs.getBoolean("autoconnect", false)) {
			Message msg = mServiceHandler.obtainMessage();
			msg.arg1 = MSG_CONNECT;
			msg.arg2 = 0;
			mServiceHandler.sendMessage(msg);
		}

	}

	@Override
	public void onCreate() {
		if (DEBUG)
			Log.d(TAG, "Telemetry service created");
		startup();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		// Currently only using as bound service

		// If we get killed, after returning from here, restart
		return START_STICKY;
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	@Override
	public void onDestroy() {

		if (telemTask != null) {
			Log.d(TAG, "onDestroy() shutting down telemetry task");
			telemTask.disconnect();
			telemTask = null;

			try {
				activeTelem.join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		Log.d(TAG, "onDestory() shut down telemetry task");
		Toast.makeText(this, "Telemetry service done", Toast.LENGTH_SHORT).show();
	}

	public class LocalBinder extends Binder {
		public TelemTask getTelemTask(int id) {
			if (telemTask != null)
				return telemTask.getTelemTaskIface();
			return null;
		}
		public void openConnection() {
			Toast.makeText(getApplicationContext(), "Requested open connection", Toast.LENGTH_SHORT).show();
			Message msg = mServiceHandler.obtainMessage();
			msg.arg1 = MSG_CONNECT;
			mServiceHandler.sendMessage(msg);
		}
		public void stopConnection() {
			Message msg = mServiceHandler.obtainMessage();
			msg.arg1 = MSG_DISCONNECT;
			mServiceHandler.sendMessage(msg);
		}
		public boolean isConnected() {
			return (activeTelem != null) && (telemTask != null) && (telemTask.getConnected());
		}
	};

	public void toastMessage(String msgText) {
		Message msg = mServiceHandler.obtainMessage();
		msg.arg1 = MSG_TOAST;
		msg.obj = msgText;
		mServiceHandler.sendMessage(msg);
	}

	/**
	 * This is used by other processes to get a handle to the object manager
	 */
	public interface TelemTask {
		public UAVObjectManager getObjectManager();
	};


	/************************************************************/
	/* Everything below here has to do with getting the UAVOs   */
	/* from the package.  This shouldn't really be in the telem */
	/* service class but needs to be in this context            */
	/************************************************************/

	private static void copyStream(InputStream inputStream, OutputStream outputStream) throws IOException
    {
        byte[] buffer = new byte[1024 * 10];
        int numRead = inputStream.read(buffer);
        while (numRead > 0)
        {
            outputStream.write(buffer, 0, numRead);
            numRead = inputStream.read(buffer);
        }
    }

	private void copyAssets(String JAR_DIR, String JAR_NAME)
    {
        File jarsDir = getDir(JAR_DIR, MODE_WORLD_READABLE);
        AssetManager assetManager = getAssets();
        try
        {
            InputStream inputStream = null;
            OutputStream outputStream = null;
            try
            {
                File outputFile = new File(jarsDir, JAR_NAME);
                inputStream = assetManager.open("uavos/" + JAR_NAME);
                outputStream = new FileOutputStream(outputFile);
                copyStream(inputStream, outputStream);
            }
            finally
            {
                if (inputStream != null)
                    inputStream.close();
                if (outputStream != null)
                    outputStream.close();
            }
        }
        catch (IOException e)
        {
            Log.e(TAG, e.toString(), e);
            String[] list;
			try {
				list = assetManager.list("uavos/");
				Log.i(TAG, "Listing found uavos");
	            for(int i = 0; i < list.length; i++) {
	            	Log.i(TAG, "Found: " + list[i]);
	            }

			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}

        }
    }

	/**
	 * Delete the files in a directories
	 * @param directory
	 */
	private static void deleteDirectoryContents(File directory)
	{
		File contents[] = directory.listFiles();
		if (contents != null)
		{
			for (File file : contents)
			{
				if (file.isDirectory())
					deleteDirectoryContents(file);

				file.delete();
			}
		}
	}

	/**
	 * Load the UAVObjects from a JAR file.  This method must be called in the
	 * service context.
	 * @return True if success, False otherwise
	 */
	public boolean loadUavobjects(String jar, UAVObjectManager objMngr) {
	    final String JAR_DIR = "jars";
	    final String DEX_DIR = "optimized_dex";

	    File jarsDir = getDir(JAR_DIR, MODE_WORLD_READABLE);
	    if (jarsDir.exists())
	    	deleteDirectoryContents(jarsDir);

	    copyAssets(JAR_DIR, jar);

		Log.d(TAG, "Starting dex loader");
		File dexDir = getDir(DEX_DIR, Context.MODE_WORLD_READABLE);

		// Necessary to get dexOpt to run
		if (dexDir.exists())
			deleteDirectoryContents(dexDir);

		String classpath = new File(jarsDir, jar).getAbsolutePath();

		DexClassLoader loader = new DexClassLoader(classpath, dexDir.getAbsolutePath(), null, getClassLoader());

		try {
			Class<?> initClass = loader.loadClass("org.openpilot.uavtalk.uavobjects.UAVObjectsInitialize");
			Method initMethod = initClass.getMethod("register", UAVObjectManager.class);
			initMethod.invoke(null, objMngr);
		} catch (ClassNotFoundException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
			return false;
		} catch (IllegalAccessException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
			return false;
		} catch (NoSuchMethodException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
			return false;
		} catch (IllegalArgumentException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		} catch (InvocationTargetException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}

		return true;
	}
}
