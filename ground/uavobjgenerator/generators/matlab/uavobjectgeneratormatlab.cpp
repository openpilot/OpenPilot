/**
 ******************************************************************************
 *
 * @file       uavobjectgeneratormatlab.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      produce matlab code for uavobjects
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

#include "uavobjectgeneratormatlab.h"

using namespace std;

bool UAVObjectGeneratorMatlab::generate(UAVObjectParser* parser,QString basepath,QString outputpath) {

    fieldTypeStrMatlab << "int8" << "int16" << "int32"
        << "uint8" << "uint16" << "uint32" << "float32" << "uint8";

    QDir matlabTemplatePath = QDir( basepath + QString("ground/src/plugins/uavobjects"));
    QDir matlabOutputPath = QDir( outputpath + QString("matlab"));

    QString matlabCodeTemplate = readFile( matlabTemplatePath.absoluteFilePath( "uavobjecttemplate.m") );

    if (matlabCodeTemplate.isEmpty() ) {
        std::cerr << "Problem reading matlab templates" << endl;
        return false;
    }

    for (int objidx = 0; objidx < parser->getNumObjects(); ++objidx) {
        ObjectInfo* info=parser->getObjectByIndex(objidx);
        process_object(info);
    }

    matlabCodeTemplate.replace( QString("$(ALLOCATIONCODE)"), matlabAllocationCode);
    matlabCodeTemplate.replace( QString("$(SWITCHCODE)"), matlabSwithCode);
    matlabCodeTemplate.replace( QString("$(SAVEOBJECTSCODE)"), matlabSaveObjectsCode);
    matlabCodeTemplate.replace( QString("$(FUNCTIONSCODE)"), matlabFunctionsCode);

    bool res = writeFile( matlabOutputPath.absolutePath() + "/OPLogConvert.m", matlabCodeTemplate );
    if (!res) {
        cout << "Error: Could not write output files" << endl;
        return false;
    }

    return true; // if we come here everything should be fine
}

/**
 * Generate the matlab object files
 */
bool UAVObjectGeneratorMatlab::process_object(ObjectInfo* info)
{
    if (info == NULL)
        return false;

    QString objectName(info->name);
    // QString objectTableName(objectName + "Objects");
    QString objectTableName(objectName);
    QString tableIdxName(objectName.toLower() + "Idx");
    QString functionName("Read" + info->name + "Object");
    QString functionCall(functionName + "(fid, timestamp)");
    QString objectID(QString().setNum(info->id));
    QString isSingleInst = boolTo01String( info->isSingleInst );

    // Generate allocation code (will replace the $(ALLOCATIONCODE) tag)
    matlabAllocationCode.append("\n    " + tableIdxName + " = 1;\n");
    matlabAllocationCode.append("    " + objectTableName + ".timestamp = 0;\n");
    QString type;
    QString allocfields;
    for (int n = 0; n < info->fields.length(); ++n) {
        // Determine type
        type = fieldTypeStrMatlab[info->fields[n]->type];
        // Append field
        if ( info->fields[n]->numElements > 1 )
            allocfields.append("    " + objectTableName + "(1)." + info->fields[n]->name + " = zeros(1," + QString::number(info->fields[n]->numElements, 10) + ");\n");
        else
            allocfields.append("    " + objectTableName + "(1)." + info->fields[n]->name + " = 0;\n");
    }
    matlabAllocationCode.append(allocfields);

    // Generate 'swith:' code (will replace the $(SWITCHCODE) tag)
    matlabSwithCode.append("            case " + objectID + "\n");
    matlabSwithCode.append("                " + objectTableName + "(" + tableIdxName +") = " + functionCall + ";\n");
    matlabSwithCode.append("                " + tableIdxName + " = " + tableIdxName +" + 1;\n");

    // Generate objects saving code code (will replace the $(SAVEOBJECTSCODE) tag)
    matlabSaveObjectsCode.append(",'"+objectTableName+"'");

    // Generate functions code (will replace the $(FUNCTIONSCODE) tag)
    matlabFunctionsCode.append("function [" + objectName + "] = " + functionCall + "\n");
    matlabFunctionsCode.append("    if " + isSingleInst + "\n");
    matlabFunctionsCode.append("        headerSize = 8;\n");
    matlabFunctionsCode.append("    else\n");
    matlabFunctionsCode.append("        " + objectName + ".instanceID = fread(fid, 1, 'uint16');\n");
    matlabFunctionsCode.append("        headerSize = 10;\n");
    matlabFunctionsCode.append("    end\n\n");
    matlabFunctionsCode.append("    " + objectName + ".timestamp = timestamp;\n");

    // Generate functions code, actual fields of the object
    QString funcfields;

    for (int n = 0; n < info->fields.length(); ++n) {
        // Determine type
        type = fieldTypeStrMatlab[info->fields[n]->type];
        // Append field
        if ( info->fields[n]->numElements > 1 )
            funcfields.append("    " + objectName + "." + info->fields[n]->name + " = double(fread(fid, " + QString::number(info->fields[n]->numElements, 10) + ", '" + type + "'));\n");
        else
            funcfields.append("    " + objectName + "." + info->fields[n]->name + " = double(fread(fid, 1, '" + type + "'));\n");
    }
    matlabFunctionsCode.append(funcfields);

    matlabFunctionsCode.append("    % read CRC\n");
    matlabFunctionsCode.append("    fread(fid, 1, 'uint8');\n");

    matlabFunctionsCode.append("end\n\n");

    return true;
}
