/**
 ******************************************************************************
 *
 * @file       uavobjectgeneratorjava.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      produce java code for uavobjects
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

#include <QDebug>
#include "uavobjectgeneratorjava.h"
using namespace std;

bool UAVObjectGeneratorJava::generate(UAVObjectParser* parser,QString templatepath,QString outputpath) {
    fieldTypeStrCPP << "Byte" << "Short" << "Int" <<
        "Short" << "Int" << "Long" << "Float" << "Byte";

    fieldTypeStrCPPClass << "INT8" << "INT16" << "INT32"
        << "UINT8" << "UINT16" << "UINT32" << "FLOAT32" << "ENUM";

    javaCodePath = QDir( templatepath + QString(JAVA_TEMPLATE_DIR));
    javaOutputPath = QDir( outputpath + QString("java") );
    javaOutputPath.mkpath(javaOutputPath.absolutePath());

    javaCodeTemplate = readFile( javaCodePath.absoluteFilePath("uavobjecttemplate.java") );
    QString javaInitTemplate = readFile( javaCodePath.absoluteFilePath("uavobjectsinittemplate.java") );

    if (javaCodeTemplate.isEmpty() || javaInitTemplate.isEmpty()) {
        std::cerr << "Problem reading java code templates" << endl;
        return false;
    }

    QString objInc;
    QString javaObjInit;

    for (int objidx = 0; objidx < parser->getNumObjects(); ++objidx) {
        ObjectInfo* info=parser->getObjectByIndex(objidx);
        process_object(info);

        javaObjInit.append("\t\t\tobjMngr.registerObject( new " + info->name + "() );\n");
        objInc.append("#include \"" + info->namelc + ".h\"\n");
    }

    // Write the gcs object inialization files
    javaInitTemplate.replace( QString("$(OBJINC)"), objInc);
    javaInitTemplate.replace( QString("$(OBJINIT)"), javaObjInit);
    bool res = writeFileIfDiffrent( javaOutputPath.absolutePath() + "/UAVObjectsInitialize.java", javaInitTemplate );
    if (!res) {
        cout << "Error: Could not write output files" << endl;
        return false;
    }

    return true; // if we come here everything should be fine
}


/**
 * Generate the java object files
 */
bool UAVObjectGeneratorJava::process_object(ObjectInfo* info)
{
    if (info == NULL)
        return false;

    // Prepare output strings
    QString outInclude = javaIncludeTemplate;
    QString outCode = javaCodeTemplate;

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
        finit.append("\n");

        // Setup element names
        QString varElemName = info->fields[n]->name + "ElemNames";
        finit.append( QString("\t\tList<String> %1 = new ArrayList<String>();\n").arg(varElemName) );
        QStringList elemNames = info->fields[n]->elementNames;
        for (int m = 0; m < elemNames.length(); ++m)
            finit.append( QString("\t\t%1.add(\"%2\");\n")
                          .arg(varElemName)
                          .arg(elemNames[m]) );

        // Only for enum types
        if (info->fields[n]->type == FIELDTYPE_ENUM) {
            QString varOptionName = info->fields[n]->name + "EnumOptions";
            finit.append( QString("\t\tList<String> %1 = new ArrayList<String>();\n").arg(varOptionName) );
            QStringList options = info->fields[n]->options;
            for (int m = 0; m < options.length(); ++m)
            {
                finit.append( QString("\t\t%1.add(\"%2\");\n")
                              .arg(varOptionName)
                              .arg(options[m]) );
            }
            finit.append( QString("\t\tfields.add( new UAVObjectField(\"%1\", \"%2\", UAVObjectField.FieldType.ENUM, %3, %4) );\n")
                          .arg(info->fields[n]->name)
                          .arg(info->fields[n]->units)
                          .arg(varElemName)
                          .arg(varOptionName) );
        }
        // For all other types
        else {
            finit.append( QString("\t\tfields.add( new UAVObjectField(\"%1\", \"%2\", UAVObjectField.FieldType.%3, %4, null) );\n")
                          .arg(info->fields[n]->name)
                          .arg(info->fields[n]->units)
                          .arg(fieldTypeStrCPPClass[info->fields[n]->type])
                          .arg(varElemName) );
        }
    }
    outCode.replace(QString("$(FIELDSINIT)"), finit);

    // Replace the $(DATAFIELDINFO) tag
    QString name;
    QString enums;
    for (int n = 0; n < info->fields.length(); ++n)
    {
        enums.append(QString("    // Field %1 information\n").arg(info->fields[n]->name));
        // Only for enum types
        if (info->fields[n]->type == FIELDTYPE_ENUM)
        {
            enums.append(QString("    /* Enumeration options for field %1 */\n").arg(info->fields[n]->name));
            enums.append("    typedef enum { ");
            // Go through each option
            QStringList options = info->fields[n]->options;
            for (int m = 0; m < options.length(); ++m) {
                QString s = (m != (options.length()-1)) ? "%1_%2=%3, " : "%1_%2=%3";
                enums.append( s.arg( info->fields[n]->name.toUpper() )
                               .arg( options[m].toUpper().replace(QRegExp(ENUM_SPECIAL_CHARS), "") )
                               .arg(m) );

            }
            enums.append( QString(" } %1Options;\n")
                          .arg( info->fields[n]->name ) );
        }
        // Generate element names (only if field has more than one element)
        if (info->fields[n]->numElements > 1 && !info->fields[n]->defaultElementNames) {
            enums.append(QString("    /* Array element names for field %1 */\n").arg(info->fields[n]->name));
            enums.append("    typedef enum { ");
            // Go through the element names
            QStringList elemNames = info->fields[n]->elementNames;
            for (int m = 0; m < elemNames.length(); ++m) {
                QString s = (m != (elemNames.length()-1)) ? "%1_%2=%3, " : "%1_%2=%3";
                enums.append( s.arg( info->fields[n]->name.toUpper() )
                               .arg( elemNames[m].toUpper() )
                               .arg(m) );

            }
            enums.append( QString(" } %1Elem;\n")
                          .arg( info->fields[n]->name ) );
        }
        // Generate array information
        if (info->fields[n]->numElements > 1) {
            enums.append(QString("    /* Number of elements for field %1 */\n").arg(info->fields[n]->name));
            enums.append( QString("    static const quint32 %1_NUMELEM = %2;\n")
                          .arg( info->fields[n]->name.toUpper() )
                          .arg( info->fields[n]->numElements ) );
        }
    }
    outInclude.replace(QString("$(DATAFIELDINFO)"), enums);

    // Replace the $(INITFIELDS) tag
    QString initfields;
    for (int n = 0; n < info->fields.length(); ++n)
    {
        if (!info->fields[n]->defaultValues.isEmpty() )
        {
            // For non-array fields
            if ( info->fields[n]->numElements == 1)
            {
                if ( info->fields[n]->type == FIELDTYPE_ENUM )
                {
                    initfields.append( QString("\t\tgetField(\"%1\").setValue(\"%2\");\n")
                                .arg( info->fields[n]->name )
                                .arg( info->fields[n]->defaultValues[0] ) );
                }
                else if ( info->fields[n]->type == FIELDTYPE_FLOAT32 )
                {
                    initfields.append( QString("\t\tgetField(\"%1\").setValue(%2);\n")
                                .arg( info->fields[n]->name )
                                .arg( info->fields[n]->defaultValues[0].toFloat() ) );
                }
                else
                {
                    initfields.append( QString("\t\tgetField(\"%1\").setValue(%2);\n")
                                .arg( info->fields[n]->name )
                                .arg( info->fields[n]->defaultValues[0].toInt() ) );
                }
            }
            else
            {
                // Initialize all fields in the array
                for (int idx = 0; idx < info->fields[n]->numElements; ++idx)
                {
                    if ( info->fields[n]->type == FIELDTYPE_ENUM ) {
                        initfields.append( QString("\t\tgetField(\"%1\").setValue(\"%3\",%2);\n")
                                    .arg( info->fields[n]->name )
                                    .arg( idx )
                                    .arg( info->fields[n]->defaultValues[idx] ) );
                    }
                    else if ( info->fields[n]->type == FIELDTYPE_FLOAT32 ) {
                        initfields.append( QString("\t\tgetField(\"%1\").setValue(%3,%2);\n")
                                    .arg( info->fields[n]->name )
                                    .arg( idx )
                                    .arg( info->fields[n]->defaultValues[idx].toFloat() ) );
                    }
                    else {
                        initfields.append( QString("\t\tgetField(\"%1\").setValue(%3,%2);\n")
                                    .arg( info->fields[n]->name )
                                    .arg( idx )
                                    .arg( info->fields[n]->defaultValues[idx].toInt() ) );
                    }
                }
            }
        }
    }

    outCode.replace(QString("$(INITFIELDS)"), initfields);

    // Write the java code
    bool res = writeFileIfDiffrent( javaOutputPath.absolutePath() + "/" + info->name + ".java", outCode );
    if (!res) {
        cout << "Error: Could not write gcs output files" << endl;
        return false;
    }

    return true;
}

