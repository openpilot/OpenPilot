package org.openpilot.uavtalk;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import org.junit.BeforeClass;
import org.junit.Test;

import org.openpilot.uavtalk.UAVObjectField;
import org.openpilot.uavtalk.UAVObject;
import org.openpilot.uavtalk.uavobjects.*;

public class FieldTest {

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@Test
	public void testPackUint16() {
		// Need an object initialized to the field to provide metadata
		UAVObject obj = null;
		try {
			obj = new ActuatorCommand();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		UAVObjectField field = new UAVObjectField("TestField", "No Units", UAVObjectField.FieldType.UINT16, 3, null);
		field.initialize(obj);
		field.setValue(3,0);
		field.setValue(-50,1);		
		field.setValue(5003585,2);

		ByteBuffer bbuf = ByteBuffer.allocate(field.getNumBytes());

		try {
			field.pack(bbuf);
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			fail("Buffer size incorrect");
		}
		
		// Expect data to come out as little endian
		byte[] expected = {3,0,0,0,(byte) 0xff,(byte) 0xff};
		for(int i = 0; i < expected.length && i < bbuf.array().length; i++) {
			System.out.println("Expected: " + expected[i] + " (" + i + ")");
			System.out.println("Received: " + bbuf.array()[i] + " (" + i + ")");
			assertEquals(bbuf.array()[i],expected[i]);
		}
	}

	@Test
	public void testUnpackUint16() {
		// Need an object initialized to the field to provide metadata
		UAVObject obj = null;
			obj = new ActuatorCommand();
		UAVObjectField field = new UAVObjectField("TestField", "No Units", UAVObjectField.FieldType.UINT16, 3, null);
		field.initialize(obj);

		ByteBuffer bbuf = ByteBuffer.allocate(field.getNumBytes());
		byte[] expected = {3,0,0,0,(byte) 0xff,(byte) 0xff};
		bbuf.put(expected);
		bbuf.position(0);
		field.unpack(bbuf);
				
		assertEquals(field.getValue(0), 3);
		assertEquals(field.getValue(1), 0);
		assertEquals(field.getValue(2), 65535);
	}

	@Test
	public void testEnumSetGetValue() {
		List<String> options = new ArrayList<String>();
		options.add("Opt1");
		options.add("Opt2");
		options.add("Opt3");

		// Need an object initialized to the field to provide metadata
		UAVObject obj = null;
		try {
			obj = new ActuatorCommand();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		UAVObjectField field = new UAVObjectField("TestField", "No Units", UAVObjectField.FieldType.ENUM, 3, options);
		field.initialize(obj);
		field.setValue("Opt1",0);
		field.setValue("Opt2",1);		
		field.setValue("Opt3",2);
		assertEquals(field.getValue(0), "Opt1");
		assertEquals(field.getValue(1), "Opt2");
		assertEquals(field.getValue(2), "Opt3");
	}

	@Test
	public void testUint16SetGetValue() {

		// Need an object initialized to the field to provide metadata
		UAVObject obj = null;
		try {
			obj = new ActuatorCommand();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		UAVObjectField field = new UAVObjectField("TestField", "No Units", UAVObjectField.FieldType.UINT16, 3, null);
		field.initialize(obj);
		field.setValue(3,0);
		field.setValue(-50,1);		
		field.setValue(5003585,2);
		assertEquals(field.getValue(0), 3);
		assertEquals(field.getValue(1), 0);
		assertEquals(field.getValue(2), 65535);
	}
}
