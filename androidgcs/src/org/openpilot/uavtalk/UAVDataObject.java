/**
 ******************************************************************************
 * @file       UAVDataObject.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      The base object for all UAVO data.
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

public abstract class UAVDataObject extends UAVObject {

	/**
	 * @brief Constructor for UAVDataObject
	 * @param objID the object id to be created
	 * @param isSingleInst
	 * @param isSet
	 * @param name
	 */
	public UAVDataObject(long objID, Boolean isSingleInst, Boolean isSet, String name) {
		super(objID, isSingleInst, name);
		mobj = null;
		this.isSet = isSet;
	}
	/**
	 * Initialize instance ID and assign a metaobject
	 */
	public void initialize(long instID, UAVMetaObject mobj)
	{
	    //QMutexLocker locker(mutex);
	    this.mobj = mobj;
	    super.initialize(instID);
	}

	@Override
	public boolean isMetadata() { return false; };
	/**
	 * Assign a metaobject
	 */
	public void initialize(UAVMetaObject mobj)
	{
	    //QMutexLocker locker(mutex);
	    this.mobj = mobj;
	}

	/**
	 * Returns true if this is a data object holding module settings
	 */
	public boolean isSettings()
	{
	    return isSet;
	}

	/**
	 * Set the object's metadata
	 */
	@Override
	public void setMetadata(Metadata mdata)
	{
	    if ( mobj != null )
	    {
	        mobj.setData(mdata);
	    }
	}

	/**
	 * Get the object's metadata
	 */
	@Override
	public Metadata getMetadata()
	{
	    if ( mobj != null)
	    {
	        return mobj.getData();
	    }
	    else
	    {
	        return getDefaultMetadata();
	    }
	}

	/**
	 * Get the metaobject
	 */
	public UAVMetaObject getMetaObject()
	{
	    return mobj;
	}

    // TODO: Make abstract
    public UAVDataObject clone(long instID) {
		return (UAVDataObject) super.clone();
    }

    private UAVMetaObject mobj;
    private final boolean isSet;

}
