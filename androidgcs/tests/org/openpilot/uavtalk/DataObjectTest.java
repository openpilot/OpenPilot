package org.openpilot.uavtalk;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;
import java.util.Observer;

import org.junit.BeforeClass;
import org.junit.Test;
import org.openpilot.uavtalk.uavobjects.ActuatorCommand;
import org.openpilot.uavtalk.uavobjects.UAVObjectsInitialize;

import android.database.Observable;

public class DataObjectTest {

 boolean succeed = false;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@Test
	public void testUpdatedObserver() {
		
		succeed = false;
		
		UAVObject obj = new ActuatorCommand();
		obj.addUpdatedObserver( new Observer() {

			public void update(java.util.Observable observable, Object data) {
				System.out.println("Updated: " + data.toString());				
				succeed = true;
			}
		});
		obj.updated();
		
		if(!succeed)
			fail("No callback");
		
		System.out.println("Done");

	}

	@Test
	public void testUpdatedViaObjMngr() {
		succeed = false;
		
		UAVObjectManager objMngr = new UAVObjectManager();
		UAVObjectsInitialize.register(objMngr);

		UAVObject obj = objMngr.getObject("FlightTelemetryStats");
		obj.addUpdatedObserver( new Observer() {

			public void update(java.util.Observable observable, Object data) {
				System.out.println("Updated: " + data.toString());				
				succeed = true;
			}
		});
		objMngr.getObject("FlightTelemetryStats").updated();	

		if(!succeed)
			fail("No callback");
		System.out.println("Done");

	}
	
	@Test
	public void testUnpackedViaObjMngr() {
		succeed = false;
		
		UAVObjectManager objMngr = new UAVObjectManager();
		UAVObjectsInitialize.register(objMngr);

		UAVObject obj = objMngr.getObject("FlightTelemetryStats");
		obj.addUnpackedObserver( new Observer() {

			public void update(java.util.Observable observable, Object data) {
				System.out.println("Updated: " + data.toString());				
				succeed = true;
			}
		});
		
		ByteBuffer bbuf = ByteBuffer.allocate(obj.getNumBytes());
		objMngr.getObject("FlightTelemetryStats").unpack(bbuf);

		if(!succeed)
			fail("No callback");

		System.out.println("Done");

	}


}
