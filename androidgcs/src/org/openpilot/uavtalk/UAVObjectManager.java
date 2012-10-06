/**
 ******************************************************************************
 * @file       UAVObjectManager.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Critical class.  This is the data store for all UAVOs.  Allows
 *             other objects to access and change this data.  Takes care of
 *             propagating changes to the UAV.
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

import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;
import java.util.Observable;
import java.util.Observer;

public class UAVObjectManager {

	public class CallbackListener extends Observable {
		public void event (UAVObject obj) {
			setChanged();
			notifyObservers(obj);
		}
	}
	private final CallbackListener newInstance = new CallbackListener();
	public void addNewInstanceObserver(Observer o) {
		synchronized(newInstance) {
			newInstance.addObserver(o);
		}
	}
	private final CallbackListener newObject = new CallbackListener();
	public void addNewObjectObserver(Observer o) {
		synchronized(newObject) {
			newObject.addObserver(o);
		}
	}
	private final int MAX_INSTANCES = 10;

	// Use array list to store objects since rarely added or deleted
	private final List<List<UAVObject>> objects = new ArrayList<List<UAVObject>>();

	public UAVObjectManager()
	{
		//mutex = new QMutex(QMutex::Recursive);
	}

	/**
	 * Register an object with the manager. This function must be called for all newly created instances.
	 * A new instance can be created directly by instantiating a new object or by calling clone() of
	 * an existing object. The object will be registered and will be properly initialized so that it can accept
	 * updates.
	 * @throws Exception
	 */
	public synchronized boolean registerObject(UAVDataObject obj) throws Exception
	{

		ListIterator<List<UAVObject>> objIt = objects.listIterator(0);

		// Check if this object type is already in the list
		while(objIt.hasNext()) {
			List<UAVObject> instList = objIt.next();

			// Check if the object ID is in the list
			if( (instList.size() > 0) && (instList.get(0).getObjID() == obj.getObjID() )) {
				// Check if this is a single instance object, if yes we can not add a new instance
				if(obj.isSingleInstance()) {
					return false;
				}
				// The object type has alredy been added, so now we need to initialize the new instance with the appropriate id
				// There is a single metaobject for all object instances of this type, so no need to create a new one
				// Get object type metaobject from existing instance
				UAVDataObject refObj = (UAVDataObject) instList.get(0);
				if (refObj == null)
				{
					return false;
				}
				UAVMetaObject mobj = refObj.getMetaObject();

				// Make sure we aren't requesting to create too many instances
				if(obj.getInstID() >= MAX_INSTANCES || instList.size() >= MAX_INSTANCES || obj.getInstID() < 0) {
					return false;
				}

				// If InstID is zero then we find the next open instId and create it
				if (obj.getInstID() == 0)
				{
					// Assign the next available ID and initialize the object instance the nadd
					obj.initialize(instList.size(), mobj);
					instList.add(obj);
					return true;
				}

				// Check if that inst ID already exists
				ListIterator<UAVObject> instIter = instList.listIterator();
				while(instIter.hasNext()) {
					UAVObject testObj = instIter.next();
					if(testObj.getInstID() == obj.getInstID()) {
						return false;
					}
				}

				// If the instance ID is specified and not at the default value (0) then we need to make sure
				// that there are no gaps in the instance list. If gaps are found then then additional instances
				// will be created.
				for(long instId = instList.size(); instId < obj.getInstID(); instId++) {
					UAVDataObject newObj = obj.clone(instId);
					newObj.initialize(mobj);
					instList.add(newObj);
					newInstance.event(newObj);
				}
				obj.initialize(mobj);
				instList.add(obj);
				newInstance.event(obj);

				instIter = instList.listIterator();
				while(instIter.hasNext()) {
					UAVObject testObj = instIter.next();
					if(testObj.getInstID() == obj.getInstID()) {
						return false;
					}
				}


				// Check if there are any gaps between the requested instance ID and the ones in the list,
				// if any then create the missing instances.
				for (long instId = instList.size(); instId < obj.getInstID(); ++instId)
				{
					UAVDataObject cobj = obj.clone(instId);
					cobj.initialize(mobj);
					instList.add(cobj);
					newInstance.event(cobj);
				}
				// Finally, initialize the actual object instance
				obj.initialize(mobj);
				// Add the actual object instance in the list
				instList.add(obj);
				newInstance.event(obj);
				return true;
			}

		}

		// If this point is reached then this is the first time this object type (ID) is added in the list
		// create a new list of the instances, add in the object collection and create the object's metaobject
		// Create metaobject
		String mname = obj.getName();
		mname += "Meta";

		UAVMetaObject mobj = new UAVMetaObject(obj.getObjID()+1, mname, obj);
		// Initialize object
		obj.initialize(0, mobj);
		// Add to list
		addObject(obj);
		addObject(mobj);
		return true;
	}

	public synchronized void addObject(UAVObject obj)
	{
		// Add to list
		List<UAVObject> ls = new ArrayList<UAVObject>();
		ls.add(obj);
		objects.add(ls);
		newObject.event(obj);
	}

	/**
	 * Get all objects. A two dimentional QList is returned. Objects are grouped by
	 * instances of the same object type.
	 */
	public List<List<UAVObject>> getObjects()
	{
		return objects;
	}

	/**
	 * Same as getObjects() but will only return DataObjects.
	 */
	public synchronized List< List<UAVDataObject> > getDataObjects()
	{
		List< List<UAVDataObject> > dObjects = new ArrayList< List<UAVDataObject> > ();

		// Go through objects and copy to new list when types match
		ListIterator<List<UAVObject>> objIt = objects.listIterator(0);

		// Check if this object type is already in the list
		while(objIt.hasNext()) {
			List<UAVObject> instList = objIt.next();

			// If no instances skip
			if(instList.size() == 0)
				continue;

			// If meta data skip
			if(instList.get(0).isMetadata())
				continue;

			List<UAVDataObject> newInstList = new ArrayList<UAVDataObject>();
			ListIterator<UAVObject> instIt = instList.listIterator();
			while(instIt.hasNext()) {
				newInstList.add((UAVDataObject) instIt.next());
			}
			dObjects.add(newInstList);
		}
		// Done
		return dObjects;
	}

	/**
	 * Same as getObjects() but will only return MetaObjects.
	 */
	public synchronized List <List<UAVMetaObject> > getMetaObjects()
	{
		List< List<UAVMetaObject> > mObjects = new ArrayList< List<UAVMetaObject> > ();

		// Go through objects and copy to new list when types match
		ListIterator<List<UAVObject>> objIt = objects.listIterator(0);

		// Check if this object type is already in the list
		while(objIt.hasNext()) {
			List<UAVObject> instList = objIt.next();

			// If no instances skip
			if(instList.size() == 0)
				continue;

			// If meta data skip
			if(!instList.get(0).isMetadata())
				continue;

			List<UAVMetaObject> newInstList = new ArrayList<UAVMetaObject>();
			ListIterator<UAVObject> instIt = instList.listIterator();
			while(instIt.hasNext()) {
				newInstList.add((UAVMetaObject) instIt.next());
			}
			mObjects.add(newInstList);
		}
		// Done
		return mObjects;
	}


	/**
	 * Returns a specific object by name only, returns instance ID zero
	 * @param name The object name
	 * @return The object or null if not found
	 */
	public UAVObject getObject(String name)
	{
		return getObject(name, 0, 0);
	}

	/**
	 * Get a specific object given its name and instance ID
	 * @returns The object is found or NULL if not
	 */
	public UAVObject getObject(String name, long instId)
	{
		return getObject(name, 0, instId);
	}

	/**
	 * Get a specific object with given object ID and instance ID zero
	 * @param objId the object id
	 * @returns The object is found or NULL if not
	 */
	public UAVObject getObject(long objId)
	{
		return getObject(null, objId, 0);
	}

	/**
	 * Get a specific object given its object and instance ID
	 * @returns The object is found or NULL if not
	 */
	public UAVObject getObject(long objId, long instId)
	{
		return getObject(null, objId, instId);
	}

	/**
	 * Helper function for the public getObject() functions.
	 */
	public synchronized UAVObject getObject(String name, long objId, long instId)
	{
		// Check if this object type is already in the list
		ListIterator<List<UAVObject>> objIter = objects.listIterator();
		while(objIter.hasNext()) {
			List<UAVObject> instList = objIter.next();
			if (instList.size() > 0) {
				if ( (name != null && instList.get(0).getName().compareTo(name) == 0) || (name == null && instList.get(0).getObjID() == objId) ) {
					// Look for the requested instance ID
					ListIterator<UAVObject> iter = instList.listIterator();
					while(iter.hasNext()) {
						UAVObject obj = iter.next();
						if(obj.getInstID() == instId) {
							return obj;
						}
					}
				}
			}
		}

		return null;
	}

	/**
	 * Get all the instances of the object specified by name
	 */
	public List<UAVObject> getObjectInstances(String name)
	{
		return getObjectInstances(name, 0);
	}

	/**
	 * Get all the instances of the object specified by its ID
	 */
	public List<UAVObject> getObjectInstances(long objId)
	{
		return getObjectInstances(null, objId);
	}

	/**
	 * Helper function for the public getObjectInstances()
	 */
	public synchronized List<UAVObject> getObjectInstances(String name, long objId)
	{
		// Check if this object type is already in the list
		ListIterator<List<UAVObject>> objIter = objects.listIterator();
		while(objIter.hasNext()) {
			List<UAVObject> instList = objIter.next();
			if (instList.size() > 0) {
				if ( (name != null && instList.get(0).getName().compareTo(name) == 0) || (name == null && instList.get(0).getObjID() == objId) ) {
					return instList;
				}
			}
		}

		return null;
	}

	/**
	 * Get the number of instances for an object given its name
	 */
	public int getNumInstances(String name)
	{
		return getNumInstances(name, 0);
	}

	/**
	 * Get the number of instances for an object given its ID
	 */
	public int getNumInstances(long objId)
	{
		return getNumInstances(null, objId);
	}

	/**
	 * Helper function for public getNumInstances
	 */
	public int getNumInstances(String name, long objId)
	{
		return getObjectInstances(name,objId).size();
	}

}
