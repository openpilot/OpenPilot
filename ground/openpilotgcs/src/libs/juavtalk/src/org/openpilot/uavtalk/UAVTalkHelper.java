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
import org.openpilot.uavtalk.CRC8;
import org.openpilot.uavtalk.UAVObject;

/**
 ******************************************************************************
 *
 * @file       UAVTalkHelper.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      misc static helper functions to deal with UAVTalk
 *
 ****************************************************************************
*/
public class UAVTalkHelper {
	
	public final static int PACKAGE_LENGTH_SYNC=1;
	public final static int PACKAGE_LENGTH_CRC=1;
	public final static int PACKAGE_LENGTH_TYPE=1;
	public final static int PACKAGE_LENGTH_OBJID=4;
	public final static int PACKAGE_LENGTH_SIZE=2;
	
	public final static int MIN_PACKAGE_SIZE=PACKAGE_LENGTH_SYNC 
										    + PACKAGE_LENGTH_TYPE 
										    + PACKAGE_LENGTH_SIZE 
										    + PACKAGE_LENGTH_OBJID
										    + PACKAGE_LENGTH_CRC ;

	/**
	 * generate a UAVTalk package 
	 * 
	 * @param type - the package 
	 * @param obj_id - the object ID
	 * @param payload - the payload byte-array
	 * @return byte-array containing the UAVTalk Package
	 */
	public static byte[] generateUAVTalkPackage(byte type,int obj_id,byte[] payload) {
		byte[] res=new byte[MIN_PACKAGE_SIZE +payload.length];
		res[0]=UAVTalkDefinitions.SYNC_VAL;
		res[1]=type;
		res[2]=(byte)((MIN_PACKAGE_SIZE-1 + payload.length)&0xFF);
		res[3]=(byte)(((MIN_PACKAGE_SIZE-1  + payload.length)>>8)&0xFF);
		res[4]=(byte)((obj_id)&0xFF);
		res[5]=(byte)((obj_id>>8)&0xFF);
		res[6]=(byte)((obj_id>>16)&0xFF);
		res[7]=(byte)((obj_id>>24)&0xFF);
		
		
		for (int i=0;i<payload.length;i++) 
			res[8+i]=payload[i];
		
		
		res[8+payload.length]=CRC8.arrayUpdate((byte)0,res,8+payload.length);
	
		
		return res;
	}
	
	/**
	 * generate a UAVTalk package without any payload
	 * 
	 * @param type - the package 
	 * @param obj_id - the object ID
	 * @return byte-array containing the UAVTalk Package
	 */
	public static byte[] generateUAVTalkPackage(byte type,int obj_id) {
		return  generateUAVTalkPackage(type,obj_id,new byte[0]); 
	}

	/**
	 * generate a UAVTalk package and serialize the object as payload if needed
	 * 
	 * @param type - the package 
	 * @param obj - the object
	 * @return byte-array containing the UAVTalk Package
	 */
	public static byte[] generateUAVTalkPackage(byte type,UAVObject obj) {
		switch (type) {
			case UAVTalkDefinitions.TYPE_OBJ_ACK:
			case UAVTalkDefinitions.TYPE_OBJ:
				return generateUAVTalkPackage(type,obj.getObjID(),obj.serialize()); 
			default:	
				return generateUAVTalkPackage(type,obj.getObjID()); 
		}
		
	}
}	