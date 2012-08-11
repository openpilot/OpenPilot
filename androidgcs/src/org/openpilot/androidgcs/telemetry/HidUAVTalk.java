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
	public static int LOGLEVEL = 0;
	public static boolean WARN = LOGLEVEL > 1;
	public static boolean DEBUG = LOGLEVEL > 0;

	//! USB constants
	static final int OPENPILOT_VENDOR_ID = 0x20A0;

	static final int USB_PRODUCT_ID_OPENPILOT_MAIN = 0x415A;
	static final int USB_PRODUCT_ID_COPTERCONTROL  = 0x415B;
	static final int USB_PRODUCT_ID_PIPXTREME      = 0x415C;
	static final int USB_PRODUCT_ID_CC3D           = 0x415D;
	static final int USB_PRODUCT_ID_REVOLUTION     = 0x415E;
	static final int USB_PRODUCT_ID_OSD            = 0x4194;
	static final int USB_PRODUCT_ID_SPARE          = 0x4195;

	private static final String ACTION_USB_PERMISSION = "com.access.device.USB_PERMISSION";

	UsbDevice currentDevice;

	public HidUAVTalk(OPTelemetryService service) {
		super(service);
	}

	@Override
	public void disconnect() {

		CleanUpAndClose();
		//hostDisplayActivity.unregisterReceiver(usbReceiver);
		telemService.unregisterReceiver(usbPermissionReceiver);
		((TalkInputStream)inStream).stop();
		((TalkOutputStream)outStream).stop();

		super.disconnect();

		try {
			readThread.join();
			writeThread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		if (readRequest != null) {
			readRequest.cancel();
			readRequest.close();
			readRequest = null;
		}

		if (writeRequest != null) {
			writeRequest.cancel();
			writeRequest.close();
			writeRequest = null;
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

		// Go through all the devices plugged in
		HashMap<String, UsbDevice> deviceList = usbManager.getDeviceList();
		if (DEBUG) Log.d(TAG, "Found " + deviceList.size() + " devices");
		Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
		while(deviceIterator.hasNext()){
			UsbDevice dev = deviceIterator.next();
			if (DEBUG) Log.d(TAG, "Testing device: " + dev);
			usbManager.requestPermission(dev, permissionIntent);
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
	};

	private UsbEndpoint usbEndpointRead;

	private UsbEndpoint usbEndpointWrite;

	private UsbManager usbManager;

	private PendingIntent permissionIntent;

	private UsbDeviceConnection connectionRead;

	private UsbDeviceConnection connectionWrite;

	private IntentFilter deviceAttachedFilter;

	private IntentFilter permissionFilter;

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
		if (DEBUG) Log.d(TAG, "Opened endpoints");

		// Create the USB requests
		readRequest = new UsbRequest();
		readRequest.initialize(connectionRead, usbEndpointRead);

		writeRequest = new UsbRequest();
		writeRequest.initialize(connectionWrite, usbEndpointWrite);


		handler.post(new Runnable() {
			@Override
			public void run() {
				inStream = new TalkInputStream();
				outStream = new TalkOutputStream();
				attemptSucceeded();
			}
		});

		readThread = new Thread(new Runnable() {
			@Override
			public void run() {
				while (!shutdown) {
					readData();
					sendData();
				}
			}

		});
		readThread.start();

		writeThread = new Thread(new Runnable() {
			@Override
			public void run() {
				while (!shutdown) {
					sendData();
				}
			}

		});
		//writeThread.start();

		return true;
	}

	Thread readThread;
	Thread writeThread;
	private int byteToInt(byte b) { return b & 0x000000ff; }

	private class TalkInputStream extends InputStream {

		ByteFifo data = new ByteFifo();
		boolean stopped = false;

		TalkInputStream() {
		}

		@Override
		public int read() {
			try {
				return data.getByteBlocking();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			return -1;
		}

		public void stop() {
			stopped = true;
		}

		public void put(byte b) {
		}

		public void write(byte[] b) {
			data.put(b);
		}
	};

	private class ByteFifo {

		//! The maximum size of the fifo
		private final int MAX_SIZE = 1024;
		//! The number of bytes in the buffer
		private int size = 0;
		//! Internal buffer
		private final ByteBuffer buf;

		ByteFifo() {
			buf = ByteBuffer.allocate(MAX_SIZE);
			size = 0;
		}

		public boolean put(byte[] dat) {
			if ((size + dat.length) > MAX_SIZE)
				return false;

			// Place data at the end of the buffer
			synchronized(buf) {
				buf.position(size);
				buf.put(dat);
				size = size + dat.length;
				buf.notify();
			}
			return true;
		}

		public byte[] get(int size) throws InterruptedException {
			size = Math.min(size, this.size);
			if (size > 0) {
				synchronized(buf) {
					byte[] dst = new byte[size];
					buf.position(0);
					buf.get(dst,0,size);
					buf.compact();
					this.size = this.size - size;
					Assert.assertEquals(this.size, buf.position());

					buf.wait();
				}
			}
			return new byte[0];
		}

		public int getByteBlocking() throws InterruptedException {
			synchronized(buf) {
				if (size == 0)
					buf.wait();
				int val = byteToInt(buf.get(0));
				buf.position(1);
				buf.compact();
				size--;

				return val;
			}
		}
	}

	/**
	 * Gets a report from HID, extract the meaningful data and push
	 * it to the input stream
	 */
	UsbRequest readRequest = null;
	public int readData() {
		int bufferDataLength = usbEndpointRead.getMaxPacketSize();
		ByteBuffer buffer = ByteBuffer.allocate(bufferDataLength + 1);

        // queue a request on the interrupt endpoint
        if(!readRequest.queue(buffer, bufferDataLength)) {
        	if (DEBUG) Log.d(TAG, "Failed to queue request");
        	return 0;
        }

        if (DEBUG) Log.d(TAG, "Request queued");

        int dataSize;
        // wait for status event
        if (connectionRead.requestWait() == readRequest) {
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
        		if (DEBUG) Log.d(TAG, "Entered read");
        		((TalkInputStream)inStream).write(dst);
        		if (DEBUG) Log.d(TAG, "Got read: " + dataSize + " bytes");
        	}
        } else
        	return 0;

        return dataSize;
	}

	private class TalkOutputStream extends OutputStream {
		ByteBuffer data = ByteBuffer.allocate(1024);
		boolean stopped = false;
		int writePosition = 0;

		public int read() throws IOException {
			if (!stopped)


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
				// Move the cursor to the end of the byte array to append
				data.position(writePosition);
				if (b.length < data.remaining()) {
					data.put(b);
					writePosition = data.position();
				}
				data.notify();
			}
		}

		public void packetizeData() {
			ByteBuffer packet;
			synchronized(data) {
				// Determine how much data to put in the packet
				int size = Math.min(writePosition, MAX_HID_PACKET_SIZE - 2);
				if (size <= 0)
					return;

				// Format into a HID packet
				packet = ByteBuffer.allocate(MAX_HID_PACKET_SIZE);
				packet.put(0,(byte) 2);             // Report ID
				packet.put(1,(byte) size);          // The number of bytes of data
				data.position(0);
				data.get(packet.array(), 2, size);  // Copy data into the other array

				// Remove that data from the write buffer
				data.compact();
				writePosition -= size;
				if (DEBUG) Log.d(TAG, "packetizeData(): size="+size);
			}
			WriteToDevice(packet);
		}

	};
	private static final int MAX_HID_PACKET_SIZE = 64;

	/**
	 * Send a packet or wait for data to be available
	 */
	public void sendData() {
		TalkOutputStream o = (TalkOutputStream) outStream;
		synchronized(o.data){
			if (o.writePosition > 0)
				o.packetizeData();
			else {
				o.data.notify();
				o.packetizeData();
			}
		}
	}

	UsbRequest writeRequest = null;
	boolean WriteToDevice(ByteBuffer DataToSend) {
		if (DEBUG) Log.d(TAG, "Writing to device()");

		//The report must be formatted correctly for the device being connected to. On some devices, this requires that a specific value must be the first byte in the report. This can be followed by the length of the data in the report. This format is determined by the device, and isn't specified here.

		int bufferDataLength = usbEndpointWrite.getMaxPacketSize();
		ByteBuffer buffer = ByteBuffer.allocate(bufferDataLength + 1);

		if(writeRequest == null) {
			writeRequest = new UsbRequest();
			writeRequest.initialize(connectionWrite, usbEndpointWrite);
		}

		buffer.put(DataToSend);

		writeRequest.queue(buffer, bufferDataLength);
		try
		{
			if (writeRequest.equals(connectionWrite.requestWait()))
				return true;
		}
		catch (Exception ex)
		{
			// An exception has occured
			return false;
		}

		if (false) {
			writeRequest.cancel();
			writeRequest.close();
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
