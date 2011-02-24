package org.openpilot.uavtalk;

import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;

public class UAVObjectManager {

	private final int MAX_INSTANCES = 10;

	// Use array list to store objects since rarely added or deleted
	private List<List<UAVObject>> objects = new ArrayList<List<UAVObject>>();  

	public UAVObjectManager()
	{
		//mutex = new QMutex(QMutex::Recursive);
	}

	/**
	 * Register an object with the manager. This function must be called for all newly created instances.
	 * A new instance can be created directly by instantiating a new object or by calling clone() of
	 * an existing object. The object will be registered and will be properly initialized so that it can accept
	 * updates.
	 */
	Boolean registerObject(UAVDataObject obj)
	{
		//  QMutexLocker locker(mutex);

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
				for(int instID = instList.size(); instID < obj.getInstID(); instID++) {
					UAVDataObject newObj = obj.clone(instID);
					newObj.initialize(mobj);
					instList.add(newObj);
					// emit new instance signal
				}
				obj.initialize(mobj);
				//emit new instance signal
				instList.add(obj);

				instIter = instList.listIterator();
				while(instIter.hasNext()) {
					UAVObject testObj = instIter.next();
					if(testObj.getInstID() == obj.getInstID()) {
						return false;
					}
				}


				// Check if there are any gaps between the requested instance ID and the ones in the list,
				// if any then create the missing instances.
				for (int instId = instList.size(); instId < obj.getInstID(); ++instId)
				{
					UAVDataObject cobj = obj.clone(instId);
					cobj.initialize(mobj);
					instList.add(cobj);
					// emit newInstance(cobj);
				}
				// Finally, initialize the actual object instance
				obj.initialize(mobj);
				// Add the actual object instance in the list
				instList.add(obj);
				//emit newInstance(obj);
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

	void addObject(UAVObject obj)
	{
		// Add to list
		List<UAVObject> ls = new ArrayList<UAVObject>();
		ls.add(obj);
		objects.add(ls);
		//emit newObject(obj);
	}

	/**
	 * Get all objects. A two dimentional QList is returned. Objects are grouped by
	 * instances of the same object type.
	 */
	List<List<UAVObject>> getObjects()
	{
		//QMutexLocker locker(mutex);
		return objects;
	}

	/**
	 * Same as getObjects() but will only return DataObjects.
	 */
	List< List<UAVDataObject> > getDataObjects()
	{
		return new ArrayList<List<UAVDataObject>>();

		/*		QMutexLocker locker(mutex);
		QList< QList<UAVDataObject*> > dObjects;

		// Go through objects and copy to new list when types match
		for (int objidx = 0; objidx < objects.length(); ++objidx)
		{
			if (objects[objidx].length() > 0)
			{
				// Check type
				UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objects[objidx][0]);
				if (obj != NULL)
				{
					// Create instance list
					QList<UAVDataObject*> list;
					// Go through instances and cast them to UAVDataObject, then add to list
					for (int instidx = 0; instidx < objects[objidx].length(); ++instidx)
					{
						obj = dynamic_cast<UAVDataObject*>(objects[objidx][instidx]);
						if (obj != NULL)
						{
							list.append(obj);
						}
					}
					// Append to object list
					dObjects.append(list);
				}
			}
		}*/
		// Done
	}

	/**
	 * Same as getObjects() but will only return MetaObjects.
	 */
	List <List<UAVMetaObject> > getMetaObjects()
	{
		return new ArrayList< List<UAVMetaObject> >();
		/*
		QMutexLocker locker(mutex);
		QList< QList<UAVMetaObject*> > mObjects;

		// Go through objects and copy to new list when types match
		for (int objidx = 0; objidx < objects.length(); ++objidx)
		{
			if (objects[objidx].length() > 0)
			{
				// Check type
				UAVMetaObject* obj = dynamic_cast<UAVMetaObject*>(objects[objidx][0]);
				if (obj != NULL)
				{
					// Create instance list
					QList<UAVMetaObject*> list;
					// Go through instances and cast them to UAVMetaObject, then add to list
					for (int instidx = 0; instidx < objects[objidx].length(); ++instidx)
					{
						obj = dynamic_cast<UAVMetaObject*>(objects[objidx][instidx]);
						if (obj != NULL)
						{
							list.append(obj);
						}
					}
					// Append to object list
					mObjects.append(list);
				}
			}
		}
		// Done
		return mObjects;
		 */
	}

	/**
	 * Get a specific object given its name and instance ID
	 * @returns The object is found or NULL if not
	 */
	UAVObject getObject(String name, int instId)
	{
		return getObject(name, 0, instId);
	}

	/**
	 * Get a specific object given its object and instance ID
	 * @returns The object is found or NULL if not
	 */
	UAVObject getObject(int objId, int instId)
	{
		return getObject(null, objId, instId);
	}

	/**
	 * Helper function for the public getObject() functions.
	 */
	UAVObject getObject(String name, int objId, int instId)
	{
		//QMutexLocker locker(mutex);
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
	List<UAVObject> getObjectInstances(String name)
	{
		return getObjectInstances(name, 0);
	}

	/**
	 * Get all the instances of the object specified by its ID
	 */
	List<UAVObject> getObjectInstances(int objId)
	{
		return getObjectInstances(null, objId);
	}

	/**
	 * Helper function for the public getObjectInstances()
	 */
	List<UAVObject> getObjectInstances(String name, int objId)
	{
		//QMutexLocker locker(mutex);
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
	int getNumInstances(String name)
	{
		return getNumInstances(name, 0);
	}

	/**
	 * Get the number of instances for an object given its ID
	 */
	int getNumInstances(int objId)
	{
		return getNumInstances(null, objId);
	}

	/**
	 * Helper function for public getNumInstances
	 */
	int getNumInstances(String name, int objId)
	{
		return getObjectInstances(name,objId).size();
	}


}
