package org.openpilot.uavtalk;

public class UAVMetaObject extends UAVObject {

	public UAVMetaObject(int objID, String name, UAVDataObject parent) {
		super(objID, true, name);
		this.parent = parent;
		
	    ownMetadata.flightAccess = UAVObject.AccessMode.ACCESS_READWRITE;
	    ownMetadata.gcsAccess = UAVObject.AccessMode.ACCESS_READWRITE;
	    ownMetadata.flightTelemetryAcked = UAVObject.Acked.TRUE;
	    ownMetadata.flightTelemetryUpdateMode = UAVObject.UpdateMode.UPDATEMODE_ONCHANGE;
	    ownMetadata.flightTelemetryUpdatePeriod = 0;
	    ownMetadata.gcsTelemetryAcked = UAVObject.Acked.TRUE;
	    ownMetadata.gcsTelemetryUpdateMode = UAVObject.UpdateMode.UPDATEMODE_ONCHANGE;
	    ownMetadata.gcsTelemetryUpdatePeriod = 0;
	    ownMetadata.loggingUpdateMode = UAVObject.UpdateMode.UPDATEMODE_ONCHANGE;
	    ownMetadata.loggingUpdatePeriod = 0;
		
	}

	UAVObject getParentObject() {
		return parent;
	}

	@Override
	public void deserialize(byte[] arr, int offset) {
		// TODO Auto-generated method stub

	}

	@Override
	public Metadata getMetadata() {
		return ownMetadata;
	}

	@Override
	public Metadata getDefaultMetadata() {
	    return ownMetadata;
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

	
	public UAVMetaObject(quint32 objID, const QString& name, UAVObject* parent);
	UAVObject* getParentObject();
	void setMetadata(const Metadata& mdata);
	void setData(const Metadata& mdata);
	Metadata getData();

	
	private UAVObject parent;
	private Metadata ownMetadata;
	private Metadata parentMetadata;


}
