package org.openpilot.uavtalk;
import static org.junit.Assert.*;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.Observable;
import java.util.Observer;

import org.junit.BeforeClass;
import org.junit.Test;
import org.openpilot.uavtalk.uavobjects.UAVObjectsInitialize;
import org.openpilot.uavtalk.UAVTalk;


public class TalkTest {

	static UAVObjectManager objMngr;
	static final String IP_ADDRDESS = new String("127.0.0.1");
	static final int PORT_NUM = 7777;
	boolean succeed = false;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		objMngr = new UAVObjectManager();
		UAVObjectsInitialize.register(objMngr);
	}

	//@Test
	public void testGetFlightStatus() {
		Socket connection = null;
		UAVTalk talk = null;
		try{
			InetAddress ip = InetAddress.getByName(IP_ADDRDESS);
			connection = new Socket(ip, PORT_NUM);
		} catch (Exception e) {
			e.printStackTrace();
			fail("Couldn't connect to test platform");
		}
		
		try {
			talk = new UAVTalk(connection.getInputStream(), connection.getOutputStream(), objMngr);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			fail("Couldn't construct UAVTalk object");
		}
		
		Thread inputStream = talk.getInputProcessThread();
		inputStream.start();

		succeed = false;
		
		UAVObject obj = objMngr.getObject("FlightTelemetryStats");

		obj.addUpdatedObserver( new Observer() {
			public void update(Observable observable, Object data) {
				// TODO Auto-generated method stub
				System.out.println("Updated: " + data.toString());
				succeed = true;
			}
		});
		
		try {
			Thread.sleep(1000);
		} catch (InterruptedException e1) {
		}
		
		if(!succeed)
			fail("Never received a FlightTelemetryStats update");

	}

	@Test
	public void testSendObjectRequest() {
		ByteArrayInputStream is = new ByteArrayInputStream(new byte[0], 0, 0);
		ByteArrayOutputStream os = new ByteArrayOutputStream(100);
		
		UAVTalk talk = new UAVTalk(is,os,objMngr);
		UAVObject obj = objMngr.getObject("FlightTelemetryStats");
		obj.getField("Status").setValue("Connected");
		
		talk.sendObject(obj, false, false);
		
		System.out.println("Size: " + os.size());
		byte [] array = os.toByteArray();
		for(int i = 0; i < array.length; i++) {
			System.out.print("0x" + Integer.toHexString((int) array[i] & 0xff));
			if(i != array.length-1)
				System.out.print(", ");
		}
		System.out.print("\n");
	}
	
	@Test
	public void testReceiveObject() {
		ByteArrayInputStream is = new ByteArrayInputStream(new byte[0], 0, 0);
		ByteArrayOutputStream os = new ByteArrayOutputStream(100);
		
		// Send object to create the test packet (should hard code in test string)
		UAVTalk talk = new UAVTalk(is,os,objMngr);
		UAVObject obj = objMngr.getObject("FlightTelemetryStats");
		obj.getField("Status").setValue("Connected");
		talk.sendObject(obj, false, false);
		
		obj.getField("Status").setValue("Disconnected");
		
		// Test receiving from that stream
		is = new ByteArrayInputStream(os.toByteArray(), 0, os.size());
		talk = new UAVTalk(is,os,objMngr);
		Thread inputStream = talk.getInputProcessThread();
		inputStream.start();
		
		System.out.println("Should be FlightTelemetry Stats:");
		System.out.println(objMngr.getObject("FlightTelemetryStats").toString());

		fail("Not working yet");
	}


}
