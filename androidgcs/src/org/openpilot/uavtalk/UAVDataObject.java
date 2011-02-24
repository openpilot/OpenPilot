package org.openpilot.uavtalk;

public abstract class UAVDataObject extends UAVObject {
	
	public UAVDataObject(int objID, Boolean isSingleInst, Boolean isSet, String name) {
		super(objID, isSingleInst, name);
	}
	
	public void initialize(int instID, UAVMetaObject mobj) {

	}
	
	public void initialize(UAVMetaObject mobj) {
	    
	}
	
	Boolean isSettings() {
		return null;
	}
	
    UAVMetaObject getMetaObject() {
		return null;	
    }
	    
    public abstract UAVDataObject clone(int instID);

}
