package org.openpilot.uavtalk;

import static org.junit.Assert.*;

import org.junit.BeforeClass;
import org.junit.Test;
import org.openpilot.uavtalk.uavobjects.UAVObjectsInitialize;

public class ObjMngrTest {

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@Test
	public void testGetObjects() {
		UAVObjectManager objMngr = new UAVObjectManager();
		UAVObjectsInitialize.register(objMngr);
//		System.out.println(objMngr.getObject("ActuatorSettings", 0));
	}

}
