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


/**
 * Constructor
 */
UAVObjectParser::UAVObjectParser()
{
    fieldTypeStrC << QString("int8_t") << QString("int16_t") << QString("int32_t") <<
                     QString("uint8_t") << QString("uint16_t") << QString("uint32_t") <<
                     QString("float") << QString("uint8_t");

    fieldTypeStrCPP << QString("qint8") << QString("qint16") << QString("qint32") <<
                       QString( "quint8") << QString("quint16") << QString("quint32") <<
                       QString("float") << QString("quint8");

    fieldTypeStrCPPClass << QString("Int8") << QString("Int16") << QString("Int32") <<
                            QString( "UInt8") << QString("UInt16") << QString("UInt32") <<
                            QString("Float") << QString("Enum");

    fieldTypeStrXML << QString("int8") << QString("int16") << QString("int32") <<
                       QString("uint8") << QString("uint16") << QString("uint32") <<
                       QString("float") << QString("enum");

    updateModeStr << QString("UPDATEMODE_PERIODIC") << QString("UPDATEMODE_ONCHANGE") <<
                     QString("UPDATEMODE_MANUAL") << QString("UPDATEMODE_NEVER");

    updateModeStrXML << QString("periodic") << QString("onchange") <<
                        QString("manual") << QString("never");
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
        int index = updateModeStrXML.indexOf( elemAttr.nodeValue() );
        if (index >= 0)
        {
            *mode = (UpdateMode)index;
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
        int index = fieldTypeStrXML.indexOf(elemAttr.nodeValue());
        if (index >= 0)
        {
            field->type = (FieldType)index;
        }
        else
        {
            return QString("Object:field:type attribute value is invalid");
        }
    }
    // Get numelements or elementnames attribute
    elemAttr = elemAttributes.namedItem("elementnames");
    if ( elemAttr.isNull() )
    {
        elemAttr = elemAttributes.namedItem("elements");
        if ( elemAttr.isNull() )
        {
            return QString("Object:field:elements attribute is missing");
        }
        else
        {
            field->numElements = elemAttr.nodeValue().toInt();
            for (int n = 0; n < field->numElements; ++n)
            {
                field->elementNames.append(QString("[%1]").arg(n));
            }
            field->defaultElementNames = true;
        }
    }
    else
    {
        // Get element names
        QStringList names = elemAttr.nodeValue().split(",", QString::SkipEmptyParts);
        for (int n = 0; n < names.length(); ++n)
        {
            names[n] = names[n].trimmed();
        }
        field->elementNames = names;
        field->numElements = names.length();
        field->defaultElementNames = false;
    }
    // Get options attribute (only if an enum type)
    if (field->type == FIELDTYPE_ENUM)
    {
        // Get options attribute
        elemAttr = elemAttributes.namedItem("options");
        if ( elemAttr.isNull() )
        {
            return QString("Object:field:options attribute is missing");
        }
        else
        {
            QStringList options = elemAttr.nodeValue().split(",", QString::SkipEmptyParts);
            for (int n = 0; n < options.length(); ++n)
            {
                options[n] = options[n].trimmed();
            }
            field->options = options;
        }
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
    value = updateModeStr[info->flightTelemetryUpdateMode];
    out.replace(QString("$(FLIGHTTELEM_UPDATEMODE)"), value);
    // Replace $(FLIGHTTELEM_UPDATEPERIOD) tag
    out.replace(QString("$(FLIGHTTELEM_UPDATEPERIOD)"), QString().setNum(info->flightTelemetryUpdatePeriod));
    // Replace $(GCSTELEM_ACKED) tag
    value = boolToString( info->gcsTelemetryAcked );
    out.replace(QString("$(GCSTELEM_ACKED)"), value);
    // Replace $(GCSTELEM_UPDATEMODE) tag
    value = updateModeStr[info->gcsTelemetryUpdateMode];
    out.replace(QString("$(GCSTELEM_UPDATEMODE)"), value);
    // Replace $(GCSTELEM_UPDATEPERIOD) tag
    out.replace(QString("$(GCSTELEM_UPDATEPERIOD)"), QString().setNum(info->gcsTelemetryUpdatePeriod));
    // Replace $(LOGGING_UPDATEMODE) tag
    value = updateModeStr[info->loggingUpdateMode];
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
        type = fieldTypeStrC[info->fields[n]->type];
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

    // Replace the $(DATAENUM) tag
    QString name;
    QString enums;
    for (int n = 0; n < info->fields.length(); ++n)
    {
        // Only for enum types
        if (info->fields[n]->type == FIELDTYPE_ENUM)
        {
            enums.append(QString("/* Enumeration options for field %1 */\n").arg(info->fields[n]->name));
            enums.append("typedef enum { ");
            // Go through each option
            QStringList options = info->fields[n]->options;
            for (int m = 0; m < options.length(); ++m)
            {
                enums.append( QString("%1_%2_%3=%4, ")
                                .arg( info->name.toUpper() )
                                .arg( info->fields[n]->name.toUpper() )
                                .arg( options[m].toUpper() )
                                .arg(m) );

            }
            enums.append( QString(" } %1%2Options;\n")
                          .arg( info->name )
                          .arg( info->fields[n]->name ) );
        }
        // Generate element names (only if field has more than one element)
        if (info->fields[n]->numElements > 1 && !info->fields[n]->defaultElementNames)
        {
            enums.append(QString("/* Array element names for field %1 */\n").arg(info->fields[n]->name));
            enums.append("typedef enum { ");
            // Go through the element names
            QStringList elemNames = info->fields[n]->elementNames;
            for (int m = 0; m < elemNames.length(); ++m)
            {
                enums.append( QString("%1_%2_%3=%4, ")
                                .arg( info->name.toUpper() )
                                .arg( info->fields[n]->name.toUpper() )
                                .arg( elemNames[m].toUpper() )
                                .arg(m) );

            }
            enums.append( QString(" } %1%2Elem;\n")
                          .arg( info->name )
                          .arg( info->fields[n]->name ) );
        }
    }
    outInclude.replace(QString("$(DATAENUM)"), enums);

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
        type = fieldTypeStrCPP[info->fields[n]->type];
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
        // Setup element names
        QString varElemName = info->fields[n]->name + "ElemNames";
        finit.append( QString("    QStringList %1;\n").arg(varElemName) );
        QStringList elemNames = info->fields[n]->elementNames;
        for (int m = 0; m < elemNames.length(); ++m)
        {
            finit.append( QString("    %1.append(\"%2\");\n")
                          .arg(varElemName)
                          .arg(elemNames[m]) );
        }        
        // Only for enum types
        if (info->fields[n]->type == FIELDTYPE_ENUM)
        {
            QString varOptionName = info->fields[n]->name + "EnumOptions";
            finit.append( QString("    QStringList %1;\n").arg(varOptionName) );
            QStringList options = info->fields[n]->options;
            for (int m = 0; m < options.length(); ++m)
            {
                finit.append( QString("    %1.append(\"%2\");\n")
                              .arg(varOptionName)
                              .arg(options[m]) );
            }
            finit.append( QString("    fields.append(new UAVObjectFieldEnum(QString(\"%1\"), QString(\"%2\"), %3, %4));\n")
                          .arg(info->fields[n]->name)
                          .arg(info->fields[n]->units)
                          .arg(varElemName)
                          .arg(varOptionName) );
        }
        // For all other types
        else
        {
            finit.append( QString("    fields.append(new UAVObjectField%1(QString(\"%2\"), QString(\"%3\"), %4));\n")
                          .arg(fieldTypeStrCPPClass[info->fields[n]->type])
                          .arg(info->fields[n]->name)
                          .arg(info->fields[n]->units)
                          .arg(varElemName) );
        }
    }
    outCode.replace(QString("$(FIELDSINIT)"), finit);

    // Replace the $(DATAENUM) tag
    QString name;
    QString enums;
    for (int n = 0; n < info->fields.length(); ++n)
    {
        // Only for enum types
        if (info->fields[n]->type == FIELDTYPE_ENUM)
        {
            enums.append(QString("    /* Enumeration options for field %1 */\n").arg(info->fields[n]->name));
            enums.append("    typedef enum { ");
            // Go through each option
            QStringList options = info->fields[n]->options;
            for (int m = 0; m < options.length(); ++m)
            {
                enums.append( QString("%1_%2=%3, ")
                                .arg( info->fields[n]->name.toUpper() )
                                .arg( options[m].toUpper() )
                                .arg(m) );

            }
            enums.append( QString(" } %1Options;\n")
                          .arg( info->fields[n]->name ) );
        }
        // Generate element names (only if field has more than one element)
        if (info->fields[n]->numElements > 1 && !info->fields[n]->defaultElementNames)
        {
            enums.append(QString("    /* Array element names for field %1 */\n").arg(info->fields[n]->name));
            enums.append("    typedef enum { ");
            // Go through the element names
            QStringList elemNames = info->fields[n]->elementNames;
            for (int m = 0; m < elemNames.length(); ++m)
            {
                enums.append( QString("%1_%2=%3, ")
                                .arg( info->fields[n]->name.toUpper() )
                                .arg( elemNames[m].toUpper() )
                                .arg(m) );

            }
            enums.append( QString(" } %1Elem;\n")
                          .arg( info->fields[n]->name ) );
        }
    }
    outInclude.replace(QString("$(DATAENUM)"), enums);

    // Done
    return true;
}
