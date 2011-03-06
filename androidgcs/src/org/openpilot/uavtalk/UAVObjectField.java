package org.openpilot.uavtalk;

import java.io.Serializable;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

public class UAVObjectField {
	
    public enum FieldType { INT8, INT16, INT32, UINT8, UINT16, UINT32, FLOAT32, ENUM, STRING };

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
        clear();
    	
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
     * @throws Exception */
    public int pack(ByteBuffer dataOut) throws Exception {
        //QMutexLocker locker(obj->getMutex());
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
                	Integer val = (Integer) getValue(index);
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
                	dataOut.put((Byte) l.get(index));
                break;
            case STRING:
            	// TODO: Implement strings
            	throw new Exception("Strings not yet implemented");
        }
        // Done
        return getNumBytes();    	
    }
    
    public int unpack(ByteBuffer dataIn) {
        // Unpack each element from input buffer
    	dataIn.order(ByteOrder.LITTLE_ENDIAN);
        switch (type)
        {
            case INT8:
            {
            	List<Integer> l = (List<Integer>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		Byte val = dataIn.get();
            		l.set(index, val.intValue());
            	}
                break;
            }
            case INT16:
            {
            	List<Integer> l = (List<Integer>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		Short val = dataIn.getShort();
            		l.set(index, val.intValue());
            	}
                break;
            }
            case INT32:
            {
            	List<Integer> l = (List<Integer>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		Integer val = dataIn.getInt();
            		l.set(index, val.intValue());
            	}
                break;
            }
            case UINT8:
            	// TOOD: Deal with unsigned
            {
            	List<Integer> l = (List<Integer>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		Byte val = dataIn.get();
            		l.set(index, val.intValue());
            	}
                break;
            }
            case UINT16:
            {
            	List<Integer> l = (List<Integer>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		Short val = dataIn.getShort();
            		l.set(index, val.intValue());
            	}
                break;
            }
            case UINT32:
            {
            	List<Integer> l = (List<Integer>) this.data;
            	for (int index = 0 ; index < numElements; ++index) {
            		Integer val = dataIn.getInt();
            		l.set(index, val.intValue());
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
            case ENUM:
            {
        		List<Byte> l = (List<Byte>) data;
            	for (int index = 0 ; index < numElements; ++index) {
            		l.set(index, dataIn.get(index));
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
    
    Object getValue()  { return getValue(0); };
    Object getValue(int index)  {
//        QMutexLocker locker(obj->getMutex());
        // Check that index is not out of bounds
        if ( index >= numElements )
        {
            return null;
        }

        switch (type)
        {
            case INT8:
            case INT16:
            case INT32:
            {
            	List<Integer> l = (List<Integer>) data;
            	return l.get(index);
            }
            case UINT8:
            case UINT16:
            case UINT32:
            {
            	// TODO: Correctly deal with unsigned values
            	List<Integer> l = (List<Integer>) data;
            	return l.get(index);
            }
            case FLOAT32:
            {
            	List<Float> l = (List<Float>) data;
            	return l.get(index);
            }
            case ENUM:
            {
            	List<Byte> l = (List<Byte>) data;
            	Byte val = l.get(index);

                //if(val >= options.size() || val < 0) 
                //	throw new Exception("Invalid value for" + name);

                return options.get(val);
            }
            case STRING:
            {
            	//throw new Exception("Shit I should do this");
            }
        }
        // If this point is reached then we got an invalid type
        return null;    
    }
    
    public void setValue(Object data) { setValue(data,0); }    
    public void setValue(Object data, int index) {
    	//    	   QMutexLocker locker(obj->getMutex());
    	// Check that index is not out of bounds
    	if ( index >= numElements );
    		//throw new Exception("Index out of bounds");

    	// Get metadata
    	UAVObject.Metadata mdata = obj.getMetadata();
    	// Update value if the access mode permits
    	if ( mdata.gcsAccess == UAVObject.AccessMode.ACCESS_READWRITE )
    	{
    		switch (type)
    		{
    		case INT8:
    		case INT16:
    		case INT32:
    		{
    			List<Integer> l = (List<Integer>) this.data;
    			l.set(index,((Number) data).intValue());
    			break;
    		}
    		case UINT8:
    		case UINT16:
    		case UINT32:
    		{
    			List<Integer> l = (List<Integer>) this.data;
    			l.set(index,((Number) data).intValue());
    			break;
    		}
    		case FLOAT32:
    		{
    			List<Float> l = (List<Float>) this.data;
    			l.set(index, (Float) data);
    			break;
    		}
    		case ENUM:
    		{
    			byte val = (byte) options.indexOf((String) data);
    			//if(val < 0) throw new Exception("Enumerated value not found");    	            	
    			List<Byte> l = (List<Byte>) this.data;
    			l.set(index, val);
    			break;
    		}
    		case STRING: 
    		{
    			//throw new Exception("Sorry I haven't implemented strings yet");
    		}
    		}
    	}
    }
    
    public double getDouble() throws Exception { return getDouble(0); };
    public double getDouble(int index) throws Exception {
    	return Double.valueOf((Double) getValue(index));
    }
    
    public void setDouble(double value) throws Exception { setDouble(value, 0); };
    public void setDouble(double value, int index) throws Exception {
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
            case STRING:
                return true;
            default:
                return false;
        }    	
    }
    
    public String toString() {
        String sout = new String();
        sout += name + ": [ ";
        for (int n = 0; n < numElements; ++n)
        {
        	sout += String.valueOf(n) + "(" + getValue(n) + ") ";
        }
        
        sout += "] " + units + "\n";
        return sout;    	
    }

    void fieldUpdated(UAVObjectField field) {
    	
    }

    public void clear() {
    	switch (type)
        {
            case INT8:
            case INT16:
            case INT32:
            case UINT8:
            case UINT16:
            case UINT32:
            	((ArrayList<Integer>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Integer>) data).add(0);
            	}
                break;
            case FLOAT32:
            	((ArrayList<Float>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Float>) data).add((float) 0);
            	}
                break;
            case ENUM:
            	((ArrayList<Byte>) data).clear();
            	for(int index = 0; index < numElements; ++index) {
            		((ArrayList<Byte>) data).add((byte)0);
            	}
                break;
            case STRING:
            	// TODO: Add string
                break;
            default:
                numBytesPerElement = 0;
        }
    }
    
    public void constructorInitialize(String name, String units, FieldType type, List<String> elementNames, List<String> options) {
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
            	data = (Object) new ArrayList<Integer>(this.numElements);
                numBytesPerElement = 1;
                break;
            case INT16:
            	data = (Object) new ArrayList<Integer>(this.numElements);
                numBytesPerElement = 2;
                break;
            case INT32:
            	data = (Object) new ArrayList<Integer>(this.numElements);
                numBytesPerElement = 4;
                break;
            case UINT8:
            	data = (Object) new ArrayList<Integer>(this.numElements);
                numBytesPerElement = 1;
                break;
            case UINT16:
            	data = (Object) new ArrayList<Integer>(this.numElements);
                numBytesPerElement = 2;
                break;
            case UINT32:
            	data = (Object) new ArrayList<Integer>(this.numElements);
                numBytesPerElement = 4;
                break;
            case FLOAT32:
            	data = (Object) new ArrayList<Double>(this.numElements);
                numBytesPerElement = 4;
                break;
            case ENUM:
            	data = (Object) new ArrayList<Byte>(this.numElements);
                numBytesPerElement = 1;
                break;
            case STRING:
            	data = (Object) new ArrayList<String>(this.numElements);
                numBytesPerElement = 1;
                break;
            default:
                numBytesPerElement = 0;
        }
        clear();
    }
    

	private String name;
	private String units;
	private FieldType type;
	private List<String> elementNames;
	private List<String> options;
    private int numElements;
    private int numBytesPerElement;
    private int offset;
    private Object data;
    private UAVObject obj;

}
