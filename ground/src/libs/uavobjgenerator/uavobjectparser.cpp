/**
 ******************************************************************************
 *
 * @file       uavobjectparser.cpp
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

#include "uavobjectparser.h"
#include <QByteArray>

const char* UAVObjectParser::FieldTypeStr[] = {"FIELDTYPE_INT8", "FIELDTYPE_INT16", "FIELDTYPE_INT32", "FIELDTYPE_FLOAT32", "FIELDTYPE_CHAR"};
const char* UAVObjectParser::FieldTypeStrC[] = {"int8_t", "int16_t", "int32_t", "float", "char"};
const char* UAVObjectParser::FieldTypeStrCPP[] = {"qint8", "qint16", "qint32", "float", "char"};
const char* UAVObjectParser::UpdateModeStr[] = {"UPDATEMODE_PERIODIC", "UPDATEMODE_ONCHANGE", "UPDATEMODE_MANUAL", "UPDATEMODE_NEVER"};

/**
 * Constructor
 */
UAVObjectParser::UAVObjectParser()
{

}

/**
 * Get number of objects
 */
int UAVObjectParser::getNumObjects()
{
    return objInfo.length();
}

/**
 * Get the detailed object information
 */
QList<UAVObjectParser::ObjectInfo*> UAVObjectParser::getObjectInfo()
{
    return objInfo;
}

/**
 * Get the name of the object
 */
QString UAVObjectParser::getObjectName(int objIndex)
{
    ObjectInfo* info = objInfo[objIndex];
    if (info == NULL)
    {
        return QString();
    }
    else
    {
        return info->name;
    }
}

/**
 * Get the ID of the object
 */
quint32 UAVObjectParser::getObjectID(int objIndex)
{
    ObjectInfo* info = objInfo[objIndex];
    if (info == NULL)
    {
        return 0;
    }
    else
    {
        return info->id;
    }
}

/**
 * Parse supplied XML file
 * @param xml The xml text
 * @param filename The xml filename
 * @returns Null QString() on success, error message on failure
 */
QString UAVObjectParser::parseXML(QString& xml, QString& filename)
{
    this->filename = filename;
    // Create DOM document and parse it
    QDomDocument doc("UAVObjects");
    bool parsed = doc.setContent(xml);
    if (!parsed) return QString("Improperly formated XML file");

    // Read all objects contained in the XML file, creating an new ObjectInfo for each
    QDomElement docElement = doc.documentElement();
    QDomNode node = docElement.firstChild();
    while ( !node.isNull() )
    {
        // Create new object entry
        ObjectInfo* info = new ObjectInfo;
        // Process object attributes
        QString status = processObjectAttributes(node, info);
        if (!status.isNull())
        {
            return status;
        }

        // Process child elements (fields and metadata)
        QDomNode childNode = node.firstChild();
        while ( !childNode.isNull() )
        {
            // Process element depending on its type
            if ( childNode.nodeName().compare(QString("field")) == 0 )
            {
                QString status = processObjectFields(childNode, info);
                if (!status.isNull())
                {
                    return status;
                }
            }
            else if ( childNode.nodeName().compare(QString("telemetrygcs")) == 0 )
            {
                QString status = processObjectMetadata(childNode, &info->gcsTelemetryUpdateMode,
                                                       &info->gcsTelemetryUpdatePeriod, &info->gcsTelemetryAcked);
                if (!status.isNull())
                {
                    return status;
                }
            }
            else if ( childNode.nodeName().compare(QString("telemetryflight")) == 0 )
            {
                QString status = processObjectMetadata(childNode, &info->flightTelemetryUpdateMode,
                                                       &info->flightTelemetryUpdatePeriod, &info->flightTelemetryAcked);
                if (!status.isNull())
                {
                    return status;
                }
            }
            else if ( childNode.nodeName().compare(QString("logging")) == 0 )
            {
                QString status = processObjectMetadata(childNode, &info->loggingUpdateMode,
                                                       &info->loggingUpdatePeriod, NULL);
                if (!status.isNull())
                {
                    return status;
                }
            }
            else
            {
                return QString("Unknown object element");
            }
            // Get next element
            childNode = childNode.nextSibling();
        }

        // Calculate ID
        calculateID(info);

        // Add object
        objInfo.append(info);

        // Get next object
        node = node.nextSibling();
    }

    // Done, return null string
    return QString();
}

/**
 * Calculate the unique object ID based on the object information.
 * The ID will change if the object definition changes, this is intentional
 * and is used to avoid connecting objects with incompatible configurations.
 */
void UAVObjectParser::calculateID(ObjectInfo* info)
{
    // Hash object name
    quint32 hash = updateHash(info->name, 0);
    // Hash object attributes
    hash = updateHash(info->isSettings, hash);
    hash = updateHash(info->isSingleInst, hash);
    // Hash field information
    for (int n = 0; n < info->fields.length(); ++n)
    {
        hash = updateHash(info->fields[n]->name, hash);
        hash = updateHash(info->fields[n]->numElements, hash);
        hash = updateHash(info->fields[n]->type, hash);
    }
    // Done
    info->id = hash;
}

/**
 * Shift-Add-XOR hash implementation. LSB is set to zero, it is reserved
 * for the ID of the metaobject.
 *
 * http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
 */
quint32 UAVObjectParser::updateHash(quint32 value, quint32 hash)
{
    return (hash ^ ((hash<<5) + (hash>>2) + value)) & 0xFFFFFFFE;
}

/**
 * Update the hash given a string
 */
quint32 UAVObjectParser::updateHash(QString& value, quint32 hash)
{
    QByteArray bytes = value.toAscii();
    quint32 hashout = hash;
    for (int n = 0; n < bytes.length(); ++n)
    {
        hashout = updateHash(bytes[n], hashout);
    }
    return hashout;
}

/**
 * Process the metadata part of the XML
 */
QString UAVObjectParser::processObjectMetadata(QDomNode& childNode, UpdateMode* mode, int* period, bool* acked)
{
    // Get updatemode attribute
    QDomNamedNodeMap elemAttributes = childNode.attributes();
    QDomNode elemAttr = elemAttributes.namedItem("updatemode");
    if ( elemAttr.isNull() )
    {
        return QString("Object:telemetrygcs:updatemode attribute is missing");
    }
    else
    {
        if ( elemAttr.nodeValue().compare(QString("periodic")) == 0 )
        {
            *mode = UPDATEMODE_PERIODIC;
        }
        else if ( elemAttr.nodeValue().compare(QString("onchange")) == 0 )
        {
            *mode = UPDATEMODE_ONCHANGE;
        }
        else if ( elemAttr.nodeValue().compare(QString("manual")) == 0 )
        {
            *mode = UPDATEMODE_MANUAL;
        }
        else if ( elemAttr.nodeValue().compare(QString("never")) == 0 )
        {
            *mode = UPDATEMODE_NEVER;
        }
        else
        {
            return QString("Object:telemetrygcs:updatemode attribute value is invalid");
        }
    }
    // Get period attribute
    elemAttr = elemAttributes.namedItem("period");
    if ( elemAttr.isNull() )
    {
        return QString("Object:telemetrygcs:period attribute is missing");
    }
    else
    {
        *period = elemAttr.nodeValue().toInt();
    }
    // Get acked attribute (only if acked parameter is not null, not applicable for logging metadata)
    if ( acked != NULL)
    {
        elemAttr = elemAttributes.namedItem("acked");
        if ( elemAttr.isNull())
        {
            return QString("Object:telemetrygcs:acked attribute is missing");
        }
        else
        {
            if ( elemAttr.nodeValue().compare(QString("true")) == 0 )
            {
                *acked = true;
            }
            else if ( elemAttr.nodeValue().compare(QString("false")) == 0 )
            {
                *acked = false;
            }
            else
            {
                return QString("Object:telemetrygcs:acked attribute value is invalid");
            }
        }
    }
    // Done
    return QString();
}

/**
 * Process the object fields of the XML
 */
QString UAVObjectParser::processObjectFields(QDomNode& childNode, ObjectInfo* info)
{
    // Create field
    FieldInfo* field = new FieldInfo;
    // Get name attribute
    QDomNamedNodeMap elemAttributes = childNode.attributes();
    QDomNode elemAttr = elemAttributes.namedItem("name");
    if ( elemAttr.isNull() )
    {
        return QString("Object:field:name attribute is missing");
    }
    else
    {
        field->name = elemAttr.nodeValue();
    }
    // Get units attribute
    elemAttr = elemAttributes.namedItem("units");
    if ( elemAttr.isNull() )
    {
        return QString("Object:field:units attribute is missing");
    }
    else
    {
        field->units = elemAttr.nodeValue();
    }
    // Get type attribute
    elemAttr = elemAttributes.namedItem("type");
    if ( elemAttr.isNull() )
    {
        return QString("Object:field:type attribute is missing");
    }
    else
    {
        if ( elemAttr.nodeValue().compare(QString("int8")) == 0 )
        {
            field->type = FIELDTYPE_INT8;
        }
        else if ( elemAttr.nodeValue().compare(QString("int16")) == 0 )
        {
            field->type = FIELDTYPE_INT16;
        }
        else if ( elemAttr.nodeValue().compare(QString("int32")) == 0 )
        {
            field->type = FIELDTYPE_INT32;
        }
        else if ( elemAttr.nodeValue().compare(QString("float")) == 0 )
        {
            field->type = FIELDTYPE_FLOAT32;
        }
        else if ( elemAttr.nodeValue().compare(QString("char")) == 0 )
        {
            field->type = FIELDTYPE_CHAR;
        }
        else
        {
            return QString("Object:field:type attribute value is invalid");
        }
    }
    // Get numelements attribute
    elemAttr = elemAttributes.namedItem("elements");
    if ( elemAttr.isNull() )
    {
        return QString("Object:field:elements attribute is missing");
    }
    else
    {
        field->numElements = elemAttr.nodeValue().toInt();
    }
    // Add field to object
    info->fields.append(field);
    // Done
    return QString();
}

/**
 * Process the object attributes from the XML
 */
QString UAVObjectParser::processObjectAttributes(QDomNode& node, ObjectInfo* info)
{
    // Get name attribute
    QDomNamedNodeMap attributes = node.attributes();
    QDomNode attr = attributes.namedItem("name");
    if ( attr.isNull() )
    {
        return QString("Object:name attribute is missing");
    }
    else
    {
        info->name = attr.nodeValue();
    }
    // Get singleinstance attribute
    attr = attributes.namedItem("singleinstance");
    if ( attr.isNull() )
    {
        return QString("Object:singleinstance attribute is missing");
    }
    else
    {
        if ( attr.nodeValue().compare(QString("true")) == 0 )
        {
            info->isSingleInst = true;
        }
        else if ( attr.nodeValue().compare(QString("false")) == 0 )
        {
            info->isSingleInst = false;
        }
        else
        {
            return QString("Object:singleinstance attribute value is invalid");
        }
    }
    // Get settings attribute
    attr = attributes.namedItem("settings");
    if ( attr.isNull() )
    {
        return QString("Object:settings attribute is missing");
    }
    else
    {
        if ( attr.nodeValue().compare(QString("true")) == 0 )
        {
            info->isSettings = true;
        }
        else if ( attr.nodeValue().compare(QString("false")) == 0 )
        {
            info->isSettings = false;
        }
        else
        {
            return QString("Object:settings attribute value is invalid");
        }
    }
    // Settings objects can only have a single instance
    if ( info->isSettings && !info->isSingleInst )
    {
        return QString("Object: Settings objects can not have multiple instances");
    }
    // Done
    return QString();
}

/**
 * Replace all the common tags from the template file with actual object
 * information.
 */
void UAVObjectParser::replaceCommonTags(QString& out, ObjectInfo* info)
{
    QString value;
    // Replace $(XMLFILE) tag
    out.replace(QString("$(XMLFILE)"), filename);
    // Replace $(NAME) tag
    out.replace(QString("$(NAME)"), info->name);
    // Replace $(NAMELC) tag
    out.replace(QString("$(NAMELC)"), info->name.toLower());
    // Replace $(NAMEUC) tag
    out.replace(QString("$(NAMEUC)"), info->name.toUpper());
    // Replace $(OBJID) tag
    out.replace(QString("$(OBJID)"), QString().setNum(info->id));
    // Replace $(ISSINGLEINST) tag
    value = boolToString( info->isSingleInst );
    out.replace(QString("$(ISSINGLEINST)"), value);
    // Replace $(ISSETTINGS) tag
    value = boolToString( info->isSettings );
    out.replace(QString("$(ISSETTINGS)"), value);
    // Replace $(FLIGHTTELEM_ACKED) tag
    value = boolToString( info->flightTelemetryAcked );
    out.replace(QString("$(FLIGHTTELEM_ACKED)"), value);
    // Replace $(FLIGHTTELEM_UPDATEMODE) tag
    value = UpdateModeStr[info->flightTelemetryUpdateMode];
    out.replace(QString("$(FLIGHTTELEM_UPDATEMODE)"), value);
    // Replace $(FLIGHTTELEM_UPDATEPERIOD) tag
    out.replace(QString("$(FLIGHTTELEM_UPDATEPERIOD)"), QString().setNum(info->flightTelemetryUpdatePeriod));
    // Replace $(GCSTELEM_ACKED) tag
    value = boolToString( info->gcsTelemetryAcked );
    out.replace(QString("$(GCSTELEM_ACKED)"), value);
    // Replace $(GCSTELEM_UPDATEMODE) tag
    value = UpdateModeStr[info->gcsTelemetryUpdateMode];
    out.replace(QString("$(GCSTELEM_UPDATEMODE)"), value);
    // Replace $(GCSTELEM_UPDATEPERIOD) tag
    out.replace(QString("$(GCSTELEM_UPDATEPERIOD)"), QString().setNum(info->gcsTelemetryUpdatePeriod));
    // Replace $(LOGGING_UPDATEMODE) tag
    value = UpdateModeStr[info->loggingUpdateMode];
    out.replace(QString("$(LOGGING_UPDATEMODE)"), value);
    // Replace $(LOGGING_UPDATEPERIOD) tag
    out.replace(QString("$(LOGGING_UPDATEPERIOD)"), QString().setNum(info->loggingUpdatePeriod));
}

/**
 * Convert a boolean to string
 */
QString UAVObjectParser::boolToString(bool value)
{
    if ( value )
    {
        return QString("1");
    }
    else
    {
        return QString("0");
    }
}

/**
 * Generate the flight object files
 */
bool UAVObjectParser::generateFlightObject(int objIndex, const QString& templateInclude, const QString& templateCode,
                                              QString& outInclude, QString& outCode)
{
    // Get object
    ObjectInfo* info = objInfo[objIndex];
    if (info == NULL) return false;

    // Prepare output strings
    outInclude = templateInclude;
    outCode = templateCode;

    // Replace common tags
    replaceCommonTags(outInclude, info);
    replaceCommonTags(outCode, info);

    // Replace the $(DATAFIELDS) tag
    QString type;
    QString fields;
    for (int n = 0; n < info->fields.length(); ++n)
    {
        // Determine type
        type = FieldTypeStrC[info->fields[n]->type];
        // Append field
        if ( info->fields[n]->numElements > 1 )
        {
            fields.append( QString("    %1 %2[%3];\n").arg(type)
                           .arg(info->fields[n]->name).arg(info->fields[n]->numElements) );
        }
        else
        {
            fields.append( QString("    %1 %2;\n").arg(type).arg(info->fields[n]->name) );
        }
    }
    outInclude.replace(QString("$(DATAFIELDS)"), fields);

    // Done
    return true;
}

/**
 * Generate the GCS object files
 */
bool UAVObjectParser::generateGCSObject(int objIndex, const QString& templateInclude, const QString& templateCode,
                                           QString& outInclude, QString& outCode)
{
    // Get object
    ObjectInfo* info = objInfo[objIndex];
    if (info == NULL) return false;

    // Prepare output strings
    outInclude = templateInclude;
    outCode = templateCode;

    // Replace common tags
    replaceCommonTags(outInclude, info);
    replaceCommonTags(outCode, info);

    // Replace the $(DATAFIELDS) tag
    QString type;
    QString fields;
    for (int n = 0; n < info->fields.length(); ++n)
    {
        // Determine type
        type = FieldTypeStrCPP[info->fields[n]->type];
        // Append field
        if ( info->fields[n]->numElements > 1 )
        {
            fields.append( QString("        %1 %2[%3];\n").arg(type).arg(info->fields[n]->name)
                           .arg(info->fields[n]->numElements) );
        }
        else
        {
            fields.append( QString("        %1 %2;\n").arg(type).arg(info->fields[n]->name) );
        }
    }
    outInclude.replace(QString("$(DATAFIELDS)"), fields);

    // Replace the $(FIELDSINIT) tag
    QString finit;
    for (int n = 0; n < info->fields.length(); ++n)
    {
        finit.append( QString("    fields.append(new UAVObjectField(QString(\"%1\"), QString(\"%2\"), UAVObjectField::%3, %4));\n")
                      .arg(info->fields[n]->name)
                      .arg(info->fields[n]->units)
                      .arg(FieldTypeStr[info->fields[n]->type])
                      .arg(info->fields[n]->numElements) );
    }
    outCode.replace(QString("$(FIELDSINIT)"), finit);

    // Done
    return true;
}
