/**
 ******************************************************************************
 *
 * @file       uavobjectgeneratorjson.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      produce json code for uavobjects
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

#include "uavobjectgeneratorjson.h"
using namespace std;

bool UAVObjectGeneratorJson::generate(UAVObjectParser *parser, QString templatepath, QString outputpath)
{
    // Load template and setup output directory
    jsonCodePath     = QDir(templatepath + QString("flight/modules/FlightPlan/lib"));
    jsonOutputPath   = QDir(outputpath + QString("json"));
    jsonOutputPath.mkpath(jsonOutputPath.absolutePath());
    jsonCodeTemplate = readFile(jsonCodePath.absoluteFilePath("uavobject.json.template"));
    if (jsonCodeTemplate.isEmpty()) {
        std::cerr << "Problem reading json templates" << endl;
        return false;
    }

    // Process each object
    for (int objidx = 0; objidx < parser->getNumObjects(); ++objidx) {
        ObjectInfo *info = parser->getObjectByIndex(objidx);
        process_object(info);
    }

    return true; // if we come here everything should be fine
}

/**
 * Generate the json object files
 */
bool UAVObjectGeneratorJson::process_object(ObjectInfo *info)
{
    if (info == NULL) {
        return false;
    }

    // Prepare output strings
    QString outCode = jsonCodeTemplate;

    // Replace common tags
    replaceCommonTags(outCode, info);

    // Replace the ($DATAFIELDS) tag
    QStringList datafields;
    for (int n = 0; n < info->fields.length(); ++n) {
         QString f = "{\n";
        // Class header
        f.append(QString("  \"name\": \"%1\",\n").arg(info->fields[n]->name));
        f.append(QString("  \"type\": %1,\n").arg(info->fields[n]->type));
        f.append(QString("  \"numElements\": %1\n").arg(info->fields[n]->numElements));
        // Only for enum types
        if (info->fields[n]->type == FIELDTYPE_ENUM) {
            /*
            datafields.append(QString("    # Enumeration options\n"));
            // Go through each option
            QStringList options = info->fields[n]->options;
            for (int m = 0; m < options.length(); ++m) {
                QString name = options[m].toUpper().replace(QRegExp(ENUM_SPECIAL_CHARS), "");
                if (name[0].isDigit()) {
                    name = QString("N%1").arg(name);
                }
                datafields.append(QString("    %1 = %2\n").arg(name).arg(m));
            }
            */
        }
        // Generate element names (only if field has more than one element)
        if (info->fields[n]->numElements > 1 && !info->fields[n]->defaultElementNames) {
            /*
            datafields.append(QString("    # Array element names\n"));
            // Go through the element names
            QStringList elemNames = info->fields[n]->elementNames;
            for (int m = 0; m < elemNames.length(); ++m) {
                QString name = elemNames[m].toUpper().replace(QRegExp(ENUM_SPECIAL_CHARS), "");
                if (name[0].isDigit()) {
                    name = QString("N%1").arg(name);
                }
                datafields.append(QString("    %1 = %2\n").arg(name).arg(m));
            }
            */
        }
        f.append("}");
        datafields.append(f);
    }
    outCode.replace(QString("$(DATAFIELDS)"), datafields.join(",\n"));


    // Write the Json code
    bool res = writeFileIfDiffrent(jsonOutputPath.absolutePath() + "/" + info->namelc + ".json", outCode);
    if (!res) {
        cout << "Error: Could not write Json output files" << endl;
        return false;
    }

    return true;
}
