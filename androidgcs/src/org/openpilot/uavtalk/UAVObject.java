package org.openpilot.uavtalk;

import java.nio.ByteBuffer;
import java.util.List;
import java.util.ListIterator;

public abstract class UAVObject {

	/**
	 * Object update mode
	 */
	public enum UpdateMode {
		UPDATEMODE_PERIODIC, /**
		 * Automatically update object at periodic
		 * intervals
		 */
		UPDATEMODE_ONCHANGE, /** Only update object when its data changes */
		UPDATEMODE_MANUAL, /**
		 * Manually update object, by calling the updated()
		 * function
		 */
		UPDATEMODE_NEVER
		/** Object is never updated */
	};

	/**
	 * Access mode
	 */
	public enum AccessMode {
		ACCESS_READWRITE, ACCESS_READONLY
	};

	/**
	 * Access mode
	 */
	public enum Acked {
		FALSE, TRUE
	};

	public final class Metadata {
		public AccessMode flightAccess;
		/**
		 * Defines the access level for the local flight transactions (readonly
		 * and readwrite)
		 */
		public AccessMode gcsAccess;
		/**
		 * Defines the access level for the local GCS transactions (readonly and
		 * readwrite)
		 */
		public Acked flightTelemetryAcked;
		/**
		 * Defines if an ack is required for the transactions of this object
		 * (1:acked, 0:not acked)
		 */
		public UpdateMode flightTelemetryUpdateMode;
		/** Update mode used by the autopilot (UpdateMode) */
		public int flightTelemetryUpdatePeriod;
		/**
		 * Update period used by the autopilot (only if telemetry mode is
		 * PERIODIC)
		 */
		public Acked gcsTelemetryAcked;
		/**
		 * Defines if an ack is required for the transactions of this object
		 * (1:acked, 0:not acked)
		 */
		public UpdateMode gcsTelemetryUpdateMode;
		/** Update mode used by the GCS (UpdateMode) */
		public int gcsTelemetryUpdatePeriod;
		/** Update period used by the GCS (only if telemetry mode is PERIODIC) */
		public UpdateMode loggingUpdateMode;
		/** Update mode used by the logging module (UpdateMode) */
		public int loggingUpdatePeriod;
		/**
		 * Update period used by the logging module (only if logging mode is
		 * PERIODIC)
		 */
	};

	public UAVObject(int objID, Boolean isSingleInst, String name) {
		this.objID = objID;
		this.instID = 0;
		this.isSingleInst = isSingleInst;
		this.name = name;
		// this.mutex = new QMutex(QMutex::Recursive);
	};

	public void initialize(int instID) {
		// QMutexLocker locker(mutex);
		this.instID = instID;
	}

	/**
	 * Initialize objects' data fields
	 * 
	 * @param fields
	 *            List of fields held by the object
	 * @param data
	 *            Pointer to that actual object data, this is needed by the
	 *            fields to access the data
	 * @param numBytes
	 *            Number of bytes in the object (total, including all fields)
	 * @throws Exception
	 *             When unable to unpack a field
	 */
	public void initializeFields(List<UAVObjectField> fields, ByteBuffer data,
			int numBytes) {
		// TODO: QMutexLocker locker(mutex);
		this.numBytes = numBytes;
		this.fields = fields;
		// Initialize fields
		for (int n = 0; n < fields.size(); ++n) {
			fields.get(n).initialize(this);
		}
		unpack(data);
	}

	/**
	 * Get the object ID
	 */
	public int getObjID() {
		return objID;
	}

	/**
	 * Get the instance ID
	 */
	public int getInstID() {
		return instID;
	}

	/**
	 * Returns true if this is a single instance object
	 */
	public boolean isSingleInstance() {
		return isSingleInst;
	}

	/**
	 * Get the name of the object
	 */
	public String getName() {
		return name;
	}

	/**
	 * Get the description of the object
	 * 
	 * @return The description of the object
	 */
	public String getDescription() {
		return description;
	}

	/**
	 * Set the description of the object
	 * 
	 * @param The
	 *            description of the object
	 * @return
	 */
	public void setDescription(String description) {
		this.description = description;
	}

	/**
	 * Get the total number of bytes of the object's data
	 */
	public int getNumBytes() {
		return numBytes;
	}

	// /**
	// * Request that this object is updated with the latest values from the
	// autopilot
	// */
	// /* void UAVObject::requestUpdate()
	// {
	// emit updateRequested(this);
	// } */
	//
	// /**
	// * Signal that the object has been updated
	// */
	// /* void UAVObject::updated()
	// {
	// emit objectUpdatedManual(this);
	// emit objectUpdated(this);
	// } */
	//
	// /**
	// * Lock mutex of this object
	// */
	// /* void UAVObject::lock()
	// {
	// mutex->lock();
	// } */
	//
	// /**
	// * Lock mutex of this object
	// */
	// /* void UAVObject::lock(int timeoutMs)
	// {
	// mutex->tryLock(timeoutMs);
	// } */
	//
	// /**
	// * Unlock mutex of this object
	// */
	// /* void UAVObject::unlock()
	// {
	// mutex->unlock();
	// } */
	//
	// /**
	// * Get object's mutex
	// */
	// QMutex* UAVObject::getMutex()
	// {
	// return mutex;
	// }

	/**
	 * Get the number of fields held by this object
	 */
	public int getNumFields() {
		// QMutexLocker locker(mutex);
		return fields.size();
	}

	/**
	 * Get the object's fields
	 */
	public List<UAVObjectField> getFields() {
		// QMutexLocker locker(mutex);
		return fields;
	}

	/**
	 * Get a specific field
	 * 
	 * @throws Exception
	 * @returns The field or NULL if not found
	 */
	public UAVObjectField getField(String name) {
		// QMutexLocker locker(mutex);
		// Look for field
		ListIterator<UAVObjectField> li = fields.listIterator();
		while (li.hasNext()) {
			UAVObjectField field = li.next();
			if (field.getName().equals(name))
				return field;
		}
		//throw new Exception("Field not found");
		return null;
	}

	/**
	 * Pack the object data into a byte array
	 * 
	 * @param dataOut
	 *            ByteBuffer to receive the data.
	 * @throws Exception
	 * @returns The number of bytes copied
	 * @note The array must already have enough space allocated for the object
	 */
	public int pack(ByteBuffer dataOut) throws Exception {
		// QMutexLocker locker(mutex);
		if (dataOut.remaining() < getNumBytes())
			throw new Exception("Not enough bytes in ByteBuffer to pack object");
		int numBytes = 0;

		ListIterator<UAVObjectField> li = fields.listIterator();
		while (li.hasNext()) {
			UAVObjectField field = li.next();
			numBytes += field.pack(dataOut);
		}
		return numBytes;
	}

	/**
	 * Unpack the object data from a byte array
	 * 
	 * @param dataIn
	 *            The ByteBuffer to pull data from
	 * @throws Exception
	 * @returns The number of bytes copied
	 */
	public int unpack(ByteBuffer dataIn) {
		if( dataIn == null )
			return 0;

		// QMutexLocker locker(mutex);
		int numBytes = 0;
		ListIterator<UAVObjectField> li = fields.listIterator();
		while (li.hasNext()) {
			UAVObjectField field = li.next();
			numBytes += field.unpack(dataIn);
		}
		return numBytes;
		// TODO: Callbacks
		// emit objectUnpacked(this); // trigger object updated event
		// emit objectUpdated(this);
	}

	// /**
	// * Save the object data to the file.
	// * The file will be created in the current directory
	// * and its name will be the same as the object with
	// * the .uavobj extension.
	// * @returns True on success, false on failure
	// */
	// bool UAVObject::save()
	// {
	// QMutexLocker locker(mutex);
	//
	// // Open file
	// QFile file(name + ".uavobj");
	// if (!file.open(QFile::WriteOnly))
	// {
	// return false;
	// }
	//
	// // Write object
	// if ( !save(file) )
	// {
	// return false;
	// }
	//
	// // Close file
	// file.close();
	// return true;
	// }
	//
	// /**
	// * Save the object data to the file.
	// * The file is expected to be already open for writting.
	// * The data will be appended and the file will not be closed.
	// * @returns True on success, false on failure
	// */
	// bool UAVObject::save(QFile& file)
	// {
	// QMutexLocker locker(mutex);
	// quint8 buffer[numBytes];
	// quint8 tmpId[4];
	//
	// // Write the object ID
	// qToLittleEndian<quint32>(objID, tmpId);
	// if ( file.write((const char*)tmpId, 4) == -1 )
	// {
	// return false;
	// }
	//
	// // Write the instance ID
	// if (!isSingleInst)
	// {
	// qToLittleEndian<quint16>(instID, tmpId);
	// if ( file.write((const char*)tmpId, 2) == -1 )
	// {
	// return false;
	// }
	// }
	//
	// // Write the data
	// pack(buffer);
	// if ( file.write((const char*)buffer, numBytes) == -1 )
	// {
	// return false;
	// }
	//
	// // Done
	// return true;
	// }
	//
	// /**
	// * Load the object data from a file.
	// * The file will be openned in the current directory
	// * and its name will be the same as the object with
	// * the .uavobj extension.
	// * @returns True on success, false on failure
	// */
	// bool UAVObject::load()
	// {
	// QMutexLocker locker(mutex);
	//
	// // Open file
	// QFile file(name + ".uavobj");
	// if (!file.open(QFile::ReadOnly))
	// {
	// return false;
	// }
	//
	// // Load object
	// if ( !load(file) )
	// {
	// return false;
	// }
	//
	// // Close file
	// file.close();
	// return true;
	// }
	//
	// /**
	// * Load the object data from file.
	// * The file is expected to be already open for reading.
	// * The data will be read and the file will not be closed.
	// * @returns True on success, false on failure
	// */
	// bool UAVObject::load(QFile& file)
	// {
	// QMutexLocker locker(mutex);
	// quint8 buffer[numBytes];
	// quint8 tmpId[4];
	//
	// // Read the object ID
	// if ( file.read((char*)tmpId, 4) != 4 )
	// {
	// return false;
	// }
	//
	// // Check that the IDs match
	// if (qFromLittleEndian<quint32>(tmpId) != objID)
	// {
	// return false;
	// }
	//
	// // Read the instance ID
	// if ( file.read((char*)tmpId, 2) != 2 )
	// {
	// return false;
	// }
	//
	// // Check that the IDs match
	// if (qFromLittleEndian<quint16>(tmpId) != instID)
	// {
	// return false;
	// }
	//
	// // Read and unpack the data
	// if ( file.read((char*)buffer, numBytes) != numBytes )
	// {
	// return false;
	// }
	// unpack(buffer);
	//
	// // Done
	// return true;
	// }

	/**
	 * Return a string with the object information
	 */
	@Override
	public String toString() {
		return toStringBrief() + toStringData();
	}

	/**
	 * Return a string with the object information (only the header)
	 */
	public String toStringBrief() {
		return getName() + " ( " + Integer.toHexString(getObjID()) + " " + Integer.toHexString(getInstID()) + " " + getNumBytes() + ")\n";
	}

	/**
	 * Return a string with the object information (only the data)
	 */
	public String toStringData() {
		String s = new String();
		ListIterator<UAVObjectField> li = fields.listIterator();
		while (li.hasNext()) {
			UAVObjectField field = li.next();
			s += field.toString();
		}
		return s;
	}

	// /**
	// * Emit the transactionCompleted event (used by the UAVTalk plugin)
	// */
	// void UAVObject::emitTransactionCompleted(bool success)
	// {
	// emit transactionCompleted(this, success);
	// }

	/**
	 * Java specific functions
	 */
	public UAVObject clone() {
		return (UAVObject) clone();
	}

	/**
	 * Abstract functions
	 */
	public abstract void setMetadata(Metadata mdata);

	public abstract Metadata getMetadata();

	public abstract Metadata getDefaultMetadata();

	/**
	 * Private data for the object, common to all
	 */
	protected int objID;
	protected int instID;
	protected boolean isSingleInst;
	protected String name;
	protected String description;
	protected int numBytes;
	// TODO: QMutex* mutex;
//	protected ByteBuffer data;
	protected List<UAVObjectField> fields;
}
