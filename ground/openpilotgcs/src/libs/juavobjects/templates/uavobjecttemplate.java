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
import  org.openpilot.uavtalk.UAVObjectMetaData;
import org.openpilot.uavtalk.UAVObjectFieldDescription;
/**
$(DESCRIPTION)

generated from $(XMLFILE)
**/
public class $(NAME) extends UAVObject{

$(FIELDSINIT)

    public void setGeneratedMetaData() {

	getMetaData().gcsAccess = UAVObjectMetaData.$(GCSACCESS);
	getMetaData().gcsTelemetryAcked = UAVObjectMetaData.$(GCSTELEM_ACKEDTF);
	getMetaData().gcsTelemetryUpdateMode = UAVObjectMetaData.$(GCSTELEM_UPDATEMODE);
	getMetaData().gcsTelemetryUpdatePeriod = $(GCSTELEM_UPDATEPERIOD);

	getMetaData().flightAccess = UAVObjectMetaData.$(FLIGHTACCESS);
	getMetaData().flightTelemetryAcked = UAVObjectMetaData.$(FLIGHTTELEM_ACKEDTF);
	getMetaData().flightTelemetryUpdateMode = UAVObjectMetaData.$(FLIGHTTELEM_UPDATEMODE);
	getMetaData().flightTelemetryUpdatePeriod = $(FLIGHTTELEM_UPDATEPERIOD);

	getMetaData().loggingUpdateMode = UAVObjectMetaData.$(LOGGING_UPDATEMODE);
	getMetaData().loggingUpdatePeriod = $(LOGGING_UPDATEPERIOD);

    }
    
    public int getObjID() {
	return $(OBJIDHEX);
    }
    
    public String getObjName() {
	return "$(NAME)";
    }

    public String getObjDescription() {
	return "$(DESCRIPTION)";
    }

$(GETTERSETTER)
}