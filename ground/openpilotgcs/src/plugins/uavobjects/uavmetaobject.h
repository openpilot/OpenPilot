/**
 ******************************************************************************
 *
 * @file       uavmetaobject.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
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
#ifndef UAVMETAOBJECT_H
#define UAVMETAOBJECT_H

#include "uavobjects_global.h"
#include "uavobject.h"

class UAVOBJECTS_EXPORT UAVMetaObject: public UAVObject
{
    Q_OBJECT

public:
    UAVMetaObject(quint32 objID, const QString& name, UAVObject* parent);
    UAVObject* getParentObject();
    void setMetadata(const Metadata& mdata);
    Metadata getMetadata();
    Metadata getDefaultMetadata();
    void setData(const Metadata& mdata);
    Metadata getData();

private:
    UAVObject* parent;
    Metadata ownMetadata;
    Metadata parentMetadata;

};

#endif // UAVMETAOBJECT_H
