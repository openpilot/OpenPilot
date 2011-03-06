package org.openpilot.uavtalk;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

public class UAVMetaObject extends UAVObject {

	public UAVMetaObject(int objID, String name, UAVDataObject parent) throws Exception {
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

		// Setup fields
		List<String> boolEnum = new ArrayList<String>();
		boolEnum.add("False");
		boolEnum.add("True");

		List<String> updateModeEnum = new ArrayList<String>();
		updateModeEnum.add("Periodic");
		updateModeEnum.add("On Change");
		updateModeEnum.add("Manual");
		updateModeEnum.add("Never");

		List<String> accessModeEnum = new ArrayList<String>();	    
		accessModeEnum.add("Read/Write");
		accessModeEnum.add("Read Only");

		List<UAVObjectField> fields = new ArrayList<UAVObjectField>();    
		fields.add( new UAVObjectField("Flight Access Mode", "", UAVObjectField.FieldType.ENUM, 1, accessModeEnum) );
		fields.add( new UAVObjectField("GCS Access Mode", "", UAVObjectField.FieldType.ENUM, 1, accessModeEnum) );
		fields.add( new UAVObjectField("Flight Telemetry Acked", "", UAVObjectField.FieldType.ENUM, 1, boolEnum) );
		fields.add( new UAVObjectField("Flight Telemetry Update Mode", "", UAVObjectField.FieldType.ENUM, 1, updateModeEnum) );
		fields.add( new UAVObjectField("Flight Telemetry Update Period", "", UAVObjectField.FieldType.UINT32, 1, null) );
		fields.add( new UAVObjectField("GCS Telemetry Acked", "", UAVObjectField.FieldType.ENUM, 1, boolEnum) );
		fields.add( new UAVObjectField("GCS Telemetry Update Mode", "", UAVObjectField.FieldType.ENUM, 1, updateModeEnum) );
		fields.add( new UAVObjectField("GCS Telemetry Update Period", "", UAVObjectField.FieldType.UINT32, 1, null ) );
		fields.add( new UAVObjectField("Logging Update Mode", "", UAVObjectField.FieldType.ENUM, 1, updateModeEnum) );
		fields.add( new UAVObjectField("Logging Update Period", "", UAVObjectField.FieldType.UINT32, 1, null ) );

		// Initialize parent
		super.initialize(0);
		super.initializeFields(fields, ByteBuffer.allocate(10), 10);

		// Setup metadata of parent
		parentMetadata = parent.getDefaultMetadata();	
	}

	/**
	 * Get the parent object
	 */
	public UAVObject getParentObject()
	{
		return parent;
	}

	/**
	 * Set the metadata of the metaobject, this function will
	 * do nothing since metaobjects have read-only metadata.
	 */
	public void setMetadata(Metadata mdata)
	{
		return; // can not update metaobject's metadata
	}

	/**
	 * Get the metadata of the metaobject
	 */
	public Metadata getMetadata()
	{
		return ownMetadata;
	}

	/**
	 * Get the default metadata
	 */
	public Metadata getDefaultMetadata()
	{
		return ownMetadata;
	}

	/**
	 * Set the metadata held by the metaobject
	 */
	public void setData(Metadata mdata)
	{
		//QMutexLocker locker(mutex);
		parentMetadata = mdata;
		// TODO: Callbacks
		//	    emit objectUpdatedAuto(this); // trigger object updated event
		//	    emit objectUpdated(this);
	}

	/**
	 * Get the metadata held by the metaobject
	 */
	public Metadata getData()
	{
//		QMutexLocker locker(mutex);
		return parentMetadata;
	}


	private UAVObject parent;
	private Metadata ownMetadata;
	private Metadata parentMetadata;


}
