package org.openpilot.uavtalk;

import static org.junit.Assert.fail;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;

import org.junit.Test;
import org.openpilot.uavtalk.uavobjects.UAVObjectsInitialize;

import android.os.Looper;


public class TelemetryMonitorTest {

	static UAVObjectManager objMngr;
	static UAVTalk talk;
	static final String IP_ADDRDESS = new String("127.0.0.1");
	static final int PORT_NUM = 7777;
	static Socket connection = null;
	boolean succeed = false;

	@Test
	public void testTelemetry() throws Exception {
		objMngr = new UAVObjectManager();
		UAVObjectsInitialize.register(objMngr);
		talk = null;
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

		Looper.prepare();
		Telemetry tel = new Telemetry(talk, objMngr, Looper.myLooper());
		@SuppressWarnings("unused")
		TelemetryMonitor mon = new TelemetryMonitor(objMngr,tel);

		Thread.sleep(10000);

		System.out.println("Done");
	}


}
