/**
 ******************************************************************************
 *
 * @file       uavobjectgenerator.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      UAVObjectGenerator, generates UAVObjects from XML files.
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

/*
 * The following is done by the object generator:
 * - Read all XML files in the XML source directory
 * - Process each XML and extract the information for all defined objects
 * - Generate the unique objects IDs and check for collisions
 * - Generate the GCS and Flight UAVObject code based on the supplied template source files
 * - Update the object initialization source files for both the GCS and Flight code
 */

#include "uavobjectgenerator.h"

/**
 * Constructor
 * @param basepath The base directory where the 'flight' and 'gcs' folders are located (the SVN structure is expected)
 * @param out The stdout file
 */
UAVObjectGenerator::UAVObjectGenerator(QString& basepath, FILE* out) :
        basepath(basepath),
        sout(out)
{
    xmlPath = QDir( basepath + QString("ground/src/shared/uavobjectdefinition"));
    flightCodePath = QDir( basepath + QString("flight/OpenPilot/UAVObjects"));
    gcsCodePath = QDir( basepath + QString("ground/src/plugins/uavobjects"));
    pythonTemplatePath = QDir( basepath + QString("ground/src/plugins/uavobjects"));
    // synthetic output files should go into the build directory once the various GUI build systems
    // learn how to find these output files in the build directory.
    //pythonCodePath = QDir( basepath + QString("build/uavobjects"));
    pythonCodePath = QDir( basepath + QString("ground/src/plugins/uavobjects"));
    objectTemplateFilename = QString("uavobjecttemplate");
    objectsInitTemplateFilename = QString("uavobjectsinittemplate");
    objectsInitFilename = QString("uavobjectsinit");
    sout << "- OpenPilot UAVObject Generator -" << endl;
}

/**
 * Process all XML files and generate object code.
 */
bool UAVObjectGenerator::processAll()
{
    // Read the template files
    QString flightCodeTemplate = readFile( flightCodePath.absoluteFilePath(objectTemplateFilename + ".c") );
    QString flightIncludeTemplate = readFile( flightCodePath.absoluteFilePath("inc/" + objectTemplateFilename + ".h") );
    QString flightInitTemplate = readFile( flightCodePath.absoluteFilePath(objectsInitTemplateFilename + ".c") );
    QString gcsCodeTemplate = readFile( gcsCodePath.absoluteFilePath(objectTemplateFilename + ".cpp") );
    QString gcsIncludeTemplate = readFile( gcsCodePath.absoluteFilePath(objectTemplateFilename + ".h") );
    QString gcsInitTemplate = readFile( gcsCodePath.absoluteFilePath(objectsInitTemplateFilename + ".cpp") );
    QString pythonCodeTemplate = readFile( pythonTemplatePath.absoluteFilePath(objectTemplateFilename + ".py") );
    if ( flightCodeTemplate.isNull() || flightIncludeTemplate.isNull() ||
         gcsCodeTemplate.isNull() || gcsIncludeTemplate.isNull() ||
	 pythonCodeTemplate.isNull() ||
         flightInitTemplate.isNull() || gcsInitTemplate.isNull() )
    {
        sout << "Error: Could not open template files." << endl;
        return false;
    }

    // Get all XML files
    QStringList filters;
    filters.append("*.xml");
    xmlPath.setNameFilters(filters);
    QFileInfoList xmlList = xmlPath.entryInfoList();

    // Read in each XML file and parse it
    QList<UAVObjectParser*> parsers;
    for (int n = 0; n < xmlList.length(); ++n)
    {
        QFileInfo fileinfo = xmlList[n];
        QString filename = fileinfo.fileName();
        QString xmlstr = readFile(fileinfo.absoluteFilePath());
        UAVObjectParser* parser = new UAVObjectParser();
        sout << "Parsing XML file: " << fileinfo.fileName() << endl;
        QString res = parser->parseXML(xmlstr, filename);
        if (!res.isNull())
        {
            sout << "Error: " << res << endl;
            return false;
        }
        parsers.append(parser);
    }

    // Generate the code for each object and write it to the destination directory
    QList<quint32> objIDList;
    QString objInc;
    QString pythonImport;
    QString flightObjInit;
    QString gcsObjInit;
    QString pythonObjInit;
    bool res;
    for (int parseridx = 0; parseridx < parsers.length(); ++ parseridx)
    {
        UAVObjectParser* parser = parsers[parseridx];
        for (int objidx = 0; objidx < parser->getNumObjects(); ++objidx)
        {
            QString name = parser->getObjectName(objidx);
            QString namelc = name.toLower();
            sout << "Generating code for object: " << name << endl;
            // Check for object ID conflicts
            quint32 id = parser->getObjectID(objidx);
            if ( objIDList.contains(id) || id == 0 )
            {
                sout << "Error: Object ID collision found in object " << name << ", modify object name" << endl;
                return false;
            }
            else
            {
                objIDList.append(id);
            }
            // Generate the flight code
            QString flightCode;
            QString flightInclude;
            res = parser->generateFlightObject(objidx, flightIncludeTemplate, flightCodeTemplate,
                                               flightInclude, flightCode);
            if (!res)
            {
                sout << "Error: Improperly formatted flight object template file" << endl;
                return false;
            }
            // Generate the GCS code
            QString gcsCode;
            QString gcsInclude;
            res = parser->generateGCSObject(objidx, gcsIncludeTemplate, gcsCodeTemplate,
                                            gcsInclude, gcsCode);
            if (!res)
            {
                sout << "Error: Improperly formatted GCS object template file" << endl;
                return false;
            }
            // Generate the Python code
            QString pythonCode;
            res = parser->generatePythonObject(objidx, pythonCodeTemplate, pythonCode);
            if (!res)
            {
                sout << "Error: Improperly formatted Python object template file" << endl;
                return false;
            }
            // Write the flight code
            res = writeFileIfDiffrent( flightCodePath.absolutePath() + "/" + namelc + ".c", flightCode );
            if (!res)
            {
                sout << "Error: Could not write output files" << endl;
                return false;
            }
            res = writeFileIfDiffrent( flightCodePath.absolutePath() + "/inc/" + namelc + ".h", flightInclude );
            if (!res)
            {
                sout << "Error: Could not write output files" << endl;
                return false;
            }
            // Write the GCS code
            res = writeFileIfDiffrent( gcsCodePath.absolutePath() + "/" + namelc + ".cpp", gcsCode );
            if (!res)
            {
                sout << "Error: Could not write output files" << endl;
                return false;
            }
            res = writeFileIfDiffrent( gcsCodePath.absolutePath() + "/" + namelc + ".h", gcsInclude );
            if (!res)
            {
                sout << "Error: Could not write output files" << endl;
                return false;
            }
            // Write the Python code
            res = writeFileIfDiffrent( pythonCodePath.absolutePath() + "/" + namelc + ".py", pythonCode );
            if (!res)
            {
                sout << "Error: Could not write output files" << endl;
                return false;
            }
            // Update strings for the object initialization
            objInc.append("#include \"" + namelc + ".h\"\n");
	    pythonImport.append("import " + namelc + "\n");
            flightObjInit.append("    " + name + "Initialize();\n");
            gcsObjInit.append("    objMngr->registerObject( new " + name + "() );\n");
	    pythonObjInit.append("\tuavobjectsinit.uavobjects.append(" + name + "." + name + "())\n");
        }
    }

    // Write the flight object inialization files
    sout << "Updating object initialization files" << endl;
    flightInitTemplate.replace( QString("$(OBJINC)"), objInc);
    flightInitTemplate.replace( QString("$(OBJINIT)"), flightObjInit);
    res = writeFileIfDiffrent( flightCodePath.absolutePath() + "/" + objectsInitFilename + ".c",
                     flightInitTemplate );
    if (!res)
    {
        sout << "Error: Could not write output files" << endl;
        return false;
    }

    // Write the gcs object inialization files
    gcsInitTemplate.replace( QString("$(OBJINC)"), objInc);
    gcsInitTemplate.replace( QString("$(OBJINIT)"), gcsObjInit);
    res = writeFileIfDiffrent( gcsCodePath.absolutePath() + "/" + objectsInitFilename + ".cpp",
                     gcsInitTemplate );
    if (!res)
    {
        sout << "Error: Could not write output files" << endl;
        return false;
    }

    // Done
    sout << "Done: processed " << xmlList.length() << " XML files and generated "
            << objIDList.length() << " objects with no ID collisions." << endl;
    return true;
}

/**
 * Read a file and return its contents as a string
 */
QString UAVObjectGenerator::readFile(QString name)
{
    QFile file(name);
    if (!file.open(QFile::ReadOnly)) 
    {
        sout << "Warning: Could not open " << name << endl;
        return QString();
    }

    QTextStream fileStr(&file);
    QString str = fileStr.readAll();
    file.close();
    return str;
}

/**
 * Write contents of string to file
 */
bool UAVObjectGenerator::writeFile(QString name, QString& str)
{
    QFile file(name);
    if (!file.open(QFile::WriteOnly)) return false;
    QTextStream fileStr(&file);
    fileStr << str;
    file.close();
    return true;
}


/**
 * Write contents of string to file if the content changes
 */
bool UAVObjectGenerator::writeFileIfDiffrent(QString name, QString& str)
{
    if (readFile(name)==str) return true;
    return writeFile(name,str);
}

