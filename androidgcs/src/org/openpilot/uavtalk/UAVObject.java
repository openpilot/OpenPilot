/**
 ******************************************************************************
 * @file       UAVObject.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Base object for UAVDataObject and UAVMetaObject.
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
import java.util.Observable;
import java.util.Observer;

public abstract class UAVObject {

	public class CallbackListener extends Observable {
		private final UAVObject parent;

		public CallbackListener(UAVObject parent) {
			this.parent = parent;
		}

		public void event () {
			synchronized(this) {
				setChanged();
				notifyObservers(parent);
			}
		}
		public void event (Object data) {
			synchronized(this) {
				setChanged();
				notifyObservers(data);
			}
		}
	}

	public class TransactionResult {
		public UAVObject obj;
		public boolean success;
		public TransactionResult(UAVObject obj, boolean success) {
			this.obj = obj;
			this.success = success;
		}
	}

	private final CallbackListener transactionCompletedListeners = new CallbackListener(this);
	public void addTransactionCompleted(Observer o) {
		synchronized(transactionCompletedListeners) {
			transactionCompletedListeners.addObserver(o);
		}
	}
	public void removeTransactionCompleted(Observer o) {
		synchronized(transactionCompletedListeners) {
			transactionCompletedListeners.deleteObserver(o);
		}
	}
	void transactionCompleted(boolean status) {
		synchronized(transactionCompletedListeners) {
			transactionCompletedListeners.event(new TransactionResult(this,status));
		}
	}

	private final CallbackListener updatedListeners = new CallbackListener(this);
	public void removeUpdatedObserver(Observer o) {
		synchronized(updatedListeners) {
			updatedListeners.deleteObserver(o);
		}
	}
	public void addUpdatedObserver(Observer o) {
		synchronized(updatedListeners) {
			updatedListeners.addObserver(o);
		}
	}
	void updated(boolean manually) {
		synchronized(updatedListeners) {
			updatedListeners.event();
		}
		if(manually)
			updatedManual();
	}
	public void updated() { updated(true); };

	private final CallbackListener unpackedListeners = new CallbackListener(this);
	public void addUnpackedObserver(Observer o) {
		synchronized(unpackedListeners) {
			unpackedListeners.addObserver(o);
		}
	}
	public void removeUnpackedObserver(Observer o) {
		synchronized(unpackedListeners) {
			unpackedListeners.deleteObserver(o);
		}
	}
	void unpacked() {
		synchronized(unpackedListeners) {
			unpackedListeners.event();
		}
	}

	private final CallbackListener updatedAutoListeners = new CallbackListener(this);
	public void addUpdatedAutoObserver(Observer o) {
		synchronized(updatedAutoListeners) {
			updatedAutoListeners.addObserver(o);
		}
	}
	public void removeUpdatedAutoObserver(Observer o) {
		synchronized(updatedAutoListeners) {
			updatedAutoListeners.deleteObserver(o);
		}
	}
	void updatedAuto() {
		synchronized(updatedAutoListeners) {
			updatedAutoListeners.event();
		}
	}

	private final CallbackListener updatedManualListeners = new CallbackListener(this);
	public void addUpdatedManualObserver(Observer o) {
		synchronized(updatedManualListeners) {
			updatedManualListeners.addObserver(o);
		}
	}
	public void removeUpdatedManualObserver(Observer o) {
		synchronized(updatedManualListeners) {
			updatedManualListeners.deleteObserver(o);
		}
	}
	void updatedManual() {
		synchronized(updatedManualListeners) {
			updatedManualListeners.event();
		}
	}

	private final CallbackListener updateRequestedListeners = new CallbackListener(this);
	public void addUpdateRequestedObserver(Observer o) {
		synchronized(updateRequestedListeners) {
			updateRequestedListeners.addObserver(o);
		}
	}
	public void removeUpdateRequestedObserver(Observer o) {
		synchronized(updateRequestedListeners) {
			updateRequestedListeners.deleteObserver(o);
		}
	}
	public void updateRequested() {
		synchronized(updateRequestedListeners) {
			updateRequestedListeners.event();
		}
	}

	public abstract boolean isMetadata();

	/**
	 * Object update mode
	 */
	public enum UpdateMode {
		UPDATEMODE_MANUAL,   /** Manually update object, by calling the updated() function */
		UPDATEMODE_PERIODIC, /** Automatically update object at periodic intervals */
		UPDATEMODE_ONCHANGE, /** Only update object when its data changes */
		UPDATEMODE_THROTTLED /** Object is updated on change, but not more often than the interval time */
	};

	/**
	 * Access mode
	 */
	public enum AccessMode {
		ACCESS_READWRITE, ACCESS_READONLY
	};

	public final static int UAVOBJ_ACCESS_SHIFT = 0;
	public final static int UAVOBJ_GCS_ACCESS_SHIFT = 1;
	public final static int UAVOBJ_TELEMETRY_ACKED_SHIFT = 2;
	public final static int UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT = 3;
	public final static int UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT = 4;
	public final static int UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT = 6;
	public final static int UAVOBJ_UPDATE_MODE_MASK = 0x3;

	public final static class Metadata {
		/**
		 * Object metadata, each object has a meta object that holds its metadata. The metadata define
		 * properties for each object and can be used by multiple modules (e.g. telemetry and logger)
		 *
		 * The object metadata flags are packed into a single 16 bit integer.
		 * The bits in the flag field are defined as:
		 *
		 *   Bit(s)  Name                       Meaning
		 *   ------  ----                       -------
		 *      0    access                     Defines the access level for the local transactions (readonly=0 and readwrite=1)
		 *      1    gcsAccess                  Defines the access level for the local GCS transactions (readonly=0 and readwrite=1), not used in the flight s/w
		 *      2    telemetryAcked             Defines if an ack is required for the transactions of this object (1:acked, 0:not acked)
		 *      3    gcsTelemetryAcked          Defines if an ack is required for the transactions of this object (1:acked, 0:not acked)
		 *    4-5    telemetryUpdateMode        Update mode used by the telemetry module (UAVObjUpdateMode)
		 *    6-7    gcsTelemetryUpdateMode     Update mode used by the GCS (UAVObjUpdateMode)
		 */
		public int flags; /** Defines flags for update and logging modes and whether an update should be ACK'd (bits defined above) */

		/** Update period used by the telemetry module (only if telemetry mode is PERIODIC) */
		public int flightTelemetryUpdatePeriod;

		/** Update period used by the GCS (only if telemetry mode is PERIODIC) */
		public int gcsTelemetryUpdatePeriod;

		/** Update period used by the GCS (only if telemetry mode is PERIODIC) */
		public int loggingUpdatePeriod;
		/**
		 * Update period used by the logging module (only if logging mode is
		 * PERIODIC)
		 */

		/**
		 * @brief Helper method for metadata accessors
		 * @param var The starting value
		 * @param shift The offset of these bits
		 * @param value The new value
		 * @param mask The mask of these bits
		 * @return
		 */
		private void SET_BITS(int shift, int value, int mask) {
			this.flags = (this.flags & ~(mask << shift)) |	(value << shift);
		}

		/**
		 * Get the UAVObject metadata access member
		 * \return the access type
		 */
		public AccessMode GetFlightAccess()
		{
			return AccessModeEnum((this.flags >> UAVOBJ_ACCESS_SHIFT) & 1);
		}

		/**
		 * Set the UAVObject metadata access member
		 * \param[in] mode The access mode
		 */
		public void SetFlightAccess(Metadata metadata, AccessMode mode)
		{
			// Need to convert java enums which have no numeric value to bits
			SET_BITS(UAVOBJ_ACCESS_SHIFT, AccessModeNum(mode), 1);
		}

		/**
		 * Get the UAVObject metadata GCS access member
		 * \return the GCS access type
		 */
		public AccessMode GetGcsAccess()
		{
			return AccessModeEnum((this.flags >> UAVOBJ_GCS_ACCESS_SHIFT) & 1);
		}

		/**
		 * Set the UAVObject metadata GCS access member
		 * \param[in] mode The access mode
		 */
		public void SetGcsAccess(Metadata metadata, AccessMode mode) {
			// Need to convert java enums which have no numeric value to bits
			SET_BITS(UAVOBJ_GCS_ACCESS_SHIFT, AccessModeNum(mode), 1);
		}

		/**
		 * Get the UAVObject metadata telemetry acked member
		 * \return the telemetry acked boolean
		 */
		public boolean GetFlightTelemetryAcked() {
			return (((this.flags >> UAVOBJ_TELEMETRY_ACKED_SHIFT) & 1) == 1);
		}

		/**
		 * Set the UAVObject metadata telemetry acked member
		 * \param[in] val The telemetry acked boolean
		 */
		public void SetFlightTelemetryAcked(boolean val) {
			SET_BITS(UAVOBJ_TELEMETRY_ACKED_SHIFT, val ? 1 : 0, 1);
		}

		/**
		 * Get the UAVObject metadata GCS telemetry acked member
		 * \return the telemetry acked boolean
		 */
		public boolean GetGcsTelemetryAcked() {
			return ((this.flags >> UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT) & 1) == 1;
		}

		/**
		 * Set the UAVObject metadata GCS telemetry acked member
		 * \param[in] val The GCS telemetry acked boolean
		 */
		public void SetGcsTelemetryAcked(boolean val) {
			SET_BITS(UAVOBJ_GCS_TELEMETRY_ACKED_SHIFT, val ? 1 : 0, 1);
		}

		/**
		 * Maps from the bitfield number to the symbolic java enumeration
		 * @param num The value in the bitfield after shifting
		 * @return The update mode
		 */
		public static AccessMode AccessModeEnum(int num) {
			switch(num) {
			case 0:
				return AccessMode.ACCESS_READONLY;
			case 1:
				return AccessMode.ACCESS_READWRITE;
			}
			return AccessMode.ACCESS_READONLY;
		}

		/**
		 * Maps from the java symbolic enumeration of update mode to the bitfield value
		 * @param e The update mode
		 * @return The numeric value to use on the wire
		 */
		public static int AccessModeNum(AccessMode e) {
			switch(e) {
			case ACCESS_READONLY:
				return 0;
			case ACCESS_READWRITE:
				return 1;
			}
			return 0;
		}

		/**
		 * Maps from the bitfield number to the symbolic java enumeration
		 * @param num The value in the bitfield after shifting
		 * @return The update mode
		 */
		public static UpdateMode UpdateModeEnum(int num) {
			switch(num) {
			case 0:
				return UpdateMode.UPDATEMODE_MANUAL;
			case 1:
				return UpdateMode.UPDATEMODE_PERIODIC;
			case 2:
				return UpdateMode.UPDATEMODE_ONCHANGE;
			default:
				return UpdateMode.UPDATEMODE_THROTTLED;
			}
		}

		/**
		 * Maps from the java symbolic enumeration of update mode to the bitfield value
		 * @param e The update mode
		 * @return The numeric value to use on the wire
		 */
		public static int UpdateModeNum(UpdateMode e) {
			switch(e) {
			case UPDATEMODE_MANUAL:
				return 0;
			case UPDATEMODE_PERIODIC:
				return 1;
			case UPDATEMODE_ONCHANGE:
				return 2;
			case UPDATEMODE_THROTTLED:
				return 3;
			}
			return 0;
		}

		/**
		 * Get the UAVObject metadata telemetry update mode
		 * \return the telemetry update mode
		 */
		public UpdateMode GetFlightTelemetryUpdateMode() {
			return UpdateModeEnum((this.flags >> UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT) & UAVOBJ_UPDATE_MODE_MASK);
		}

		/**
		 * Set the UAVObject metadata telemetry update mode member
		 * \param[in] metadata The metadata object
		 * \param[in] val The telemetry update mode
		 */
		public void SetFlightTelemetryUpdateMode(UpdateMode val) {
			SET_BITS(UAVOBJ_TELEMETRY_UPDATE_MODE_SHIFT, UpdateModeNum(val), UAVOBJ_UPDATE_MODE_MASK);
		}

		/**
		 * Get the UAVObject metadata GCS telemetry update mode
		 * \param[in] metadata The metadata object
		 * \return the GCS telemetry update mode
		 */
		public UpdateMode GetGcsTelemetryUpdateMode() {
			return UpdateModeEnum((this.flags >> UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT) & UAVOBJ_UPDATE_MODE_MASK);
		}

		/**
		 * Set the UAVObject metadata GCS telemetry update mode member
		 * \param[in] metadata The metadata object
		 * \param[in] val The GCS telemetry update mode
		 */
		public void SetGcsTelemetryUpdateMode(UpdateMode val) {
			SET_BITS(UAVOBJ_GCS_TELEMETRY_UPDATE_MODE_SHIFT, UpdateModeNum(val), UAVOBJ_UPDATE_MODE_MASK);
		}

	};

	public UAVObject(long objID, Boolean isSingleInst, String name) {
		this.objID = objID;
		this.instID = 0;
		this.isSingleInst = isSingleInst;
		this.name = name;
		// this.mutex = new QMutex(QMutex::Recursive);
	};

	public synchronized void initialize(long instID) {
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
	public synchronized void initializeFields(List<UAVObjectField> fields, ByteBuffer data,
			int numBytes) {
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
	public long getObjID() {
		return objID;
	}

	/**
	 * Get the instance ID
	 */
	public long getInstID() {
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
		return fields.size();
	}

	/**
	 * Get the object's fields
	 */
	public synchronized List<UAVObjectField> getFields() {
		return fields;
	}

	/**
	 * Get a specific field
	 *
	 * @throws Exception
	 * @returns The field or NULL if not found
	 */
	public UAVObjectField getField(String name) {
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
	public synchronized int pack(ByteBuffer dataOut) throws Exception {
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
	public synchronized int unpack(ByteBuffer dataIn) {
		if( dataIn == null )
			return 0;

		// QMutexLocker locker(mutex);
		int numBytes = 0;
		ListIterator<UAVObjectField> li = fields.listIterator();
		while (li.hasNext()) {
			UAVObjectField field = li.next();
			numBytes += field.unpack(dataIn);
		}

		// Trigger all the listeners for the unpack event
		unpacked();
		updated(false);

		return numBytes;
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
		return toStringBrief(); // + toStringData();
	}

	/**
	 * Return a string with the object information (only the header)
	 */
	public String toStringBrief() {
		return getName(); // + " (" + Integer.toHexString(getObjID()) + " " + Integer.toHexString(getInstID()) + " " + getNumBytes() + ")\n";
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
	@Override
	public synchronized UAVObject clone() {
		UAVObject newObj = clone();
		List<UAVObjectField> newFields = new ArrayList<UAVObjectField>();
		ListIterator<UAVObjectField> li = fields.listIterator();
		while(li.hasNext()) {
			UAVObjectField nf = li.next().clone();
			nf.initialize(newObj);
			newFields.add(nf);
		}
		newObj.initializeFields(newFields, ByteBuffer.allocate(numBytes), numBytes);
		return newObj;
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
	protected long objID;
	protected long instID;
	protected boolean isSingleInst;
	protected String name;
	protected String description;
	protected int numBytes;
	// TODO: QMutex* mutex;
//	protected ByteBuffer data;
	protected List<UAVObjectField> fields;
}
