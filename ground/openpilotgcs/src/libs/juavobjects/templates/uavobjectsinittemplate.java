/**
 ******************************************************************************
 *
 *
 * @file       uavobjectsinittemplate.java
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      the template for the uavobjects init part
 *             $(GENERATEDWARNING)
 *
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

import org.openpilot.uavtalk.uavobjects.*;
import org.openpilot.uavtalk.UAVObject;
import java.util.HashMap;

public class UAVObjects {

    private static UAVObject[] uavobjects=null;
    private static HashMap id2obj;

    public static void init() {
	if (uavobjects==null) {
	    uavobjects=new UAVObject[] {
$(OBJINIT)
	    };
	    id2obj=new HashMap();
	    for (int i=0;i< uavobjects.length;i++)
		id2obj.put(uavobjects[i].getObjID(),i);
	}
    }

    public static UAVObject[] getUAVObjectArray() {
	return uavobjects;
    }

    public static boolean hasObjectWithID(int id) {
	return id2obj.containsKey(id);
    }

    public static UAVObject getObjectByID(int id) {
	if (!hasObjectWithID(id))
	    return null;
	return uavobjects[(Integer)id2obj.get(id)];
    }

    public static UAVObject getObjectByName(String name) {
	return uavobjects[0];
    }

    public static void printAll() {
	for (UAVObject obj : uavobjects)
	    System.out.println(obj.getObjName());
    }

$(OBJGETTER)
}