/**
 ******************************************************************************
 * @file       Telemetry.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Implementation of all the UAVObjectFields.
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
package org.openpilot.uavtalk;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

public class UAVObjectField {

    public enum FieldType { INT8, INT16, INT32, UINT8, UINT16, UINT32, FLOAT32, ENUM, BITFIELD, STRING };

    public UAVObjectField(String name, String units, FieldType type, int numElements, List<String> options) {
        List<String> elementNames = new ArrayList<String>();
        // Set element names
        for (int n = 0; n < numElements; ++n)
        {
            elementNames.add(String.valueOf(n));
        }
        // Initialize
        constructorInitialize(name, units, type, elementNames, options);
    }

    public UAVObjectField(String name, String units, FieldType type, List<String>  elementNames, List<String>  options) {
    	 constructorInitialize(name, units, type, elementNames, options);
    }

    public void initialize(UAVObject obj){
         this.obj = obj;
        //clear();
	}

    public UAVObject getObject() {
    	return obj;
    }

    public FieldType getType() {
    	return type;
    }

    public String getTypeAsString() {
        switch (type)
        {
            case INT8:
                return "int8";
            case INT16:
                return "int16";
            case INT32:
                return "int32";
            case UINT8:
                return "uint8";
            case UINT16:
                return "uint16";
            case UINT32:
                return "uint32";
            case FLOAT32:
                return "float32";
            case ENUM:
                return "enum";
            case BITFIELD:
            	return "bitfield";
            case STRING:
                return "string";
            default:
                return "";
        }
    }

    public String getName() {
    	return name;
    }

    public String getUnits() {
    	return units;
    }

    public int getNumElements() {
    	return numElements;
    }

    public List<String> getElementNames() {
    	return elementNames;
    }

    public List<String> getOptions() {
    	return options;
    }

    /**
     * This function copies this field from the internal storage of the parent object
     * to a new ByteBuffer for UAVTalk.  It also converts from the java standard (big endian)
     * to the arm/uavtalk standard (little endian)
     * @param dataOut
     * @return the number of bytes added
     **/
    @SuppressWarnings("unchecked")
	public synchronized int pack(ByteBuffer dataOut) {
        // Pack each element in output buffer
    	dataOut.order(ByteOrder.LITTLE_ENDIAN);
        switch (type)
        {
            case INT8:
            	for (int index = 0; index < numElements; ++index) {
            		Integer val = (Integer) getValue(index);
            		dataOut.put(val.byteValue());
            	}
                break;
            case INT16:
                for (int index = 0; index < numElements; ++index) {
                	Integer val = (Integer) getValue(index);
                	dataOut.putShort(val.shortValue());
                }
                break;
            case INT32:
                for (int index = 0; index < numElements; ++index) {
                	Integer val = (Integer) getValue(index);
                	dataOut.putInt(val);
                }
                break;
            case UINT8:
            	// TODO: Deal properly with unsigned
            	for (int index = 0; index < numElements; ++index) {
            		Integer val = (Integer) getValue(index);
            		dataOut.put(val.byteValue());
            	}
                break;
            case UINT16:
            	// TODO: Deal properly with unsigned
                for (int index = 0; index < numElements; ++index) {
                	Integer val = (Integer) getValue(index);
                	dataOut.putShort(val.shortValue());
                }
                break;
            case UINT32:
            	// TODO: Deal properly with unsigned
                for (int index = 0; index < numElements; ++index) {
                	Integer val = (int) ( ((Long) getValue(index)).longValue() & 0xffffffffL);
                	dataOut.putInt(val);
                }
                break;
            case FLOAT32:
                for (int index = 0; index < numElements; ++index)
                	dataOut.putFloat((Float) getValue(index));
                break;
            case ENUM:
            	List<Byte> l = (List<Byte>) data;
                for (int index = 0; index < numElements; ++index)
                	dataOut.put(l.get(index));
                break;
            case BITFIELD:
            	for (int index = 0; index < numElements; ++index) {
            		Integer val = (Integer) getValue(index);
            		dataOut.put(val.byteValue());
            	}
            	break;
            case STRING:
            	// TODO: Implement strings
            	throw new Error("Strings not yet implemented.  Field name: " + getName());
        }
        // Done
        return getNumBytes();
    }

    @SuppressWarnings("unchecked")
	public synchronized int unpack(ByteBuffer dataIn) {
        // Unpack each element from input buffer
    	dataIn.order(ByteOrder.LITTLE_ENDIAN);
        switch (type)
        {
            case INT8:
            {
            	List<Byte> l = (List<Byte>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		Long val = bound(dataIn.get());
            		l.set(index, val.byteValue());
            	}
                break;
            }
            case INT16:
            {
            	List<Short> l = (List<Short>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		Long val = bound(dataIn.getShort());
            		l.set(index, val.shortValue());
            	}
                break;
            }
            case INT32:
            {
            	List<Integer> l = (List<Integer>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		Long val = bound(dataIn.getInt());
            		l.set(index, val.intValue());
            	}
                break;
            }
            case UINT8:
            {
            	List<Short> l = (List<Short>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		int signedval = dataIn.get();  // this sign extends it
            		int unsignedval = signedval & 0xff;    // drop sign extension
            		l.set(index, (short) unsignedval);
            	}
                break;
            }
            case UINT16:
            {
            	List<Integer> l = (List<Integer>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		int signedval = dataIn.getShort();  // this sign extends it
            		int unsignedval = signedval & 0xffff;    // drop sign extension
            		l.set(index, unsignedval);
            	}
                break;
            }
            case UINT32:
            {
            	List<Long> l = (List<Long>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		long signedval = dataIn.getInt();  // this sign extends it
            		long unsignedval = signedval & 0xffffffffL;    // drop sign extension
            		l.set(index, unsignedval);
            	}
                break;
            }
            case FLOAT32:
            {
            	List<Float> l = (List<Float>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		Float val = dataIn.getFloat();
            		l.set(index, val);
            	}
                break;
            }
            case BITFIELD:
            {
            	List<Short> l = (List<Short>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		int signedval = dataIn.get();  // this sign extends it
            		int unsignedval = signedval & 0xff;    // drop sign extension
            		l.set(index, (short) unsignedval);
            	}
                break;
            }
            case ENUM:
            {
        		List<Byte> l = (List<Byte>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		l.set(index, dataIn.get());
            	}
                break;
            }
            case STRING:
            	// TODO: implement strings
            	//throw new Exception("Strings not handled");
        }
        // Done
        return getNumBytes();
    }

    public Object getValue()  { return getValue(0); };
    @SuppressWarnings("unchecked")
	public synchronized Object getValue(int index)  {
        // Check that index is not out of bounds
        if ( index >= numElements )
        {
            return null;
        }

        switch (type)
        {
            case INT8:
            	return ((List<Byte>) data).get(index).intValue();
            case INT16:
            	return ((List<Short>) data).get(index).intValue();
            case INT32:
            	return ((List<Integer>) data).get(index).intValue();
            case UINT8:
            	return ((List<Short>) data).get(index).intValue();
            case UINT16:
            	return ((List<Integer>) data).get(index).intValue();
            case UINT32:
            	return ((List<Long>) data).get(index);
            case FLOAT32:
            	return ((List<Float>) data).get(index);
            case ENUM:
            {
            	List<Byte> l = (List<Byte>) data;
            	Byte val = l.get(index);

                //if(val >= options.size() || val < 0)
                //	throw new Exception("Invalid value for" + name);

                return options.get(val);
            }
            case BITFIELD:
            	return ((List<Short>) data).get(index).intValue();

            case STRING:
            {
            	//throw new Exception("Shit I should do this");
            }
        }
        // If this point is reached then we got an invalid type
        return null;
    }

    public void setValue(Object data) { setValue(data,0); }
    @SuppressWarnings("unchecked")
	public synchronized void setValue(Object data, int index) {
    	// Check that index is not out of bounds
    	//if ( index >= numElements );
    		//throw new Exception("Index out of bounds");

    	// Get metadata
    	UAVObject.Metadata mdata = obj.getMetadata();
    	// Update value if the access mode permits
    	if ( mdata.GetGcsAccess() == UAVObject.AccessMode.ACCESS_READWRITE )
    	{
    		switch (type)
    		{
    		case INT8:
    		{
    			List<Byte> l = (List<Byte>) this.data;
    			l.set(index, bound(data).byteValue());
    			break;
    		}
    		case INT16:
    		{
    			List<Short> l = (List<Short>) this.data;
    			l.set(index, bound(data).shortValue());
    			break;
    		}
    		case INT32:
    		{
    			List<Integer> l = (List<Integer>) this.data;
    			l.set(index, bound(data).intValue());
    			break;
    		}
    		case UINT8:
    		{
    			List<Short> l = (List<Short>) this.data;
    			l.set(index, bound(data).shortValue());
    			break;
    		}
    		case UINT16:
    		{
    			List<Integer> l = (List<Integer>) this.data;
    			l.set(index, bound(data).intValue());
    			break;
    		}
    		case UINT32:
    		{
    			List<Long> l = (List<Long>) this.data;
    			l.set(index, bound(data));
    			break;
    		}
    		case FLOAT32:
    		{
    			List<Float> l = (List<Float>) this.data;
    			l.set(index, ((Number) data).floatValue());
    			break;
    		}
    		case ENUM:
    		{
    			byte val;
    			try {
    				// Test if numeric constant passed in
    				val = ((Number) data).byteValue();
    			} catch (Exception e) {
    				val = (byte) options.indexOf(data);
    			}
    			//if(val < 0) throw new Exception("Enumerated value not found");
    			List<Byte> l = (List<Byte>) this.data;
    			l.set(index, val);
    			break;
    		}
    		case BITFIELD:
    		{
    			List<Short> l = (List<Short>) this.data;
    			l.set(index, bound(data).shortValue());
    			break;
    		}
    		case STRING:
    		{
    			//throw new Exception("Sorry I haven't implemented strings yet");
    		}
    		}
    		//obj.updated();
    	}
    }

    public double getDouble() { return getDouble(0); };
    @SuppressWarnings("unchecked")
	public double getDouble(int index) {
    	switch (type) {
    	case ENUM:
    		return ((List<Byte>)data).get(index);
    	default:
    		break;
    	}
    	return ((Number) getValue(index)).doubleValue();
    }

    public void setDouble(double value) { setDouble(value, 0); };
    public void setDouble(double value, int index) {
    	setValue(value, index);
    }

    public int getDataOffset() {
    	return offset;
    }

    public int getNumBytes() {
        return numBytesPerElement * numElements;
    }

    public int getNumBytesElement() {
    	return numBytesPerElement;
    }

    public boolean isNumeric() {
        switch (type)
        {
            case INT8:
                return true;
            case INT16:
                return true;
            case INT32:
                return true;
            case UINT8:
                return true;
            case UINT16:
                return true;
            case UINT32:
                return true;
            case FLOAT32:
                return true;
            case ENUM:
                return false;
            case BITFIELD:
                return true;
            case STRING:
                return false;
            default:
                return false;
        }
    }

    public boolean isText() {
        switch (type)
        {
            case INT8:
                return false;
            case INT16:
                return false;
            case INT32:
                return false;
            case UINT8:
                return false;
            case UINT16:
                return false;
            case UINT32:
                return false;
            case FLOAT32:
                return false;
            case ENUM:
                return true;
            case BITFIELD:
                return false;
            case STRING:
                return true;
            default:
                return false;
        }
    }

	@Override
	public String toString() {
        String sout = new String();
        sout += name + ": ";
        for (int i = 0; i < numElements; i++) {
        	sout += getValue(i).toString();
        	if (i != numElements-1)
        		sout += ", ";
        	else
        		sout += " ";
        }
    	if (units.length() > 0)
    		sout += " (" + units + ")\n";
    	else
    		sout += "\n";
        return sout;
    }

    void fieldUpdated(UAVObjectField field) {

    }

    @SuppressWarnings("unchecked")
	public synchronized void clear() {
    	switch (type)
        {
            case INT8:
            	((ArrayList<Byte>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Byte>) data).add((byte) 0);
            	}
                break;
            case INT16:
            	((ArrayList<Short>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Short>) data).add((short) 0);
            	}
                break;
            case INT32:
            	((ArrayList<Integer>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Integer>) data).add(0);
            	}
                break;
            case UINT8:
            	((ArrayList<Short>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Short>) data).add((short) 0);
            	}
                break;
            case UINT16:
            	((ArrayList<Integer>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Integer>) data).add(0);
            	}
                break;
            case UINT32:
            	((ArrayList<Long>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Long>) data).add((long) 0);
            	}
                break;
            case FLOAT32:
            	((ArrayList<Float>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Float>) data).add((float) 0);
            	}
                break;
            case BITFIELD:
            	((ArrayList<Short>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Short>) data).add((short) 0);
            	}
                break;
            case ENUM:
            	((ArrayList<Byte>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Byte>) data).add((byte) 0);
            	}
                break;
            case STRING:
            	((ArrayList<Byte>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Byte>) data).add((byte) 0);
            	}
                break;
        }
    }

    public synchronized void constructorInitialize(String name, String units, FieldType type, List<String> elementNames, List<String> options) {
        // Copy params
        this.name = name;
        this.units = units;
        this.type = type;
        this.options = options;
        this.numElements = elementNames.size();
        this.offset = 0;
        this.data = null;
        this.obj = null;
        this.elementNames = elementNames;

        // Set field size
        switch (type)
        {
            case INT8:
            	data = new ArrayList<Byte>(this.numElements);
                numBytesPerElement = 1;
                break;
            case INT16:
            	data = new ArrayList<Short>(this.numElements);
                numBytesPerElement = 2;
                break;
            case INT32:
            	data = new ArrayList<Integer>(this.numElements);
                numBytesPerElement = 4;
                break;
            case UINT8:
            	data = new ArrayList<Short>(this.numElements);
                numBytesPerElement = 1;
                break;
            case UINT16:
            	data = new ArrayList<Integer>(this.numElements);
                numBytesPerElement = 2;
                break;
            case UINT32:
            	data = new ArrayList<Long>(this.numElements);
                numBytesPerElement = 4;
                break;
            case FLOAT32:
            	data = new ArrayList<Float>(this.numElements);
                numBytesPerElement = 4;
                break;
            case ENUM:
            	data = new ArrayList<Byte>(this.numElements);
                numBytesPerElement = 1;
                break;
            case BITFIELD:
            	data = new ArrayList<Short>(this.numElements);
                numBytesPerElement = 1;
                break;
            case STRING:
            	data = new ArrayList<Byte>(this.numElements);
                numBytesPerElement = 1;
                break;
            default:
                numBytesPerElement = 0;
        }
        clear();
    }

    /**
     * For numerical types bounds the data appropriately
     * @param val Can be any object, for numerical tries to cast to Number
     * @return long value with the right range (for float rounds)
     * @note This is mostly needed because java has no unsigned integer
     */
    protected Long bound (Object val) {

    	long num = 0;
    	if (isNumeric())
    		num = ((Number) val).longValue();

    	switch(type) {
    	case INT8:
    		if(num < Byte.MIN_VALUE)
    			return (long) Byte.MAX_VALUE;
    		if(num > Byte.MAX_VALUE)
    			return (long) Byte.MAX_VALUE;
    		return num;
    	case INT16:
    		if(num < Short.MIN_VALUE)
    			return (long) Short.MIN_VALUE;
    		if(num > Short.MAX_VALUE)
    			return (long) Short.MAX_VALUE;
    		return num;
    	case INT32:
    		if(num < Integer.MIN_VALUE)
    			return (long) Integer.MIN_VALUE;
    		if(num > Integer.MAX_VALUE)
    			return (long) Integer.MAX_VALUE;
    		return num;
    	case UINT8:
    		if(num < 0)
    			return (long) 0;
    		if(num > 255)
    			return (long) 255;
    		return num;
    	case UINT16:
    		if(num < 0)
    			return (long) 0;
    		if(num > 65535)
    			return (long) 65535;
    		return num;
    	case UINT32:
    		if(num < 0)
    			return (long) 0;
    		if(num > 4294967295L)
    			return 4294967295L;
    		return num;
    	case BITFIELD:
    		if(num < 0)
    			return (long) 0;
    		if(num > 255)
    			return (long) 255;
    		return num;
    	case FLOAT32:
    		return ((Number) val).longValue();
    	case ENUM:
    	case STRING:
    		return 0L;
    	}

    	return num;
    }

    @Override
    public UAVObjectField clone()
    {
    	UAVObjectField newField = new UAVObjectField(new String(name), new String(units), type,
    			new ArrayList<String>(elementNames),
    			new ArrayList<String>(options));
    	newField.initialize(obj);
    	newField.data = data;
		return newField;
    }

	private String name;
	private String units;
	private FieldType type;
	private List<String> elementNames;
	private List<String> options;
    private int numElements;
    private int numBytesPerElement;
    private int offset;
    private UAVObject obj;
    protected Object data;

}
