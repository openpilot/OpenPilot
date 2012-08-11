package org.openpilot.androidgcs.telemetry;

// Code based on notes from http://torvafirmus-android.blogspot.com/2011/09/implementing-usb-hid-interface-in.html
// Taken 2012-08-10

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Iterator;

import org.openpilot.uavtalk.UAVObjectManager;
import org.openpilot.uavtalk.UAVTalk;

import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbConstants;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import android.hardware.usb.UsbRequest;
import android.util.Log;

public class HidUAVTalk {

	private static final String TAG = HidUAVTalk.class.getSimpleName();
	public static int LOGLEVEL = 2;
	public static boolean DEBUG = LOGLEVEL > 1;
	public static boolean WARN = LOGLEVEL > 0;


	Service hostDisplayActivity;

	private boolean connected;

	private UAVTalk uavTalk;

	private UAVObjectManager objMngr;

	//! USB constants
	static final int OPENPILOT_VENDOR_ID = 0x20A0;

	static final int USB_PRODUCT_ID_OPENPILOT_MAIN = 0x415A;
	static final int USB_PRODUCT_ID_COPTERCONTROL  = 0x415B;
	static final int USB_PRODUCT_ID_PIPXTREME      = 0x415C;
	static final int USB_PRODUCT_ID_CC3D           = 0x415D;
	static final int USB_PRODUCT_ID_REVOLUTION     = 0x415E;
	static final int USB_PRODUCT_ID_OSD            = 0x4194;
	static final int USB_PRODUCT_ID_SPARE          = 0x4195;

	public HidUAVTalk(Service service) {
		hostDisplayActivity = service;
	}

	public boolean getConnected() {
		return connected;
	}

	public UAVTalk getUavtalk() {
		return uavTalk;
	}

	/**
	 * Opens a TCP socket to the address determined on construction.  If successful
	 * creates a UAVTalk stream connection this socket to the passed in object manager
	 */
	public boolean openTelemetryHid(UAVObjectManager objMngr) {
		uavTalk = new UAVTalk(inStream, outStream, objMngr);
		this.objMngr = objMngr;
		return true;
	}

	public void disconnect() {
		CleanUpAndClose();
		//hostDisplayActivity.unregisterReceiver(usbReceiver);
		//hostDisplayActivity.unregisterReceiver(usbPermissionReceiver);
		inStream.stop();
		outStream.stop();
	}

	public boolean connect(UAVObjectManager objMngr) {
		if (DEBUG) Log.d(TAG, "connect()");

		// Register to get permission requested dialog
		usbManager = (UsbManager) hostDisplayActivity.getSystemService(Context.USB_SERVICE);
		permissionIntent = PendingIntent.getBroadcast(hostDisplayActivity, 0, new Intent(ACTION_USB_PERMISSION), 0);
		permissionFilter = new IntentFilter(ACTION_USB_PERMISSION);
		hostDisplayActivity.registerReceiver(usbPermissionReceiver, permissionFilter);

		// Register to get notified on device attached
		/*deviceAttachedFilter = new IntentFilter();
		deviceAttachedFilter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
		deviceAttachedFilter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
		hostDisplayActivity.registerReceiver(usbReceiver, deviceAttachedFilter);*/

		HashMap<String, UsbDevice> deviceList = usbManager.getDeviceList();
		if (DEBUG) Log.d(TAG, "Found " + deviceList.size() + " devices");
		Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
		while(deviceIterator.hasNext()){
			UsbDevice dev = deviceIterator.next();
			if (DEBUG) Log.d(TAG, "Testing device: " + dev);
			usbManager.requestPermission(dev, permissionIntent);
			//ConnectToDeviceInterface(dev);
		}

		if (DEBUG) Log.d(TAG, "Registered the deviceAttachedFilter");

		try {
			Thread.sleep(1000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		if( !openTelemetryHid(objMngr) )
			return false;

		return connected;

	}

	public HidUAVTalk(OPTelemetryService opTelemetryService) {
		this.hostDisplayActivity = opTelemetryService;
	}

	private static final String ACTION_USB_PERMISSION = "com.access.device.USB_PERMISSION";

	UsbDevice currentDevice;

	/*
	 * Receives a requested broadcast from the operating system.
	 * In this case the following actions are handled:
	 *   USB_PERMISSION
	 *   UsbManager.ACTION_USB_DEVICE_DETACHED
	 *   UsbManager.ACTION_USB_DEVICE_ATTACHED
	 */
	private final BroadcastReceiver usbPermissionReceiver = new BroadcastReceiver()
	{
		@Override
		public void onReceive(Context context, Intent intent)
		{
			if (DEBUG) Log.d(TAG,"Broadcast receiver caught intent: " + intent);
			String action = intent.getAction();
			// Validate the action against the actions registered
			if (ACTION_USB_PERMISSION.equals(action))
			{
				// A permission response has been received, validate if the user has
				// GRANTED, or DENIED permission
				synchronized (this)
				{
					UsbDevice deviceConnected = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

					if (DEBUG) Log.d(TAG, "Device Permission requested" + deviceConnected);
					if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false))
					{
						// Permission has been granted, so connect to the device
						// If this fails, then keep looking
						if (deviceConnected != null)
						{
							// call method to setup device communication
							currentDevice = deviceConnected;
							if (DEBUG) Log.d(TAG, "Device Permission Acquired" + currentDevice);
							if (!ConnectToDeviceInterface(currentDevice))
							{
								if (DEBUG) Log.d(TAG, "Unable to connect to device");
							}
							sendEnabledMessage();
							// TODO: Create a listener to receive messages from the host

						}
					}
					else
					{
						// Permission has not been granted, so keep looking for another
						// device to be attached....
						if (DEBUG) Log.d(TAG, "Device Permission Denied" + deviceConnected);
						currentDevice = null;
					}
				}
			}
		}
	};
	private final BroadcastReceiver usbReceiver = new BroadcastReceiver()
	{
		@Override
		public void onReceive(Context context, Intent intent)
		{
			if (DEBUG) Log.d(TAG,"Broadcast receiver taught intent: " + intent);
			String action = intent.getAction();
			// Validate the action against the actions registered

			if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action))
			{
				// A device has been detached from the device, so close all the connections
				// and restart the search for a new device being attached
				UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
				if ((device != null) && (currentDevice != null))
				{
					if (device.equals(currentDevice))
					{
						// call your method that cleans up and closes communication with the device
						CleanUpAndClose();
					}
				}
			}
			else if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action))
			{
				// A device has been attached. If not already connected to a device,
				// validate if this device is supported
				UsbDevice searchDevice = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
				if (DEBUG) Log.d(TAG, "Device found" + searchDevice);
				if ((searchDevice != null) && (currentDevice == null))
				{
					// call your method that cleans up and closes communication with the device
					ValidateFoundDevice(searchDevice);
				}
			}
		}

		private void sendEnabledMessage() {
			// TODO Auto-generated method stub

		};
	};
	private UsbEndpoint usbEndpointRead;

	private UsbEndpoint usbEndpointWrite;

	private UsbManager usbManager;

	private PendingIntent permissionIntent;

	private UsbDeviceConnection connectionRead;

	private UsbDeviceConnection connectionWrite;

	private IntentFilter deviceAttachedFilter;

	private IntentFilter permissionFilter;

	protected void sendEnabledMessage() {
		// TODO Auto-generated method stub

	}

	protected void CleanUpAndClose() {
		if (UsingSingleInterface) {
			if(connectionRead != null && usbInterfaceRead != null)
				connectionRead.releaseInterface(usbInterfaceRead);
			usbInterfaceRead = null;
		}
	else {
		if(connectionRead != null && usbInterfaceRead != null)
			connectionRead.releaseInterface(usbInterfaceRead);
		if(connectionWrite != null && usbInterfaceWrite != null)
			connectionWrite.releaseInterface(usbInterfaceWrite);
		usbInterfaceWrite = null;
		usbInterfaceRead = null;
		}
	}

	//Validating the Connected Device - Before asking for permission to connect to the device, it is essential that you ensure that this is a device that you support or expect to connect to. This can be done by validating the devices Vendor ID and Product ID.
	boolean ValidateFoundDevice(UsbDevice searchDevice) {
		//A vendor id is a global identifier for the manufacturer. A product id refers to the product itself, and is unique to the manufacturer. The vendor id, product id combination refers to a particular product manufactured by a vendor.
		if (DEBUG) Log.d(TAG, "ValidateFoundDevice: " + searchDevice );

		if ( searchDevice.getVendorId() == OPENPILOT_VENDOR_ID ) {
			//Requesting permission
			if (DEBUG) Log.d(TAG, "Device: " + searchDevice );
			usbManager.requestPermission(searchDevice, permissionIntent);
			return true;
		}
		else
			return false;
	}

	private UsbInterface usbInterfaceRead = null;
	private UsbInterface usbInterfaceWrite = null;
	private final boolean UsingSingleInterface = true;

	private UsbDevice connectedDevice;

	boolean ConnectToDeviceInterface(UsbDevice connectDevice) {
		// Connecting to the Device - If you are reading and writing, then the device
		// can either have two end points on a single interface, or two interfaces
		// each with a single end point. Either way, it is best if you know which interface
		// you need to use and which end points

		if (DEBUG) Log.d(TAG, "ConnectToDeviceInterface:");
		UsbEndpoint ep1 = null;
		UsbEndpoint ep2 = null;


		if (UsingSingleInterface)
		{
			// Using the same interface for reading and writing
			usbInterfaceRead = connectDevice.getInterface(0x2);
			usbInterfaceWrite = usbInterfaceRead;
			if (usbInterfaceRead.getEndpointCount() == 2)
			{
				ep1 = usbInterfaceRead.getEndpoint(0);
				ep2 = usbInterfaceRead.getEndpoint(1);
			}
		}
		else        // if (!UsingSingleInterface)
		{
			usbInterfaceRead = connectDevice.getInterface(0x01);
			usbInterfaceWrite = connectDevice.getInterface(0x02);
			if ((usbInterfaceRead.getEndpointCount() == 1) && (usbInterfaceWrite.getEndpointCount() == 1))
			{
				ep1 = usbInterfaceRead.getEndpoint(0);
				ep2 = usbInterfaceWrite.getEndpoint(0);
			}
		}


		if ((ep1 == null) || (ep2 == null))
		{
			if (DEBUG) Log.d(TAG, "Null endpoints");
			return false;
		}

		// Determine which endpoint is the read, and which is the write

		if (ep1.getType() == UsbConstants.USB_ENDPOINT_XFER_INT)
		{
			if (ep1.getDirection() == UsbConstants.USB_DIR_IN)
			{
				usbEndpointRead = ep1;
			}
			else if (ep1.getDirection() == UsbConstants.USB_DIR_OUT)
			{
				usbEndpointWrite = ep1;
			}
		}
		if (ep2.getType() == UsbConstants.USB_ENDPOINT_XFER_INT)
		{
			if (ep2.getDirection() == UsbConstants.USB_DIR_IN)
			{
				usbEndpointRead = ep2;
			}
			else if (ep2.getDirection() == UsbConstants.USB_DIR_OUT)
			{
				usbEndpointWrite = ep2;
			}
		}
		if ((usbEndpointRead == null) || (usbEndpointWrite == null))
		{
			if (DEBUG) Log.d(TAG, "Endpoints wrong way around");
			return false;
		}
		connectionRead = usbManager.openDevice(connectDevice);
		connectionRead.claimInterface(usbInterfaceRead, true);


		if (UsingSingleInterface)
		{
			connectionWrite = connectionRead;
		}
		else // if (!UsingSingleInterface)
		{
			connectionWrite = usbManager.openDevice(connectDevice);
			connectionWrite.claimInterface(usbInterfaceWrite, true);
		}

		connectedDevice = connectDevice;
		connected = true;
		if (DEBUG) Log.d(TAG, "Opened endpoints");

		return true;
	}

	private class TalkInputStream extends InputStream {
		ByteBuffer data = null;
		boolean stopped = false;
		private static final int SIZE = 1024;
		int readPosition = 0;
		TalkInputStream() {
			data = ByteBuffer.allocate(SIZE);
			data.limit(SIZE);
			data.clear();
		}

		@Override
		public int read() throws IOException {
			while(!stopped) {
				synchronized(data) {
					if(data.capacity() > readPosition)
						return data.get(readPosition++);
					else
						try {
							data.wait(50);
						} catch (InterruptedException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
				}
			}
			throw new IOException();
		}

		void compress() {
			if (readPosition > (data.capacity() * 3.0 / 4.0)) {
				synchronized(data) {
					data.position(readPosition);
					data.compact();
					readPosition = 0;
					if (DEBUG) Log.d(TAG, "Compact() Capacity: " + data.capacity() + " Position: " + data.position() + " Available: " + data.remaining() + " Limit: " + data.limit());
				}
			}
		}

		public void stop() {
			stopped = true;
		}

		public void put(byte b) {
			synchronized(data) {
				if(data.hasRemaining()) {
					data.compact();
					data.put(b);
					data.notify();
				}
			}
		}

		public void write(byte[] b) {
			synchronized(data) {
				int available = data.remaining();
				if(available >= b.length) {
					if (DEBUG) Log.d(TAG, "Size: " + b.length + " Capacity: " + data.capacity() + " Position: " + data.position() + " Available: " + available + " Limit: " + data.limit());
					//data.compact();
					data.put(b);
					data.notify();
				}
			}
		}
	};
	private final TalkInputStream inStream = new TalkInputStream();

	/**
	 * Gets a report from HID, extract the meaningful data and push
	 * it to the input stream
	 */
	public int readData() {
		int bufferDataLength = usbEndpointRead.getMaxPacketSize();
		ByteBuffer buffer = ByteBuffer.allocate(bufferDataLength + 1);
		UsbRequest request = new UsbRequest();
		request.initialize(connectionRead, usbEndpointRead);


        // queue a request on the interrupt endpoint
        if(!request.queue(buffer, bufferDataLength)) {
        	if (DEBUG) Log.d(TAG, "Failed to queue request");
        	return 0;
        }

        if (DEBUG) Log.d(TAG, "Request queued");

        int dataSize;
        // wait for status event
        if (connectionRead.requestWait() == request) {
        	// Packet format:
        	// 0: Report ID (1)
        	// 1: Number of valid bytes
        	// 2:63: Data

        	dataSize = buffer.get(1);    // Data size
        	//Assert.assertEquals(1, buffer.get()); // Report ID
        	//Assert.assertTrue(dataSize < buffer.capacity());

        	if (buffer.get(0) != 1 || buffer.get(1) < 0 || buffer.get(2) > (buffer.capacity() - 2)) {
        		if (DEBUG) Log.d(TAG, "Badly formatted HID packet");
        	} else {
        		byte[] dst = new byte[dataSize];
        		buffer.position(2);
        		buffer.get(dst, 0, dataSize);
        		inStream.write(dst);
        		if (DEBUG) Log.d(TAG, "Got read: " + dataSize + " bytes");
        	}
        } else
        	return 0;

        return dataSize;
	}

	private class TalkOutputStream extends OutputStream {
		ByteBuffer data = ByteBuffer.allocate(128);
		boolean stopped = false;

		public int read() throws IOException {
			if (stopped)
				while(!stopped) {
					synchronized(data) {
						if(data.hasRemaining())
							return data.get();
						else
							try {
								data.wait();
							} catch (InterruptedException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
					}
					throw new IOException();
				}
			return 0;
		}

		public void stop() {
			stopped = true;
		}

		@Override
		public void write(int oneByte) throws IOException {
			if (stopped)
				throw new IOException();
			synchronized(data) {
				data.put((byte) oneByte);
				data.notify();
			}
		}

		@Override
		public void write(byte[] b) throws IOException {
			if (stopped)
				throw new IOException();

			synchronized(data) {
				//data.put(b);
				data.notify();
			}
		}
	};
	private final TalkOutputStream outStream = new TalkOutputStream();

	boolean WriteToDevice(ByteBuffer DataToSend) {

		//The report must be formatted correctly for the device being connected to. On some devices, this requires that a specific value must be the first byte in the report. This can be followed by the length of the data in the report. This format is determined by the device, and isn't specified here.

		int bufferDataLength = usbEndpointWrite.getMaxPacketSize();
		ByteBuffer buffer = ByteBuffer.allocate(bufferDataLength + 1);
		UsbRequest request = new UsbRequest();

		buffer.put(DataToSend);

		request.initialize(connectionWrite, usbEndpointWrite);
		request.queue(buffer, bufferDataLength);
		try
		{
			if (request.equals(connectionWrite.requestWait()))
			{
				return true;
			}
		}
		catch (Exception ex)
		{
			// An exception has occured
			return false;
		}
		return false;
	}


	//Reading from the Device - As USB devices work with reports of a specific length. The data received from the device will always be the size specified by the report length. Even if there isn't enough data to fill the report. Some devices require the controlTransfer method for reading data. I don't cover this command in this blog.
	void ReadFromDevice() {
		//If you are expecting unsolicited data from the device, then a read thread should be started so that the data can be processed as soon as it arrives.

		int bufferDataLength = usbEndpointRead.getMaxPacketSize();
		ByteBuffer buffer = ByteBuffer.allocate(bufferDataLength + 1);
		UsbRequest requestQueued = null;
		UsbRequest request = new UsbRequest();
		request.initialize(connectionRead, usbEndpointRead);

		try
		{
			while (!getStopping())
			{
				request.queue(buffer, bufferDataLength);
				requestQueued = connectionRead.requestWait();
				if (request.equals(requestQueued))
				{
					byte[] byteBuffer = new byte[bufferDataLength + 1];
					buffer.get(byteBuffer, 0, bufferDataLength);

					// Handle data received

					buffer.clear();
				}
				else
				{
					Thread.sleep(20);
				}
			}
		}
		catch (Exception ex)
		{
			// An exception has occured
		}
		try
		{
			request.cancel();
			request.close();
		}
		catch (Exception ex)
		{
			// An exception has occured
		}
	}

	private boolean getStopping() {
		// TODO Auto-generated method stub
		return false;
	}
}
