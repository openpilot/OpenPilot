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

/**
 ******************************************************************************
 *
 * @file       UAVObjectMetaData.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Class to handle the metadata of an UAVObject
 *
 ****************************************************************************
*/
public class UAVObjectMetaData {

	public final static byte ACCESS_READWRITE=0;
	public final static byte ACCESS_READONLY=1;
	public final static byte ACCESS_WRITEONLY=2;

	public final static String getAccessString(byte access_mode) {
		switch (access_mode) {
		case ACCESS_READWRITE:
			return "Read/Write";
		case ACCESS_WRITEONLY:
			return "Write only";
		case ACCESS_READONLY:
			return "Read only";
		default:
			return "unknowm Access Mode";
		}
	}
	
	public final static byte UPDATEMODE_NEVER=0;
	public final static byte UPDATEMODE_MANUAL=1;
	public final static byte UPDATEMODE_ONCHANGE=2;
	public final static byte UPDATEMODE_PERIODIC=3;
	
	public final static String getUpdateModeString(byte update_mode) {
		switch(update_mode) {
		case UPDATEMODE_NEVER:
			return "never";
		case UPDATEMODE_MANUAL:
			return "manual";
		case UPDATEMODE_ONCHANGE:
			return "on change";
		case UPDATEMODE_PERIODIC:
			return "periodic";
		default:
			return "unknown";
			
		}
	}
	
	public final static boolean TRUE=true;
	public final static boolean FALSE=false;
	
	/* constant */
	public byte gcsAccess;
    public boolean gcsTelemetryAcked;
    public byte gcsTelemetryUpdateMode;
    public int gcsTelemetryUpdatePeriod;
      
    public byte flightAccess; 
    public boolean  flightTelemetryAcked;
    public byte  flightTelemetryUpdateMode;
    public int flightTelemetryUpdatePeriod;
      
    public byte loggingUpdateMode;
    public int loggingUpdatePeriod;

    /* changing */
    public long last_serialize;
    public long last_deserialize;
    
    public long last_log;
    public long last_gcs_update;
    public long last_fligt_update;

    public long last_send_time;
    
    public boolean gcs_flight_was_acked=false;
    public boolean gcs_ground_was_acked=false;
    
    public boolean req_pending=false;
    public boolean ack_pending=false;
}
