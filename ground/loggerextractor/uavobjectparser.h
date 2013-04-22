/**
 ******************************************************************************
 *
 * @file       uavobjectparser.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Parses XML files and extracts object information.
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

#ifndef UAVOBJECTPARSER_H
#define UAVOBJECTPARSER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QByteArray>

// Types
typedef enum {
    FIELDTYPE_INT8 = 0,
    FIELDTYPE_INT16,
    FIELDTYPE_INT32,
    FIELDTYPE_UINT8,
    FIELDTYPE_UINT16,
    FIELDTYPE_UINT32,
    FIELDTYPE_FLOAT32,
    FIELDTYPE_ENUM
} FieldType;

typedef struct {
    QString name;
    QString units;
    FieldType type;
    int numElements;
    int numBytes;
    QStringList elementNames;
    QStringList options; // for enums only
    bool defaultElementNames;
    QStringList defaultValues;
    QString limitValues;
} FieldInfo;

/**
 * Object update mode
 */
typedef enum {
    UPDATEMODE_MANUAL =    0, /** Manually update object, by calling the updated() function */
    UPDATEMODE_PERIODIC =  1, /** Automatically update object at periodic intervals */
    UPDATEMODE_ONCHANGE  = 2, /** Only update object when its data changes */
    UPDATEMODE_THROTTLED = 3  /** Object is updated on change, but not more often than the interval time */
} UpdateMode;


typedef enum {
    ACCESS_READWRITE = 0,
    ACCESS_READONLY = 1
} AccessMode;


typedef struct  {
    QString name;
    QString namelc; /** name in lowercase */
    QString filename;
    quint32 id;
    bool isSingleInst;
    bool isSettings;
    AccessMode gcsAccess;
    AccessMode flightAccess;
    bool flightTelemetryAcked;
    UpdateMode flightTelemetryUpdateMode; /** Update mode used by the autopilot (UpdateMode) */
    int flightTelemetryUpdatePeriod; /** Update period used by the autopilot (only if telemetry mode is PERIODIC) */
    bool gcsTelemetryAcked;
    UpdateMode gcsTelemetryUpdateMode; /** Update mode used by the GCS (UpdateMode) */
    int gcsTelemetryUpdatePeriod; /** Update period used by the GCS (only if telemetry mode is PERIODIC) */
    UpdateMode loggingUpdateMode; /** Update mode used by the logging module (UpdateMode) */
    int loggingUpdatePeriod; /** Update period used by the logging module (only if logging mode is PERIODIC) */
    QList<FieldInfo*> fields; /** The data fields for the object **/
    QString description; /** Description used for Doxygen **/
    QString category; /** Description used for Doxygen **/
} ObjectInfo;

class UAVObjectParser
{
public:

    // Functions
    UAVObjectParser();
    QString parseXML(QString& xml, QString& filename);
    int getNumObjects();
    QList<ObjectInfo*> getObjectInfo();
    QString getObjectName(int objIndex);
    quint32 getObjectID(int objIndex);

    ObjectInfo* getObjectByIndex(int objIndex);
    int getNumBytes(int objIndex);
    QStringList all_units;

private:
    QList<ObjectInfo*> objInfo;
    QStringList fieldTypeStrXML;
    QList<int> fieldTypeNumBytes;
    QStringList updateModeStr;
    QStringList updateModeStrXML;
    QStringList accessModeStr;
    QStringList accessModeStrXML;

    QString processObjectAttributes(QDomNode& node, ObjectInfo* info);
    QString processObjectFields(QDomNode& childNode, ObjectInfo* info);
    QString processObjectAccess(QDomNode& childNode, ObjectInfo* info);
    QString processObjectDescription(QDomNode& childNode, QString * description);
    QString processObjectCategory(QDomNode& childNode, QString * category);
    QString processObjectMetadata(QDomNode& childNode, UpdateMode* mode, int* period, bool* acked);
    void calculateID(ObjectInfo* info);
    quint32 updateHash(quint32 value, quint32 hash);
    quint32 updateHash(QString& value, quint32 hash);
};

#endif // UAVOBJECTPARSER_H
