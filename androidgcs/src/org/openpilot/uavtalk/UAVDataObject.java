package org.openpilot.uavtalk;

public abstract class UAVDataObject extends UAVObject {
	
	/**
	 * @brief Constructor for UAVDataObject
	 * @param objID the object id to be created
	 * @param isSingleInst 
	 * @param isSet
	 * @param name
	 */
	public UAVDataObject(int objID, Boolean isSingleInst, Boolean isSet, String name) {
		super(objID, isSingleInst, name);
		mobj = null;
		this.isSet = isSet;
	}
	/**
	 * Initialize instance ID and assign a metaobject
	 */
	public void initialize(int instID, UAVMetaObject mobj)
	{
	    //QMutexLocker locker(mutex);
	    this.mobj = mobj;
	    super.initialize(instID);
	}

	public boolean isMetadata() { return true; };
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
    public UAVDataObject clone(int instID) {
		return (UAVDataObject) super.clone();
    }
    
    private UAVMetaObject mobj;
    private boolean isSet;

}
