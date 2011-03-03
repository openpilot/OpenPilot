/**
 ******************************************************************************
 *
 * @file       uavobjecttemplate.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Template for an uavobject in java
 *             $(GENERATEDWARNING)
 *             $(DESCRIPTION)
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

package org.openpilot.uavtalk.uavobjects;

import  org.openpilot.uavtalk.UAVObject;
import  org.openpilot.uavtalk.UAVDataObject;
import  org.openpilot.uavtalk.UAVMetaObject;
import  org.openpilot.uavtalk.UAVObjectField;

/**
$(DESCRIPTION)

generated from $(XMLFILE)
**/
public class $(NAME) extends UAVDataObject {

$(FIELDSINIT)

    public $(NAME) (int objID, Boolean isSingleInst, Boolean isSet, String name) {
        super(OBJID, ISSINGLEINST, ISSETTINGS, NAME);
    }

    public void setGeneratedMetaData() {

	getMetaData().gcsAccess = UAVObject.AccessMode.$(GCSACCESS);
	getMetaData().gcsTelemetryAcked = UAVObject.Acked.$(GCSTELEM_ACKEDTF);
	getMetaData().gcsTelemetryUpdateMode = UAVObject.UpdateMode.$(GCSTELEM_UPDATEMODE);
	getMetaData().gcsTelemetryUpdatePeriod = $(GCSTELEM_UPDATEPERIOD);

	getMetaData().flightAccess = UAVObject.AccessMode.$(FLIGHTACCESS);
	getMetaData().flightTelemetryAcked = UAVObject.Acked.$(FLIGHTTELEM_ACKEDTF);
	getMetaData().flightTelemetryUpdateMode = UAVObject.UpdateMode.$(FLIGHTTELEM_UPDATEMODE);
	getMetaData().flightTelemetryUpdatePeriod = $(FLIGHTTELEM_UPDATEPERIOD);

	getMetaData().loggingUpdateMode = UAVObject.UpdateMode.$(LOGGING_UPDATEMODE);
	getMetaData().loggingUpdatePeriod = $(LOGGING_UPDATEPERIOD);

    }
    
    public int getObjID() {
	return $(OBJIDHEX);
    }
    
    public String getObjName() {
	return "$(NAME)";
    }

    public String getDescription() {
	return "$(DESCRIPTION)";
    }
protected:
	// Constants
	static final int OBJID = $(OBJIDHEX);
	static final String NAME;
	static final String DESCRIPTION;
	static final boolean ISSINGLEINST = $(ISSINGLEINST);
	static final boolean ISSETTINGS = $(ISSETTINGS);
	static final int NUMBYTES = sizeof(DataFields);


$(GETTERSETTER)
}