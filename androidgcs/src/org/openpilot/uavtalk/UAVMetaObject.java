package org.openpilot.uavtalk;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;

public class UAVMetaObject extends UAVObject {

	public UAVMetaObject(int objID, String name, UAVDataObject parent) throws Exception {
		super(objID, true, name);
		this.parent = parent;
		
		ownMetadata = new Metadata();

		ownMetadata.flags = 0; // TODO: Fix flags
		ownMetadata.gcsTelemetryUpdatePeriod = 0;
		ownMetadata.loggingUpdatePeriod = 0;


		List<String> modesBitField = new ArrayList<String>();
		modesBitField.add("FlightReadOnly");
		modesBitField.add("GCSReadOnly");
		modesBitField.add("FlightTelemetryAcked");
		modesBitField.add("GCSTelemetryAcked");
		modesBitField.add("FlightUpdatePeriodic");
		modesBitField.add("FlightUpdateOnChange");
		modesBitField.add("GCSUpdatePeriodic");
		modesBitField.add("GCSUpdateOnChange");
		
		List<UAVObjectField> fields = new ArrayList<UAVObjectField>();
		fields.add( new UAVObjectField("Modes", "", UAVObjectField.FieldType.BITFIELD, 1, modesBitField) );
	    fields.add( new UAVObjectField("Flight Telemetry Update Period", "ms", UAVObjectField.FieldType.UINT16, 1, null) );
	    fields.add( new UAVObjectField("GCS Telemetry Update Period", "ms", UAVObjectField.FieldType.UINT16, 1, null) );
	    fields.add( new UAVObjectField("Logging Update Period", "ms", UAVObjectField.FieldType.UINT16, 1, null) );

		int numBytes = 0;
		ListIterator<UAVObjectField> li = fields.listIterator();
		while(li.hasNext()) {
			numBytes += li.next().getNumBytes();
		}

		// Initialize object

		// Initialize parent
		super.initialize(0);
		initializeFields(fields, ByteBuffer.allocate(numBytes), numBytes);

		// Setup metadata of parent
		parentMetadata = parent.getDefaultMetadata();	
	}
	
	@Override
	public boolean isMetadata() { 
		return true; 
	};

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
