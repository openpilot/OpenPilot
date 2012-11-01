/**
 ******************************************************************************
 * @file       UAVTalk.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      The protocol layer implementation of UAVTalk.  Serializes objects
 *             for transmission (which is done in the object itself which is aware
 *             of byte packing) wraps that in the UAVTalk packet.  Parses UAVTalk
 *             packets and updates the UAVObjectManager.
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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import junit.framework.Assert;
import android.util.Log;

public class UAVTalk {

	static final String TAG = "UAVTalk";
	public static int LOGLEVEL = 0;
	public static boolean VERBOSE = LOGLEVEL > 3;
	public static boolean WARN = LOGLEVEL > 2;
	public static boolean DEBUG = LOGLEVEL > 1;
	public static boolean ERROR = LOGLEVEL > 0;

	private Thread inputProcessingThread = null;

	/**
	 * A reference to the thread for processing the incoming stream.  Currently this method is ONLY
	 * used for unit testing
	 */
	public Thread getInputProcessThread() {
		if (inputProcessingThread == null)

			inputProcessingThread = new Thread() {
				@Override
				public void run() {
					while(true) {
						try {
							if( !processInputStream() )
								break;
						} catch (IOException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					}
				}
			};
		return inputProcessingThread;
	}

	/**
	 * Constants
	 */
	private static final int SYNC_VAL = 0x3C;

	private static final short crc_table[] = { 0x00, 0x07, 0x0e, 0x09, 0x1c,
			0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
			0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46,
			0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb,
			0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd, 0x90,
			0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1,
			0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5,
			0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0,
			0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93,
			0x94, 0x9d, 0x9a, 0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
			0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50, 0x59,
			0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74,
			0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1,
			0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4, 0xf9, 0xfe, 0xf7, 0xf0,
			0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3,
			0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56,
			0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10, 0x05,
			0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
			0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78,
			0x7f, 0x6a, 0x6d, 0x64, 0x63, 0x3e, 0x39, 0x30, 0x37, 0x22, 0x25,
			0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae,
			0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f,
			0x8a, 0x8d, 0x84, 0x83, 0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc,
			0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3 };

	enum RxStateType {
		STATE_SYNC, STATE_TYPE, STATE_SIZE, STATE_OBJID, STATE_INSTID, STATE_DATA, STATE_CS
	};

	static final int TYPE_MASK = 0xF8;
	static final int TYPE_VER = 0x20;
	//! Packet contains an object
	static final int TYPE_OBJ = (TYPE_VER | 0x00);
	//! Packet is a request for an object
	static final int TYPE_OBJ_REQ = (TYPE_VER | 0x01);
	//! Packet is an object with a request for an ack
	static final int TYPE_OBJ_ACK = (TYPE_VER | 0x02);
	//! Packet is an ack for an object
	static final int TYPE_ACK = (TYPE_VER | 0x03);
	static final int TYPE_NACK = (TYPE_VER | 0x04);

	static final int MIN_HEADER_LENGTH = 8; // sync(1), type (1), size(2),
											// object ID(4)
	static final int MAX_HEADER_LENGTH = 10; // sync(1), type (1), size(2),
												// object ID (4), instance ID(2,
												// not used in single objects)

	static final int CHECKSUM_LENGTH = 1;

	static final int MAX_PAYLOAD_LENGTH = 256;

	static final int MAX_PACKET_LENGTH = (MAX_HEADER_LENGTH
			+ MAX_PAYLOAD_LENGTH + CHECKSUM_LENGTH);

	static final int ALL_INSTANCES = 0xFFFF;
	static final int TX_BUFFER_SIZE = 2 * 1024;

	/**
	 * Private data
	 */
	InputStream inStream;
	OutputStream outStream;
	UAVObjectManager objMngr;

	//! Currently only one UAVTalk transaction is permitted at a time.  If this is null none are in process
	//! otherwise points to the pending object
	UAVObject respObj;
	//! If the pending transaction is for all the instances
	boolean respAllInstances;
	//! The type of response we are expecting
	int respType;

	// Variables used by the receive state machine
	ByteBuffer rxTmpBuffer /* 4 */;
	ByteBuffer rxBuffer;
	int rxType;
	long rxObjId;
	long rxInstId;
	int rxLength;
	int rxPacketLength;

	int rxCSPacket, rxCS;
	int rxCount;
	int packetSize;
	RxStateType rxState;
	ComStats stats = new ComStats();

	/**
	 * Comm stats
	 */
	public class ComStats {
		public int txBytes = 0;
		public int rxBytes = 0;
		public int txObjectBytes = 0;
		public int rxObjectBytes = 0;
		public int rxObjects = 0;
		public int txObjects = 0;
		public int txErrors = 0;
		public int rxErrors = 0;
	}

	/**
	 * Constructor
	 */
	public UAVTalk(InputStream inStream, OutputStream outStream,
			UAVObjectManager objMngr) {
		this.objMngr = objMngr;
		this.inStream = inStream;
		this.outStream = outStream;

		rxState = RxStateType.STATE_SYNC;
		rxPacketLength = 0;

		// mutex = new QMutex(QMutex::Recursive);

		respObj = null;
		resetStats();
		rxTmpBuffer = ByteBuffer.allocate(4);
		rxTmpBuffer.order(ByteOrder.LITTLE_ENDIAN);
		rxBuffer = ByteBuffer.allocate(MAX_PAYLOAD_LENGTH);
		rxBuffer.order(ByteOrder.LITTLE_ENDIAN);

		// TOOD: Callback connect(io, SIGNAL(readyRead()), this,
		// SLOT(processInputStream()));
	}

	/**
	 * Reset the statistics counters
	 */
	public void resetStats() {
		// QMutexLocker locker(mutex);
		stats = new ComStats();
	}

	/**
	 * Get the statistics counters
	 */
	public ComStats getStats() {
		// QMutexLocker locker(mutex);
		return stats;
	}

	/**
	 * Process any data in the queue
	 * @throws IOException
	 */
	public boolean processInputStream() throws IOException {
		int val;

		//inStream.wait();
		val = inStream.read();

		if (VERBOSE) Log.v(TAG, "Read: " + val);

		if (val == -1) {
			return false;
		}

		 processInputByte(val);
		 return true;
	}


	/**
	 * Request an update for the specified object, on success the object data
	 * would have been updated by the GCS. \param[in] obj Object to update
	 * \param[in] allInstances If set true then all instances will be updated
	 * \return Success (true), Failure (false)
	 * @throws IOException
	 */
	public boolean sendObjectRequest(UAVObject obj, boolean allInstances) throws IOException {
		return objectTransaction(obj, TYPE_OBJ_REQ, allInstances);
	}

	/**
	 * Send the specified object through the telemetry link. \param[in] obj
	 * Object to send \param[in] acked Selects if an ack is required \param[in]
	 * allInstances If set true then all instances will be updated \return
	 * Success (true), Failure (false)
	 * @throws IOException
	 */
	public boolean sendObject(UAVObject obj, boolean acked, boolean allInstances) throws IOException {
		if (acked) {
			return objectTransaction(obj, TYPE_OBJ_ACK, allInstances);
		} else {
			return objectTransaction(obj, TYPE_OBJ, allInstances);
		}
	}

	/**
	 * UAVTalk takes care of it's own transactions but if the caller knows
	 * it wants to give up on one (after a timeout) then it can cancel it
	 * @return True if that object was pending, False otherwise
	 */
	public boolean cancelPendingTransaction(UAVObject obj) {
		synchronized (respObj) {
			if(respObj != null && respObj.getObjID() == obj.getObjID()) {
				if(transactionListener != null) {
					Log.d(TAG,"Canceling transaction: " + respObj.getName());
					transactionListener.TransactionFailed(respObj);
				}
				respObj = null;
				return true;
			} else
				return false;
		}
	}

	/**
	 * Cancel a pending transaction.  If there is a pending transaction and
	 * a listener then notify them that the transaction failed.
	 */
	/*private synchronized void cancelPendingTransaction() {
		if(respObj != null && transactionListener != null) {
			Log.d(TAG,"Canceling transaction: " + respObj.getName());
			transactionListener.TransactionFailed(respObj);
		}
		respObj = null;
	}*/

	/**
	 * This is the code that sets up a new UAVTalk packet that expects a response.
	 */
	private void setupTransaction(UAVObject obj, boolean allInstances, int type) {
		synchronized (this) {
			// Only cancel if it is for a different object
			if(respObj != null && respObj.getObjID() != obj.getObjID())
				cancelPendingTransaction(obj);

			respObj = obj;
			respAllInstances = allInstances;
			respType = type;
		}
	}

	/**
	 * Execute the requested transaction on an object. \param[in] obj Object
	 * \param[in] type Transaction type TYPE_OBJ: send object, TYPE_OBJ_REQ:
	 * request object update TYPE_OBJ_ACK: send object with an ack \param[in]
	 * allInstances If set true then all instances will be updated \return
	 * Success (true), Failure (false)
	 * @throws IOException
	 */
	private boolean objectTransaction(UAVObject obj, int type, boolean allInstances) throws IOException {
		if (type == TYPE_OBJ_ACK || type == TYPE_OBJ_REQ || type == TYPE_OBJ) {
			return transmitObject(obj, type, allInstances);
		} else {
			return false;
		}
	}

	/**
	 * Process an byte from the telemetry stream. \param[in] rxbyte Received
	 * byte \return Success (true), Failure (false)
	 * @throws IOException
	 */
	public boolean processInputByte(int rxbyte) throws IOException {
		Assert.assertNotNull(objMngr);

		// Only need to synchronize this method on the state machine state
		synchronized(rxState) {
			// Update stats
			stats.rxBytes++;

			rxPacketLength++; // update packet byte count

			// Receive state machine
			switch (rxState) {
			case STATE_SYNC:

				if (rxbyte != SYNC_VAL)
					break;

				// Initialize and update CRC
				rxCS = updateCRC(0, rxbyte);

				rxPacketLength = 1;

				rxState = RxStateType.STATE_TYPE;
				break;

			case STATE_TYPE:

				// Update CRC
				rxCS = updateCRC(rxCS, rxbyte);

				if ((rxbyte & TYPE_MASK) != TYPE_VER) {
					if (ERROR) Log.e(TAG, "Unknown UAVTalk type:" + rxbyte);
					rxState = RxStateType.STATE_SYNC;
					break;
				}

				rxType = rxbyte;
				if (VERBOSE) Log.v(TAG, "Received packet type: " + rxType);
				packetSize = 0;

				rxState = RxStateType.STATE_SIZE;
				rxCount = 0;
				break;

			case STATE_SIZE:

				// Update CRC
				rxCS = updateCRC(rxCS, rxbyte);

				if (rxCount == 0) {
					packetSize += rxbyte;
					rxCount++;
					break;
				}

				packetSize += (rxbyte << 8) & 0xff00;

				if (packetSize < MIN_HEADER_LENGTH
						|| packetSize > MAX_HEADER_LENGTH + MAX_PAYLOAD_LENGTH) { // incorrect
					// packet
					// size
					rxState = RxStateType.STATE_SYNC;
					break;
				}

				rxCount = 0;
				rxState = RxStateType.STATE_OBJID;
				rxTmpBuffer.position(0);
				break;

			case STATE_OBJID:

				// Update CRC
				rxCS = updateCRC(rxCS, rxbyte);

				rxTmpBuffer.put(rxCount++, (byte) (rxbyte & 0xff));
				if (rxCount < 4)
					break;

				// Search for object, if not found reset state machine
				rxObjId = rxTmpBuffer.getInt(0);
				// Because java treats ints as only signed we need to do this manually
				if (rxObjId < 0)
					rxObjId = 0x100000000l + rxObjId;
				{
					UAVObject rxObj = objMngr.getObject(rxObjId);
					if (rxObj == null) {
						if (WARN) Log.w(TAG, "Unknown ID: " + rxObjId);
						stats.rxErrors++;
						rxState = RxStateType.STATE_SYNC;
						break;
					}

					// Determine data length
					if (rxType == TYPE_OBJ_REQ || rxType == TYPE_ACK || rxType == TYPE_NACK)
						rxLength = 0;
					else
						rxLength = rxObj.getNumBytes();

					// Check length and determine next state
					if (rxLength >= MAX_PAYLOAD_LENGTH) {
						if (WARN) Log.w(TAG, "Greater than max payload length");
						stats.rxErrors++;
						rxState = RxStateType.STATE_SYNC;
						break;
					}

					// Check if this is a single instance object (i.e. if the
					// instance ID field is coming next)
					if (rxObj.isSingleInstance()) {
						// If there is a payload get it, otherwise receive checksum
						if (rxLength > 0)
							rxState = RxStateType.STATE_DATA;
						else
							rxState = RxStateType.STATE_CS;
						rxInstId = 0;
						rxCount = 0;
					} else {
						rxState = RxStateType.STATE_INSTID;
						rxCount = 0;
					}
				}

				break;

			case STATE_INSTID:

				// Update CRC
				rxCS = updateCRC(rxCS, rxbyte);

				rxTmpBuffer.put(rxCount++, (byte) (rxbyte & 0xff));
				if (rxCount < 2)
					break;

				rxInstId = rxTmpBuffer.getShort(0);

				rxCount = 0;

				// If there is a payload get it, otherwise receive checksum
				if (rxLength > 0)
					rxState = RxStateType.STATE_DATA;
				else
					rxState = RxStateType.STATE_CS;

				break;

			case STATE_DATA:

				// Update CRC
				rxCS = updateCRC(rxCS, rxbyte);

				rxBuffer.put(rxCount++, (byte) (rxbyte & 0xff));
				if (rxCount < rxLength)
					break;

				rxState = RxStateType.STATE_CS;
				rxCount = 0;
				break;

			case STATE_CS:

				// The CRC byte
				rxCSPacket = rxbyte;

				if (rxCS != rxCSPacket) { // packet error - faulty CRC
					if (WARN) Log.w(TAG,"Bad crc");
					stats.rxErrors++;
					rxState = RxStateType.STATE_SYNC;
					break;
				}

				if (rxPacketLength != (packetSize + 1)) { // packet error -
					// mismatched packet
					// size
					if (WARN) Log.w(TAG,"Bad size");
					stats.rxErrors++;
					rxState = RxStateType.STATE_SYNC;
					break;
				}

				if (DEBUG) Log.d(TAG,"Received");

				rxBuffer.position(0);
				receiveObject(rxType, rxObjId, rxInstId, rxBuffer);
				stats.rxObjectBytes += rxLength;
				stats.rxObjects++;

				rxState = RxStateType.STATE_SYNC;
				break;

			default:
				if (WARN) Log.w(TAG, "Bad state");
				rxState = RxStateType.STATE_SYNC;
				stats.rxErrors++;
			}
		}

		// Done
		return true;
	}

	/**
	 * Receive an object. This function process objects received through the
	 * telemetry stream. \param[in] type Type of received message (TYPE_OBJ,
	 * TYPE_OBJ_REQ, TYPE_OBJ_ACK, TYPE_ACK) \param[in] obj Handle of the
	 * received object \param[in] instId The instance ID of UAVOBJ_ALL_INSTANCES
	 * for all instances. \param[in] data Data buffer \param[in] length Buffer
	 * length \return Success (true), Failure (false)
	 * @throws IOException
	 */
	public boolean receiveObject(int type, long objId, long instId, ByteBuffer data) throws IOException {

		if (DEBUG) Log.d(TAG, "Received object ID: " + objId);
		assert (objMngr != null);

		UAVObject obj = null;
		boolean error = false;
		boolean allInstances = (instId == ALL_INSTANCES ? true : false);

		// Process message type
		switch (type) {
		case TYPE_OBJ:
			// All instances, not allowed for OBJ messages
			if (!allInstances) {
				if (DEBUG) Log.d(TAG,"Received object: " + objMngr.getObject(objId).getName());

				// Get object and update its data
				obj = updateObject(objId, instId, data);

				if (obj != null) {
					// Check if this is a response to a UAVTalk transaction
					updateObjReq(obj);
				} else {
					error = true;
				}
			} else {
				error = true;
			}
			break;
		case TYPE_OBJ_ACK:
			// All instances, not allowed for OBJ_ACK messages
			if (!allInstances) {
				 if (DEBUG) Log.d(TAG,"Received object ack: " + objId + " " + objMngr.getObject(objId).getName());
				// Get object and update its data
				obj = updateObject(objId, instId, data);
				// Transmit ACK
				if (obj != null) {
					transmitObject(obj, TYPE_ACK, false);
				} else {
					error = true;
				}
			} else {
				error = true;
			}
			break;
		case TYPE_OBJ_REQ:
			// Get object, if all instances are requested get instance 0 of the
			// object
			if (DEBUG) Log.d(TAG,"Received object request: " + objId + " " +
			 objMngr.getObject(objId).getName());
			if (allInstances) {
				obj = objMngr.getObject(objId);
			} else {
				obj = objMngr.getObject(objId, instId);
			}
			// If object was found transmit it
			if (obj != null) {
				transmitObject(obj, TYPE_OBJ, allInstances);
			} else {
				error = true;
			}
			break;
	    case TYPE_NACK:
        	if (DEBUG) Log.d(TAG, "Received NAK: " + objId + " " + objMngr.getObject(objId).getName());
        	// All instances, not allowed for NACK messages
	        if (!allInstances)
	        {
	            // Get object
	            obj = objMngr.getObject(objId, instId);
	            // Check if object exists:
	            if (obj != null)
	            {
	                receivedNack(obj);
	            }
	            else
	            {
	             error = true;
	            }
	        }
	        break;
		case TYPE_ACK:
			// All instances, not allowed for ACK messages
			if (!allInstances) {
				if (DEBUG) Log.d(TAG,"Received ack: " + objId + " " + objMngr.getObject(objId).getName());
				// Get object
				obj = objMngr.getObject(objId, instId);
				// Check if an ack is pending
				if (obj != null) {
					updateAck(obj);
				} else {
					error = true;
				}
			}
			break;
		default:
			error = true;
		}
		// Done
		return !error;
	}

	/**
	 * Update the data of an object from a byte array (unpack). If the object
	 * instance could not be found in the list, then a new one is created.
	 */
	public synchronized UAVObject updateObject(long objId, long instId,
			ByteBuffer data) {
		assert (objMngr != null);

		// Get object
		UAVObject obj = objMngr.getObject(objId, instId);

		// If the instance does not exist create it
		if (obj == null) {
			// Get the object type
			UAVObject tobj = objMngr.getObject(objId);
			if (tobj == null) {
				// TODO: Return a NAK since we don't know this object
				return null;
			}
			// Make sure this is a data object
			UAVDataObject dobj = null;
			try {
				dobj = (UAVDataObject) tobj;
			} catch (Exception e) {
				// Failed to cast to a data object
				return null;
			}

			// Create a new instance, unpack and register
			UAVDataObject instobj = dobj.clone(instId);
			try {
				if (!objMngr.registerObject(instobj)) {
					return null;
				}
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			if (DEBUG) Log.d(TAG, "Unpacking new object");
			instobj.unpack(data);
			return instobj;
		} else {
			// Unpack data into object instance
			if (DEBUG) Log.d(TAG, "Unpacking existing object: " + obj.getName());
			obj.unpack(data);
			return obj;
		}
	}

	/**
	 * Called when an object is received to check if this completes
	 * a UAVTalk transaction
	 */
	private void updateObjReq(UAVObject obj) {
		// Check if this is not a possible candidate
		Assert.assertNotNull(obj);

		boolean succeeded = false;

		// The lock on UAVTalk must be release before the transaction succeeded signal is sent
		// because otherwise if a transaction timeout occurs at the same time we can get a
		// deadlock:
		// 1. processInputStream -> updateObjReq (locks uavtalk) -> tranactionCompleted (locks transInfo)
		// 2. transactionTimeout (locks transInfo) -> sendObjectRequest -> ? -> setupTransaction (locks uavtalk)
		synchronized(this) {
			if(respObj != null && respType == TYPE_OBJ_REQ && respObj.getObjID() == obj.getObjID() &&
					((respObj.getInstID() == obj.getInstID() || !respAllInstances))) {

				// Indicate complete
				respObj = null;
				succeeded = true;
			}
		}

		// Notify listener
		if (succeeded && transactionListener != null)
				transactionListener.TransactionSucceeded(obj);
	}

	/**
	 * Check if a transaction is pending and if yes complete it.
	 */
	private synchronized void receivedNack(UAVObject obj)
	{
		Assert.assertNotNull(obj);
		if(respObj != null && (respType == TYPE_OBJ_REQ || respType == TYPE_OBJ_ACK ) &&
				respObj.getObjID() == obj.getObjID()) {

			if (DEBUG) Log.d(TAG, "NAK: " + obj.getName());

			// Indicate complete
			respObj = null;

			// Notify listener
			if (transactionListener != null)
				transactionListener.TransactionFailed(obj);
		}
	}

	/**
	 * Check if a transaction is pending that this acked object corresponds to
	 * and if yes complete it.
	 */
	private synchronized void updateAck(UAVObject obj) {
		if (DEBUG) Log.d(TAG, "Received ack: " + obj.getName());
		Assert.assertNotNull(obj);
		if (respObj != null && respObj.getObjID() == obj.getObjID()
				&& (respObj.getInstID() == obj.getInstID() || respAllInstances)) {

			// Indicate complete
			respObj = null;

			// Notify listener
			if (transactionListener != null)
				transactionListener.TransactionSucceeded(obj);
		}
	}

	/**
	 * Send an object through the telemetry link.
	 * @param[in] obj Object to send
	 * @param[in] type Transaction type
	 * @param[in] allInstances True is all instances of the object are to be sent
	 * @return Success (true), Failure (false)
	 * @throws IOException
	 */
	private boolean transmitObject(UAVObject obj, int type, boolean allInstances) throws IOException {
		// If all instances are requested on a single instance object it is an
		// error
		if (allInstances && obj.isSingleInstance()) {
			allInstances = false;
		}

		// Process message type
		if (type == TYPE_OBJ || type == TYPE_OBJ_ACK) {
			if (allInstances) {
				// Get number of instances
				int numInst = objMngr.getNumInstances(obj.getObjID());
				// Send all instances
				for (int instId = 0; instId < numInst; ++instId) {
					// TODO: This code is buggy probably.  We should send each request
					// and wait for an ack in the case of an TYPE_OBJ_ACK
					Assert.assertNotSame(type, TYPE_OBJ_ACK); // catch any buggy calls

					UAVObject inst = objMngr.getObject(obj.getObjID(), instId);
					transmitSingleObject(inst, type, false);
				}
				return true;
			} else {
				return transmitSingleObject(obj, type, false);
			}
		} else if (type == TYPE_OBJ_REQ) {
			return transmitSingleObject(obj, TYPE_OBJ_REQ, allInstances);
		} else if (type == TYPE_ACK) {
			if (!allInstances) {
				return transmitSingleObject(obj, TYPE_ACK, false);
			} else {
				return false;
			}
		} else {
			return false;
		}
	}

	/**
	 * Send an object through the telemetry link.
	 * @throws IOException
	 * @param[in] obj Object handle to send
	 * @param[in] type Transaction type \return Success (true), Failure (false)
	 */
	private boolean transmitSingleObject(UAVObject obj, int type, boolean allInstances) throws IOException {
		int length;
		int allInstId = ALL_INSTANCES;

		assert (objMngr != null && outStream != null);

		ByteBuffer bbuf = ByteBuffer.allocate(MAX_PACKET_LENGTH);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);

		// Determine data length
		if (type == TYPE_OBJ_REQ || type == TYPE_ACK) {
			length = 0;
		} else {
			length = obj.getNumBytes();
		}

		// Setup type and object id fields
		bbuf.put((byte) (SYNC_VAL & 0xff));
		bbuf.put((byte) (type & 0xff));
		bbuf.putShort((short) (length + 2 /* SYNC, Type */+ 2 /* Size */+ 4 /* ObjID */+ (obj
						.isSingleInstance() ? 0 : 2)));
		bbuf.putInt((int)obj.getObjID());

		// Setup instance ID if one is required
		if (!obj.isSingleInstance()) {
			// Check if all instances are requested
			if (allInstances)
				bbuf.putShort((short) (allInstId & 0xffff));
			else
				bbuf.putShort((short) (obj.getInstID() & 0xffff));
		}

		// Check length
		if (length >= MAX_PAYLOAD_LENGTH)
			return false;

		// Copy data (if any)
		if (length > 0)
			try {
				if (obj.pack(bbuf) == 0)
					return false;
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				return false;
			}

		// Calculate checksum
		bbuf.put((byte) (updateCRC(0, bbuf.array(), bbuf.position()) & 0xff));

		int packlen = bbuf.position();
		bbuf.position(0);
		byte[] dst = new byte[packlen];
		bbuf.get(dst, 0, packlen);

		if (type == TYPE_OBJ_ACK || type == TYPE_OBJ_REQ) {
			// Once we send a UAVTalk packet that requires an ack or object let's set up
			// the transaction here
			setupTransaction(obj, allInstances, type);
		}

		outStream.write(dst);


		// Update stats
		++stats.txObjects;
		stats.txBytes += bbuf.position();
		stats.txObjectBytes += length;

		// Done
		return true;
	}

	/**
	 * Update the crc value with new data.
	 *
	 * Generated by pycrc v0.7.5, http://www.tty1.net/pycrc/ using the
	 * configuration: Width = 8 Poly = 0x07 XorIn = 0x00 ReflectIn = False
	 * XorOut = 0x00 ReflectOut = False Algorithm = table-driven
	 *
	 * \param crc The current crc value. \param data Pointer to a buffer of \a
	 * data_len bytes. \param length Number of bytes in the \a data buffer.
	 * \return The updated crc value.
	 */
	int updateCRC(int crc, int data) {
		return crc_table[crc ^ (data & 0xff)];
	}

	int updateCRC(int crc, byte[] data, int length) {
		for (int i = 0; i < length; i++)
			crc = updateCRC(crc, data[i]);
		return crc;
	}

	private OnTransactionCompletedListener transactionListener = null;
	abstract class OnTransactionCompletedListener {
    	abstract void TransactionSucceeded(UAVObject data);
    	abstract void TransactionFailed(UAVObject data);
    };
    void setOnTransactionCompletedListener(OnTransactionCompletedListener onTransactionListener) {
    	this.transactionListener = onTransactionListener;
    }


}
