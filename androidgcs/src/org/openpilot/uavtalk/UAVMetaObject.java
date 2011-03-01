package org.openpilot.uavtalk;

public class UAVMetaObject extends UAVObject {

	private UAVDataObject parent;
	
	public UAVMetaObject(int objID, String mname, UAVDataObject parent) {
		super(objID, true, mname);
		this.parent = parent;
	}

	@Override
	public void deserialize(byte[] arr, int offset) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public UAVMetaObject getDefaultMetadata() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getDescription() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public int getObjID() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public String getObjName() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public byte[] serialize() {
		// TODO Auto-generated method stub
		return null;
	}

}
