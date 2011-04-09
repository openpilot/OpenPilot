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
 * @file       UAVObjectFieldDescription.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @brief      Class to deal with the description of an UAVObject Field
 *
 ****************************************************************************
*/
public class UAVObjectFieldDescription {
	private String unit;
	private String name;
	private byte type;
		
	public final static byte FIELDTYPE_INT8 = 0;
	public final static byte FIELDTYPE_INT16 = 1;
	public final static byte FIELDTYPE_INT32 =2;
	public final static byte FIELDTYPE_UINT8 =3;
	public final static byte FIELDTYPE_UINT16 =4;
	public final static byte FIELDTYPE_UINT32 =5;
	public final static byte FIELDTYPE_FLOAT32 =6;
	public final static byte FIELDTYPE_ENUM =7;

	private int objid;
	private byte fieldid;
	
	private String[] enumOptions=new String[] {};
	private String[] elementNames;

	/**
	 * @param name - the fields name
	 * @param unit - the unit in which the fields value is
	 * @param enumOptions - if field is enumeration - here are the options - otherwise null
	 * @param elementNames - if field is array - here are the names for the elements
	 */
	public UAVObjectFieldDescription(String name,int objid,byte fieldid,byte type,String unit,String[] enumOptions,String[] elementNames) {
		this.name=name;
		this.unit=unit;
		this.enumOptions=enumOptions;
		this.elementNames=elementNames;
		this.objid=objid;
		this.fieldid=fieldid;
		this.type=type;
	}
	
	public String getUnit() {
		return unit;
	}
	public String getName() {
		return name;
	}
	public String[] getEnumOptions() {
		return enumOptions;
	}
	public String[] getElementNames() {
		return elementNames;
	}

	public int getObjId() {
		return objid;
	}
	
	public byte getFieldId() {
		return fieldid;
	}

	public byte getType() {
		return type;
	}
	
}