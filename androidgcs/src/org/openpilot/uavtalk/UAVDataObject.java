package org.openpilot.uavtalk;

public abstract class UAVDataObject extends UAVObject {
	
	/**
	 * @brief Constructor for UAVDataObject
	 * @param objID the object id to be created
	 * @param isSingleInst 
	 * @param isSet
	 * @param name
	 */
	public UAVDataObject(int objID, Boolean isSingleInst, Boolean isSet, String name) {
		super(objID, isSingleInst, name);
	}
	
	public abstract void initialize(int instID, UAVMetaObject mobj);
	
	public abstract void initialize(UAVMetaObject mobj);
	
	Boolean isSettings() {
		return null;
	}
	
    UAVMetaObject getMetaObject() {
		return null;	
    }
	    
    public UAVDataObject clone(int instID) {
    	try {
			return (UAVDataObject) super.clone();
		} catch (CloneNotSupportedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
    }
    
public:
	static int OBJID = $(OBJIDHEX);
    static String NAME;
    static String DESCRIPTION;
    static boolean ISSINGLEINST = $(ISSINGLEINST);
    static boolean ISSETTINGS = $(ISSETTINGS);
    static int NUMBYTES = sizeof(DataFields);


}
