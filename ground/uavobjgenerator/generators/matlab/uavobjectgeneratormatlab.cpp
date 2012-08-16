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
        << "uint8" << "uint16" << "uint32" << "single" << "uint8";
    fieldSizeStrMatlab << "1" << "2" << "4"
        << "1" << "2" << "4" << "4" << "1";

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
        int numBytes=parser->getNumBytes(objidx);
        process_object(info, numBytes);
    }

    matlabCodeTemplate.replace( QString("$(INSTANTIATIONCODE)"), matlabInstantiationCode);
    matlabCodeTemplate.replace( QString("$(SWITCHCODE)"), matlabSwitchCode);
    matlabCodeTemplate.replace( QString("$(CLEANUPCODE)"), matlabCleanupCode);
    matlabCodeTemplate.replace( QString("$(SAVEOBJECTSCODE)"), matlabSaveObjectsCode);
    matlabCodeTemplate.replace( QString("$(ALLOCATIONCODE)"), matlabAllocationCode);
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
bool UAVObjectGeneratorMatlab::process_object(ObjectInfo* info, int numBytes)
{
    if (info == NULL)
        return false;

	//Declare variables
    QString objectName(info->name);
    // QString objectTableName(objectName + "Objects");
    QString objectTableName(objectName);
    QString tableIdxName(objectName.toLower() + "Idx");
    QString objectID(QString().setNum(info->id));
    QString numBytesString=QString("%1").arg(numBytes);


    //=========================================================================//
    // Generate instantiation code (will replace the $(INSTANTIATIONCODE) tag) //
    //=========================================================================//
    QString type;
    QString instantiationFields;

    matlabInstantiationCode.append("\n\t" + tableIdxName + " = 0;\n");
    matlabInstantiationCode.append("\t" + objectTableName + "=struct('timestamp', 0");
    if (!info->isSingleInst) {
        instantiationFields.append(",...\n\t\t 'instanceID', 0");
    }
    for (int n = 0; n < info->fields.length(); ++n) {
        // Determine type
        type = fieldTypeStrMatlab[info->fields[n]->type];
        // Append field
        if ( info->fields[n]->numElements > 1 )
            instantiationFields.append(",...\n\t\t '" + info->fields[n]->name + "', zeros(" + QString::number(info->fields[n]->numElements, 10) + ",1)");
        else
            instantiationFields.append(",...\n\t\t '" + info->fields[n]->name + "', 0");
    }
    instantiationFields.append(");\n");

    matlabInstantiationCode.append(instantiationFields);
    matlabInstantiationCode.append("\t" + objectTableName.toUpper() + "_OBJID=" + objectID + ";\n");
    matlabInstantiationCode.append("\t" + objectTableName.toUpper() + "_NUMBYTES=" + numBytesString + ";\n");
    matlabInstantiationCode.append("\t" + objectName + "FidIdx = [];\n");


    //==============================================================//
    // Generate 'Switch:' code (will replace the $(SWITCHCODE) tag) //
    //==============================================================//
    matlabSwitchCode.append("\t\tcase " + objectTableName.toUpper() + "_OBJID\n");
    matlabSwitchCode.append("\t\t\t" + tableIdxName + " = " + tableIdxName +" + 1;\n");
    matlabSwitchCode.append("\t\t\t" + objectTableName + "FidIdx(" + tableIdxName + ") = bufferIdx; %#ok<*AGROW>\n");
    matlabSwitchCode.append("\t\t\tbufferIdx=bufferIdx + " +  objectTableName.toUpper() + "_NUMBYTES+1; %+1 is for CRC\n");
    if(!info->isSingleInst){
        matlabSwitchCode.append("\t\t\tbufferIdx = bufferIdx + 2; %An extra two bytes for the instance ID\n");
    }
    matlabSwitchCode.append("\t\t\tif " + tableIdxName + " >= length(" + objectTableName +"FidIdx) %Check to see if pre-allocated memory is exhausted\n");
    matlabSwitchCode.append("\t\t\t\t" + objectTableName + "FidIdx(" + tableIdxName + "*2) = 0;\n");
    matlabSwitchCode.append("\t\t\tend\n");

	
    //============================================================//
    // Generate 'Cleanup:' code (will replace the $(CLEANUP) tag) //
    //============================================================//
    matlabCleanupCode.append(objectTableName + "FidIdx =" + objectTableName + "FidIdx(1:" + tableIdxName +");\n" );

    //=================================================================//
    // Generate functions code (will replace the $(ALLOCATIONCODE) tag) //
	//=================================================================//
    //Generate function description comment
    matlabAllocationCode.append("% " + objectName + " typecasting\n");
    QString allocationFields;

    //Add timestamp
    allocationFields.append("\t" + objectName + ".timestamp = " +
                      "double(typecast(buffer(mcolon(" + objectName + "FidIdx "
                      "- 20, " + objectName + "FidIdx + 4-1 -20)), 'uint32'))';\n");

    int currentIdx=0;

    //Add Instance ID, if necessary
    if(!info->isSingleInst){
        allocationFields.append("\t" + objectName + ".instanceID = " +
                          "double(typecast(buffer(mcolon(" + objectName + "FidIdx "
                          ", " + objectName + "FidIdx + 2-1)), 'uint16'))';\n");
        currentIdx+=2;
    }

    for (int n = 0; n < info->fields.length(); ++n) {
        // Determine variabel type
        type = fieldTypeStrMatlab[info->fields[n]->type];

        //Determine variable type length
        QString size = fieldSizeStrMatlab[info->fields[n]->type];
        // Append field
        if ( info->fields[n]->numElements > 1 ){
            allocationFields.append("\t" + objectName + "." + info->fields[n]->name + " = " +
                              "reshape(double(typecast(buffer(mcolon(" + objectName + "FidIdx + " + QString("%1").arg(currentIdx) +
                              ", " + objectName + "FidIdx + " + QString("%1").arg(currentIdx + size.toInt()*info->fields[n]->numElements - 1) + ")), '" + type + "')), "+ QString::number(info->fields[n]->numElements, 10) + ", [] );\n");
        }
        else{
            allocationFields.append("\t" + objectName + "." + info->fields[n]->name + " = " +
                              "double(typecast(buffer(mcolon(" + objectName + "FidIdx + " + QString("%1").arg(currentIdx) +
                              ", " + objectName + "FidIdx + " + QString("%1").arg(currentIdx + size.toInt() - 1) + ")), '" + type + "'))';\n");
        }
        currentIdx+=size.toInt()*info->fields[n]->numElements;
    }
    matlabAllocationCode.append(allocationFields);
    matlabAllocationCode.append("\n");


    //========================================================================//
    // Generate objects saving code (will replace the $(SAVEOBJECTSCODE) tag) //
    //========================================================================//
    matlabSaveObjectsCode.append(",'"+objectTableName+"'");


    //==========================================================================//
    // Generate objects csv export code (will replace the $(EXPORTCSVCODE) tag) //
    //==========================================================================//
    matlabExportCsvCode.append("\tOPLog2csv(" + objectTableName + ", '"+objectTableName+"', logfile);\n");
//	OPLog2csv(ActuatorCommand, 'ActuatorCommand', logfile)



    return true;
}
