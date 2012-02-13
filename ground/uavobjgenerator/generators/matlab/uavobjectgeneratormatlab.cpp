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

bool UAVObjectGeneratorMatlab::generate(UAVObjectParser* parser,QString templatepath,QString outputpath) {

    fieldTypeStrMatlab << "int8" << "int16" << "int32"
        << "uint8" << "uint16" << "uint32" << "float32" << "uint8";

    QDir matlabTemplatePath = QDir( templatepath + QString("ground/openpilotgcs/src/plugins/uavobjects"));
    QDir matlabOutputPath = QDir( outputpath + QString("matlab") );
    matlabOutputPath.mkpath(matlabOutputPath.absolutePath());

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
    matlabCodeTemplate.replace( QString("$(SWITCHCODE)"), matlabSwitchCode);
    matlabCodeTemplate.replace( QString("$(CLEANUPCODE)"), matlabCleanupCode);
    matlabCodeTemplate.replace( QString("$(SAVEOBJECTSCODE)"), matlabSaveObjectsCode);
    matlabCodeTemplate.replace( QString("$(FUNCTIONSCODE)"), matlabFunctionsCode);
    matlabCodeTemplate.replace( QString("$(EXPORTCSVCODE)"), matlabExportCsvCode);

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

	//Declare variables
    QString objectName(info->name);
    // QString objectTableName(objectName + "Objects");
    QString objectTableName(objectName);
    QString tableIdxName(objectName.toLower() + "Idx");
    QString functionName("Read" + info->name + "Object");
    QString functionCall(functionName + "(fid, timestamp, checkCRC, ");
    QString objectID(QString().setNum(info->id));
    QString isSingleInst = boolTo01String( info->isSingleInst );

	
	//===================================================================//
    // Generate allocation code (will replace the $(ALLOCATIONCODE) tag) //
	//===================================================================//
	//    matlabSwitchCode.append("\t\tcase " + objectID + "\n");
    matlabAllocationCode.append("\n\t" + tableIdxName + " = 0;\n");
	QString type;
	QString allocfields;
	if (0){
		matlabAllocationCode.append("\t" + objectTableName + ".timestamp = 0;\n");
		for (int n = 0; n < info->fields.length(); ++n) {
			// Determine type
			type = fieldTypeStrMatlab[info->fields[n]->type];
			// Append field
			if ( info->fields[n]->numElements > 1 )
				allocfields.append("\t" + objectTableName + "(1)." + info->fields[n]->name + " = zeros(" + QString::number(info->fields[n]->numElements, 10) + ",1);\n");
			else
				allocfields.append("\t" + objectTableName + "(1)." + info->fields[n]->name + " = 0;\n");
		}
	}
	else{
		matlabAllocationCode.append("\t" + objectTableName + "=struct('timestamp', 0");
		if (!info->isSingleInst) {
			allocfields.append(",...\n\t\t 'instanceID', 0");
		}
		for (int n = 0; n < info->fields.length(); ++n) {
			// Determine type
			type = fieldTypeStrMatlab[info->fields[n]->type];
			// Append field
			if ( info->fields[n]->numElements > 1 )
				allocfields.append(",...\n\t\t '" + info->fields[n]->name + "', zeros(" + QString::number(info->fields[n]->numElements, 10) + ",1)");
			else
				allocfields.append(",...\n\t\t '" + info->fields[n]->name + "', 0");
		}
		allocfields.append(");\n");
    }
    matlabAllocationCode.append(allocfields);
    matlabAllocationCode.append("\t" + objectTableName.toUpper() + "_OBJID=" + objectID + ";\n");


    //==============================================================//
    // Generate 'Switch:' code (will replace the $(SWITCHCODE) tag) //
    //==============================================================//
	

	
    matlabSwitchCode.append("\t\tcase " + objectTableName.toUpper() + "_OBJID\n");
//	matlabSwitchCode.append("\t\t\t" + objectTableName + "(" + tableIdxName +") = " + functionCall + ";\n");
	matlabSwitchCode.append("\t\t\t" + tableIdxName + " = " + tableIdxName +" + 1;\n");
	matlabSwitchCode.append("\t\t\t" + objectTableName + "= " + functionCall + objectTableName + ", " + tableIdxName + ");\n");
    matlabSwitchCode.append("\t\t\tif " + tableIdxName + " >= length(" + objectTableName +") %Check to see if pre-allocated memory is exhausted\n");
    matlabSwitchCode.append("\t\t\t\tFieldNames= fieldnames(" + objectTableName +");\n");
    matlabSwitchCode.append("\t\t\t\tfor i=1:length(FieldNames) %Grow structure\n");
    matlabSwitchCode.append("\t\t\t\t\t" + objectTableName + ".(FieldNames{i})(:," + tableIdxName + "*2+1) = 0;\n");
    matlabSwitchCode.append("\t\t\t\tend\n");
    matlabSwitchCode.append("\t\t\tend\n");
	
	
    //============================================================//
    // Generate 'Cleanup:' code (will replace the $(CLEANUP) tag) //
    //============================================================//
    matlabCleanupCode.append(objectTableName + "=PruneStructOfArrays(" + objectTableName + "," + tableIdxName +");\n" );

	
    //========================================================================//
    // Generate objects saving code (will replace the $(SAVEOBJECTSCODE) tag) //
    //========================================================================//
    matlabSaveObjectsCode.append(",'"+objectTableName+"'");


    //==========================================================================//
    // Generate objects csv export code (will replace the $(EXPORTCSVCODE) tag) //
    //==========================================================================//
    matlabExportCsvCode.append("\tOPLog2csv(" + objectTableName + ", '"+objectTableName+"', logfile);\n");
//	OPLog2csv(ActuatorCommand, 'ActuatorCommand', logfile)
	
	//=================================================================//
    // Generate functions code (will replace the $(FUNCTIONSCODE) tag) //
	//=================================================================//
    //Generate function description comment
    matlabFunctionsCode.append("%%\n% " + objectName + " read function\n");
	
    matlabFunctionsCode.append("function [" + objectName + "] = " + functionCall + objectTableName + ", " + tableIdxName + ")" + "\n");

    
    matlabFunctionsCode.append("\t" + objectName + ".timestamp(" + tableIdxName + ")= timestamp;\n");
    matlabFunctionsCode.append("\tif " + isSingleInst + "\n");
    matlabFunctionsCode.append("\t\theaderSize = 8;\n");
    matlabFunctionsCode.append("\telse\n");
    matlabFunctionsCode.append("\t\t" + objectName + ".instanceID(" + tableIdxName + ") = (fread(fid, 1, 'uint16'));\n");
    matlabFunctionsCode.append("\t\theaderSize = 10;\n");
    matlabFunctionsCode.append("\tend\n\n");

    // Generate functions code, actual fields of the object
    QString funcfields;

    matlabFunctionsCode.append("\tstartPos = ftell(fid) - headerSize;\n");    

    for (int n = 0; n < info->fields.length(); ++n) {
        // Determine type
        type = fieldTypeStrMatlab[info->fields[n]->type];
        // Append field
        if ( info->fields[n]->numElements > 1 )
            funcfields.append("\t" + objectName + "." + info->fields[n]->name + "(:," + tableIdxName + ") = double(fread(fid, " + QString::number(info->fields[n]->numElements, 10) + ", '" + type + "'));\n");
        else
            funcfields.append("\t" + objectName + "." + info->fields[n]->name + "(" + tableIdxName + ") = double(fread(fid, 1, '" + type + "'));\n");
    }
    matlabFunctionsCode.append(funcfields);

    matlabFunctionsCode.append("\tobjLen = ftell(fid) - startPos;\n");

    matlabFunctionsCode.append("\t% read CRC\n");
    matlabFunctionsCode.append("\tcrc_read = fread(fid, 1, '*uint8');\n");

    matlabFunctionsCode.append("\tif checkCRC\n");
    matlabFunctionsCode.append("\t\tfseek(fid, startPos, 'bof');\n");
    matlabFunctionsCode.append("\t\tcrc_calc = compute_crc(uint8(fread(fid,objLen,'uint8')));\n");
    matlabFunctionsCode.append("\t\tfread(fid,1,'uint8');\n");
    matlabFunctionsCode.append("\t\tif (crc_calc ~= crc_read)\n");    
    matlabFunctionsCode.append("\t\t\tdisp('CRC Error')\n");
    matlabFunctionsCode.append("\t\t\t" + tableIdxName + " = " + tableIdxName + " - 1;\n");
    matlabFunctionsCode.append("\t\tend\n");
    matlabFunctionsCode.append("\tend\n");

    matlabFunctionsCode.append("\n\n");

    return true;
}
