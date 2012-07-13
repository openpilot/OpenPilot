/**
 ******************************************************************************
 *
 * @file       $(NAMELC).h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 *   
 * @note       Object definition file: $(XMLFILE). 
 *             This is an automatically generated file.
 *             DO NOT modify manually.
 *
 * @brief      The UAVUObjects GCS plugin 
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
#ifndef $(NAMEUC)_H
#define $(NAMEUC)_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT $(NAME): public UAVDataObject
{
    Q_OBJECT
$(PROPERTIES)

public:
    // Field structure
    typedef struct {
$(DATAFIELDS)
    } __attribute__((packed)) DataFields;

    // Field information
$(DATAFIELDINFO)
  
    // Constants
    static const quint32 OBJID = $(OBJIDHEX);
    static const QString NAME;
    static const QString DESCRIPTION;
    static const QString CATEGORY;
    static const bool ISSINGLEINST = $(ISSINGLEINST);
    static const bool ISSETTINGS = $(ISSETTINGS);
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    $(NAME)();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);
	UAVDataObject* dirtyClone();
	
    static $(NAME)* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);

$(PROPERTY_GETTERS)

public slots:
$(PROPERTY_SETTERS)

signals:
$(PROPERTY_NOTIFICATIONS)

private slots:
    void emitNotifications();
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // $(NAMEUC)_H
