package org.openpilot.androidgcs.telemetry;

// Code based on notes from http://torvafirmus-android.blogspot.com/2011/09/implementing-usb-hid-interface-in.html
// Taken 2012-08-10

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Iterator;

import junit.framework.Assert;
import android.app.PendingIntent;
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

public class HidUAVTalk extends TelemetryTask {

	private static final String TAG = HidUAVTalk.class.getSimpleName();
	public static final int LOGLEVEL = 1;
	public static final boolean DEBUG = LOGLEVEL > 2;
	public static final boolean WARN = LOGLEVEL > 1;
	public static final boolean ERROR = LOGLEVEL > 0;

	//! USB constants
	private static final int MAX_HID_PACKET_SIZE = 64;
	static final int OPENPILOT_VENDOR_ID = 0x20A0;

	static final int USB_PRODUCT_ID_OPENPILOT_MAIN = 0x415A;
	static final int USB_PRODUCT_ID_COPTERCONTROL  = 0x415B;
	static final int USB_PRODUCT_ID_PIPXTREME      = 0x415C;
	static final int USB_PRODUCT_ID_CC3D           = 0x415D;
	static final int USB_PRODUCT_ID_REVOLUTION     = 0x415E;
	static final int USB_PRODUCT_ID_OSD            = 0x4194;
	static final int USB_PRODUCT_ID_SPARE          = 0x4195;

	private static final String ACTION_USB_PERMISSION = "com.access.device.USB_PERMISSION";

	private UsbDevice currentDevice;
	private UsbEndpoint usbEndpointRead;
	private UsbEndpoint usbEndpointWrite;
	private UsbManager usbManager;
	private PendingIntent permissionIntent;
	private UsbDeviceConnection usbDeviceConnection;
	private IntentFilter permissionFilter;
	private UsbInterface usbInterface = null;
	private TalkInputStream inTalkStream;
	private TalkOutputStream outTalkStream;
	private final UsbRequest writeRequest = null;
	private UsbRequest readRequest = null;
	private Thread readThread;
	private Thread writeThread;

	private boolean readPending = false;
	private boolean writePending = false;
	private IntentFilter deviceAttachedFilter;

	public HidUAVTalk(OPTelemetryService service) {
		super(service);
	}

	@Override
	public void disconnect() {

		CleanUpAndClose();
		telemService.unregisterReceiver(usbReceiver);
		telemService.unregisterReceiver(usbPermissionReceiver);

		super.disconnect();

		try {
			if(readThread != null) {
				readThread.interrupt(); // Make sure not blocking for data
				readThread.join();
				readThread = null;
			}
			if(writeThread != null) {
				writeThread.interrupt();
				writeThread.join();
				writeThread = null;
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		if (readRequest != null) {
			readRequest.cancel();
			readRequest.close();
			readRequest = null;
		}
	}

	@Override
	boolean attemptConnection() {
		if (DEBUG) Log.d(TAG, "connect()");

		// Register to get permission requested dialog
		usbManager = (UsbManager) telemService.getSystemService(Context.USB_SERVICE);
		permissionIntent = PendingIntent.getBroadcast(telemService, 0, new Intent(ACTION_USB_PERMISSION), 0);
		permissionFilter = new IntentFilter(ACTION_USB_PERMISSION);
		telemService.registerReceiver(usbPermissionReceiver, permissionFilter);

		deviceAttachedFilter = new IntentFilter();
		deviceAttachedFilter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
		deviceAttachedFilter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
		telemService.registerReceiver(usbReceiver, deviceAttachedFilter);

		// Go through all the devices plugged in
		HashMap<String, UsbDevice> deviceList = usbManager.getDeviceList();
		if (DEBUG) Log.d(TAG, "Found " + deviceList.size() + " devices");
		Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
		while(deviceIterator.hasNext()){
			UsbDevice dev = deviceIterator.next();
			if (DEBUG) Log.d(TAG, "Testing device: " + dev);
			if( ValidateFoundDevice(dev) ) {
				usbManager.requestPermission(dev, permissionIntent);
				break;
			}
		}

		if (DEBUG) Log.d(TAG, "Registered the deviceAttachedFilter");

		return deviceList.size() > 0;
	}

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
						telemService.toastMessage("Device unplugged while in use");
						if (DEBUG) Log.d(TAG, "Matching device disconnected");
						// call your method that cleans up and closes communication with the device
						disconnect();
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
	};


	protected void CleanUpAndClose() {
		if(usbDeviceConnection != null && usbInterface != null)
			usbDeviceConnection.releaseInterface(usbInterface);
		usbInterface = null;
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

	boolean ConnectToDeviceInterface(UsbDevice connectDevice) {
		// Connecting to the Device - If you are reading and writing, then the device
		// can either have two end points on a single interface, or two interfaces
		// each with a single end point. Either way, it is best if you know which interface
		// you need to use and which end points

		if (DEBUG) Log.d(TAG, "ConnectToDeviceInterface:");
		UsbEndpoint ep1 = null;
		UsbEndpoint ep2 = null;

		if (connectDevice.getInterfaceCount() < 2) {
			if (ERROR) Log.e(TAG, "Interface count for USB device incorrect");
			telemService.toastMessage("Failed to connect");
			return false;
		}

		// Using the same interface for reading and writing
		usbInterface = connectDevice.getInterface(0x2);
		if (usbInterface.getEndpointCount() == 2)
		{
			ep1 = usbInterface.getEndpoint(0);
			ep2 = usbInterface.getEndpoint(1);
		}

		if ((ep1 == null) || (ep2 == null))
		{
			if (ERROR) Log.e(TAG, "Null endpoints");
			return false;
		}

		// Determine which endpoint is the read, and which is the write
		if (ep1.getType() == UsbConstants.USB_ENDPOINT_XFER_INT)
		{
			if (ep1.getDirection() == UsbConstants.USB_DIR_IN)
				usbEndpointRead = ep1;
			else if (ep1.getDirection() == UsbConstants.USB_DIR_OUT)
				usbEndpointWrite = ep1;
		}
		if (ep2.getType() == UsbConstants.USB_ENDPOINT_XFER_INT)
		{
			if (ep2.getDirection() == UsbConstants.USB_DIR_IN)
				usbEndpointRead = ep2;
			else if (ep2.getDirection() == UsbConstants.USB_DIR_OUT)
				usbEndpointWrite = ep2;
		}
		if ((usbEndpointRead == null) || (usbEndpointWrite == null))
		{
			if (ERROR) Log.e(TAG, "Could not find write and read endpoint");
			return false;
		}

		// Claim the interface
		usbDeviceConnection = usbManager.openDevice(connectDevice);
		usbDeviceConnection.claimInterface(usbInterface, true);


		if (DEBUG) Log.d(TAG, "Opened endpoints");

		// Create the USB requests
		readRequest = new UsbRequest();
		readRequest.initialize(usbDeviceConnection, usbEndpointRead);

		inTalkStream = new TalkInputStream();
		outTalkStream = new TalkOutputStream();
		inStream = inTalkStream;
		outStream = outTalkStream;
		handler.post(new Runnable() {
			@Override
			public void run() {
				attemptSucceeded();
			}
		});

		readThread = new Thread(new Runnable() {
			@Override
			public void run() {
				// Enqueue the first read
				queueRead();
				while (!shutdown) {
					UsbRequest returned = usbDeviceConnection.requestWait();
					if (returned == readRequest) {
						if (DEBUG) Log.d(TAG, "Received read request");
						readData();
					} else {
						Log.e(TAG, "Received unknown USB response");
						break;
					}
				}
			}

		}, "HID Read");
		readThread.start();

		writeThread = new Thread(new Runnable() {
			@Override
			public void run() {
				if (DEBUG) Log.d(TAG, "Starting HID write thread");
				while(!shutdown) {
					try {
						if (sendDataSynchronous() == false)
							break;
					} catch (InterruptedException e) {
						break;
					}
				}
				if (DEBUG) Log.d(TAG, "Ending HID write thread");
			}
		}, "HID Write");
		writeThread.start();

		telemService.toastMessage("HID Device Opened");

		return true;
	}


	void displayBuffer(String msg, byte[] buf) {
		msg += " (";
		for (int i = 0; i < buf.length; i++) {
			msg += buf[i] + " ";
		}
		msg += ")";
		Log.d(TAG, msg);
	}
	/**
	 * Gets a report from HID, extract the meaningful data and push
	 * it to the input stream
	 */
	ByteBuffer readBuffer = ByteBuffer.allocate(MAX_HID_PACKET_SIZE);

	/**
	 * Schedules a USB read
	 */
	private void queueRead() {
		synchronized(readRequest) {
			if(!readRequest.queue(readBuffer, MAX_HID_PACKET_SIZE)) {
				if (ERROR) Log.e(TAG, "Failed to queue request");
			} else
				readPending = true;
		}
	}

	/**
	 * Reads data from the last USB transaction and schedules another read
	 */
	public void readData() {
		synchronized(readRequest) {

			if (!readPending) {
				if (ERROR) Log.e(TAG, "Tried to read read while a transaction was not pending");
				return;
			}

			// We just received a read
			readPending = false;
			// Packet format:
			// 0: Report ID (1)
			// 1: Number of valid bytes
			// 2:63: Data

			int dataSize = readBuffer.get(1);    // Data size
			//Assert.assertEquals(1, buffer.get()); // Report ID
			//Assert.assertTrue(dataSize < buffer.capacity());

			if (readBuffer.get(0) != 1 || readBuffer.get(1) < 0 || readBuffer.get(1) > (readBuffer.capacity() - 2)) {
				if (ERROR) Log.e(TAG, "Badly formatted HID packet");
			} else {
				byte[] dst = new byte[dataSize];
				readBuffer.position(2);
				readBuffer.get(dst, 0, dataSize);
				if (DEBUG) Log.d(TAG, "Entered read");
				inTalkStream.write(dst);
				if (DEBUG) Log.d(TAG, "Got read: " + dataSize + " bytes");
			}

			// Queue another read
			queueRead();

		}
	}


	/**
	 * Send a packet if data is available
	 */
	public void sendData() {
		synchronized(writeRequest) {
			// Don't try and send data till previous request completes
			if (writePending)
				return;

			ByteBuffer packet = outTalkStream.getHIDpacket();
			if (packet != null) {
				if (DEBUG) Log.d(TAG, "Writing to device()");

				int bufferDataLength = usbEndpointWrite.getMaxPacketSize();
				Assert.assertTrue(packet.capacity() <= bufferDataLength);

				if(writeRequest.queue(packet, bufferDataLength))
					writePending = true;
				else if (ERROR)
					Log.e(TAG, "Write queuing failed");
			}
		}
	}

	/**
	 * Send a packet if data is available
	 * @throws InterruptedException
	 */
	public boolean sendDataSynchronous() throws InterruptedException {

		ByteBuffer packet = outTalkStream.getHIDpacketBlocking();
		if (packet != null) {
			if (DEBUG) Log.d(TAG, "sendDataSynchronous() Writing to device()");

			if (usbDeviceConnection.bulkTransfer(usbEndpointWrite, packet.array(), MAX_HID_PACKET_SIZE, 1000) < 0) {
				Log.e(TAG, "Failed to perform bulk write");
				return false;
			}
		}
		return true;
	}

	/*********** Helper classes for telemetry streams ************/

	class TalkOutputStream extends OutputStream {
		// Uses ByteFifo.getByteBlocking()
		// and  ByteFifo.put(byte [])
		ByteFifo data = new ByteFifo();

		/**
		 * Blocks until data is available and then returns a properly formatted HID packet
		 */
		public ByteBuffer getHIDpacketBlocking() throws InterruptedException {
			synchronized(data) {
				if (data.remaining() == 0)
					data.wait();
				return getHIDpacket();
			}
		}

		/**
		 * Gets data from the ByteFifo in a properly formatted HID packet
		 */
		public ByteBuffer getHIDpacket() {
			ByteBuffer packet = null;
			synchronized(data) {
				// Determine how much data to put in the packet
				int size = Math.min(data.remaining(), MAX_HID_PACKET_SIZE - 2);
				if (size <= 0)
					return packet;

				// Format into a HID packet
				packet = ByteBuffer.allocate(MAX_HID_PACKET_SIZE);
				packet.put(0,(byte) 2);             // Report ID
				packet.put(1,(byte) size);          // The number of useful bytes
				data.get(packet.array(), 2, size);

				if (DEBUG) Log.d(TAG, "packetizeData(): size="+size);
			}
			return packet;
		}
		@Override
		public void write(int oneByte) throws IOException {
			// Throw exception when try and read after shutdown
			if (shutdown)
				throw new IOException();

			synchronized(data) {
				data.put((byte) oneByte);
				data.notify();
			}
		}

		@Override
		public void write(byte[] b) throws IOException {
			if (shutdown)
				throw new IOException();

			synchronized(data) {
				data.put(b);
				data.notify();
			}
		}

	};

	private class TalkInputStream extends InputStream {
		// Uses ByteFifo.getByteBlocking()
		// Uses ByteFifo.put(byte[])
		ByteFifo data = new ByteFifo();

		TalkInputStream() {
		}

		@Override
		public int read() {
			try {
				return data.getByteBlocking();
			} catch (InterruptedException e) {
				Log.e(TAG, "Timed out");
				e.printStackTrace();
			}
			return -1;
		}

		public void write(byte[] b) {
			data.put(b);
		}
	};

	private class ByteFifo {

		//! The maximum size of the fifo
		private final int MAX_SIZE = 256;
		//! The number of bytes in the buffer
		private int size = 0;
		//! Internal buffer
		private final ByteBuffer buf;

		ByteFifo() {
			buf = ByteBuffer.allocate(MAX_SIZE);
			size = 0;
		}

		private int byteToInt(byte b) { return b & 0x000000ff; }

		final int remaining() { return size; };

		public boolean put(byte b) {
			byte[] a = {b};
			return put(a);
		}

		public boolean put(byte[] dat) {
			if ((size + dat.length) > MAX_SIZE) {
				Log.e(TAG, "Dropped data.  Size:" + size + " data length: " + dat.length);
				return false;
			}

			// Place data at the end of the buffer
			synchronized(buf) {
				buf.position(size);
				buf.put(dat);
				size = size + dat.length;
				buf.notify();
			}
			return true;
		}

		public ByteBuffer get(byte[] dst, int offset, int size) {
			synchronized(buf) {
				buf.flip();
				buf.get(dst, offset, size);
				buf.compact();
				this.size -= size;
			}
			return buf;
		}

		public int getByteBlocking() throws InterruptedException {
			synchronized(buf) {
				if (size == 0) {
					buf.wait();
				}
				int val = byteToInt(buf.get(0));
				buf.position(1);
				buf.compact();
				size--;
				return val;
			}
		}
	}
}