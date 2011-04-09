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
 * @file       UAVTalkDefinitions.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Provide values which are used in the UAVTalk Protocol
 *
 ****************************************************************************
*/
public class UAVTalkDefinitions {
	
	public final static byte SYNC_VAL     = 0x3C;
	
	public final static byte TYPE_MASK_VER = (byte)0xFC;
	public final static byte TYPE_MASK_TYPE = (byte)0x03;
	
	public final static byte TYPE_VER     = 0x20;
	public final static byte TYPE_OBJ     = (TYPE_VER | 0x00);
	public final static byte TYPE_OBJ_REQ = (TYPE_VER | 0x01);
	public final static byte TYPE_OBJ_ACK = (TYPE_VER | 0x02);
	public final static byte TYPE_ACK     = (TYPE_VER | 0x03);


	public final static String getTypeString(byte type) {
		switch(type) {
			case TYPE_ACK:
				return "ack";
			case TYPE_OBJ:
				return "obj";
			case TYPE_OBJ_ACK:
				return "obj_ack";
			case TYPE_OBJ_REQ:
				return "obj_req";
		}			
		return "unknown type";
	}


}
