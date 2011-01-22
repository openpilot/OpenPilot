/**
 ******************************************************************************
 *
 * @file       uavobjectgeneratorpython.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      produce python code for uavobjects
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

#include "uavobjectgeneratorpython.h"
using namespace std;

bool UAVObjectGeneratorPython::generate(UAVObjectParser* parser,QString templatepath,QString outputpath) {

    fieldTypeStrPython << "b" << "h" << "i" << "B" << "H" << "I" << "f" << "b";

    pythonCodePath = QDir( templatepath + QString("ground/openpilotgcs/src/plugins/uavobjects"));
    pythonOutputPath = QDir( outputpath + QString("python") );
    pythonOutputPath.mkpath(pythonOutputPath.absolutePath());

    pythonCodeTemplate = readFile( pythonCodePath.absoluteFilePath("uavobjecttemplate.py") );

    QString pythonImport,pythonObjInit;

    if (pythonCodeTemplate.isEmpty()) {
        std::cerr << "Problem reading python templates" << endl;
        return false;
    }

    for (int objidx = 0; objidx < parser->getNumObjects(); ++objidx) {
        ObjectInfo* info=parser->getObjectByIndex(objidx);
        process_object(info);

        // check if they are needed - are never written atm ..
        pythonImport.append("import " + info->namelc + "\n");
        pythonObjInit.append("\tuavobjectsinit.uavobjects.append(" + info->name + "." + info->name + "())\n");
    }

    return true; // if we come here everything should be fine
}

/**
 * Generate the python object files
 */
bool UAVObjectGeneratorPython::process_object(ObjectInfo* info)
{
    if (info == NULL)
        return false;

    // Prepare output strings
    QString outCode = pythonCodeTemplate;

    // Replace common tags
    replaceCommonTags(outCode, info);

    // Replace the $(DATAFIELDS) tag
    QString fields;

    fields.append(QString("[ \\\n"));
    for (int n = 0; n < info->fields.length(); ++n)
    {
        fields.append(QString("\tuavobject.UAVObjectField(\n"));
        fields.append(QString("\t\t'%1',\n").arg(info->fields[n]->name));
        fields.append(QString("\t\t'%1',\n").arg(fieldTypeStrPython[info->fields[n]->type]));
        fields.append(QString("\t\t%1,\n").arg(info->fields[n]->numElements));

        QStringList elemNames = info->fields[n]->elementNames;
        fields.append(QString("\t\t[\n"));
        for (int m = 0; m < elemNames.length(); ++m)
        {
            fields.append(QString("\t\t\t'%1',\n").arg(elemNames[m]));
        }
        fields.append(QString("\t\t],\n"));


        fields.append(QString("\t\t{\n"));
        if (info->fields[n]->type == FIELDTYPE_ENUM)
        {
            // Go through each option
            QStringList options = info->fields[n]->options;
            for (int m = 0; m < options.length(); ++m){
                fields.append( QString("\t\t\t'%1' : '%2',\n")
                    .arg(m)
                    .arg( options[m] ) );
            }
        }
        fields.append(QString("\t\t}\n"));
        fields.append(QString("\t),\n"));
    }
    fields.append(QString("]\n"));
    outCode.replace(QString("$(DATAFIELDS)"), fields);

    // Write the Python code
    bool res = writeFileIfDiffrent( pythonOutputPath.absolutePath() + "/" + info->namelc + ".py", outCode );
    if (!res) {
        cout << "Error: Could not write Python output files" << endl;
        return false;
    }

    return true;
}

