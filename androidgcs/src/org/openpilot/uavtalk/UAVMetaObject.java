/**
 ******************************************************************************
 * @file       UAVMetaObject.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Base object for all UAVO meta data
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
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;

public class UAVMetaObject extends UAVObject {

	public UAVMetaObject(long objID, String name, UAVDataObject parent) throws Exception {
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
	@Override
	public void setMetadata(Metadata mdata)
	{
		return; // can not update metaobject's metadata
	}

	/**
	 * Get the metadata of the metaobject
	 */
	@Override
	public Metadata getMetadata()
	{
		return ownMetadata;
	}

	/**
	 * Get the default metadata
	 */
	@Override
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


	private final UAVObject parent;
	private final Metadata ownMetadata;
	private Metadata parentMetadata;


}
