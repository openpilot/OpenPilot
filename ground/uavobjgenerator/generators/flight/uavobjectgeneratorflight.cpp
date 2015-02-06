/**
 ******************************************************************************
 *
 * @file       uavobjectgeneratorflight.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      produce flight code for uavobjects
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

#include "uavobjectgeneratorflight.h"

using namespace std;

bool UAVObjectGeneratorFlight::generate(UAVObjectParser *parser, QString templatepath, QString outputpath)
{
    fieldTypeStrC << "int8_t" << "int16_t" << "int32_t" << "uint8_t"
                  << "uint16_t" << "uint32_t" << "float" << "uint8_t";

    QString flightObjInit, objInc, objFileNames, objNames;
    qint32 sizeCalc;
    flightCodePath            = QDir(templatepath + QString(FLIGHT_CODE_DIR));
    flightOutputPath          = QDir(outputpath + QString("flight"));
    flightOutputPath.mkpath(flightOutputPath.absolutePath());

    flightCodeTemplate        = readFile(flightCodePath.absoluteFilePath("uavobject.c.template"));
    flightIncludeTemplate     = readFile(flightCodePath.absoluteFilePath("inc/uavobject.h.template"));
    flightInitTemplate        = readFile(flightCodePath.absoluteFilePath("uavobjectsinit.c.template"));
    flightInitIncludeTemplate = readFile(flightCodePath.absoluteFilePath("inc/uavobjectsinit.h.template"));
    flightMakeTemplate        = readFile(flightCodePath.absoluteFilePath("Makefile.inc.template"));

    if (flightCodeTemplate.isNull() || flightIncludeTemplate.isNull() || flightInitTemplate.isNull()) {
        cerr << "Error: Could not open flight template files." << endl;
        return false;
    }

    sizeCalc = 0;
    for (int objidx = 0; objidx < parser->getNumObjects(); ++objidx) {
        ObjectInfo *info = parser->getObjectByIndex(objidx);
        process_object(info);
        flightObjInit.append("#ifdef UAVOBJ_INIT_" + info->namelc + "\n");
        flightObjInit.append("    " + info->name + "Initialize();\n");
        flightObjInit.append("#endif\n");
        objInc.append("#include \"" + info->namelc + ".h\"\n");
        objFileNames.append(" " + info->namelc);
        objNames.append(" " + info->name);
        if (parser->getNumBytes(objidx) > sizeCalc) {
            sizeCalc = parser->getNumBytes(objidx);
        }
    }

    // Write the flight object inialization files
    flightInitTemplate.replace(QString("$(OBJINC)"), objInc);
    flightInitTemplate.replace(QString("$(OBJINIT)"), flightObjInit);
    bool res = writeFileIfDiffrent(flightOutputPath.absolutePath() + "/uavobjectsinit.c",
                                   flightInitTemplate);
    if (!res) {
        cout << "Error: Could not write flight object init file" << endl;
        return false;
    }

    // Write the flight object initialization header
    flightInitIncludeTemplate.replace(QString("$(SIZECALCULATION)"), QString().setNum(sizeCalc));
    res = writeFileIfDiffrent(flightOutputPath.absolutePath() + "/uavobjectsinit.h",
                              flightInitIncludeTemplate);
    if (!res) {
        cout << "Error: Could not write flight object init header file" << endl;
        return false;
    }

    // Write the flight object Makefile
    flightMakeTemplate.replace(QString("$(UAVOBJFILENAMES)"), objFileNames);
    flightMakeTemplate.replace(QString("$(UAVOBJNAMES)"), objNames);
    res = writeFileIfDiffrent(flightOutputPath.absolutePath() + "/Makefile.inc",
                              flightMakeTemplate);
    if (!res) {
        cout << "Error: Could not write flight Makefile" << endl;
        return false;
    }

    return true; // if we come here everything should be fine
}


/**
 * Generate the Flight object files
 **/
bool UAVObjectGeneratorFlight::process_object(ObjectInfo *info)
{
    if (info == NULL) {
        return false;
    }

    // Prepare output strings
    QString outInclude = flightIncludeTemplate;
    QString outCode    = flightCodeTemplate;

    // Replace common tags
    replaceCommonTags(outInclude, info);
    replaceCommonTags(outCode, info);

    // Replace the $(DATAFIELDS) tag
    QString type;
    QString fields;
    QString dataStructures;
    for (int n = 0; n < info->fields.length(); ++n) {
        // Determine type
        type = fieldTypeStrC[info->fields[n]->type];
        // Append field
        // Check if it a named set and creates structures accordingly
        if (info->fields[n]->numElements > 1) {
            if (info->fields[n]->elementNames[0].compare(QString("0")) != 0) {
                QString structTypeName = QString("%1%2Data").arg(info->name).arg(info->fields[n]->name);
                QString structType     = QString("typedef struct __attribute__ ((__packed__)) {\n");
                for (int f = 0; f < info->fields[n]->elementNames.count(); f++) {
                    structType.append(QString("    %1 %2;\n").arg(type).arg(info->fields[n]->elementNames[f]));
                }
                structType.append(QString("}  %1 ;\n").arg(structTypeName));
                structType.append(QString("typedef struct __attribute__ ((__packed__)) {\n"));
                structType.append(QString("    %1 array[%2];\n").arg(type).arg(info->fields[n]->elementNames.count()));
                structType.append(QString("}  %1Array ;\n").arg(structTypeName));
                structType.append(QString("#define %1%2ToArray( var ) UAVObjectFieldToArray( %3, var )\n\n").arg(info->name).arg(info->fields[n]->name).arg(structTypeName));

                dataStructures.append(structType);

                fields.append(QString("    %1 %2;\n").arg(structTypeName)
                              .arg(info->fields[n]->name));
            } else {
                fields.append(QString("    %1 %2[%3];\n").arg(type)
                              .arg(info->fields[n]->name).arg(info->fields[n]->numElements));
            }
        } else {
            fields.append(QString("    %1 %2;\n").arg(type).arg(info->fields[n]->name));
        }
    }
    outInclude.replace(QString("$(DATAFIELDS)"), fields);
    outInclude.replace(QString("$(DATASTRUCTURES)"), dataStructures);
    // Replace the $(DATAFIELDINFO) tag
    QString enums;
    for (int n = 0; n < info->fields.length(); ++n) {
        enums.append(QString("/* Field %1 information */\n").arg(info->fields[n]->name));
        // Only for enum types
        if (info->fields[n]->type == FIELDTYPE_ENUM) {
            enums.append(QString("\n// Enumeration options for field %1\n").arg(info->fields[n]->name));
            enums.append("typedef enum {\n");
            // Go through each option
            QStringList options = info->fields[n]->options;
            for (int m = 0; m < options.length(); ++m) {
                QString s = (m == (options.length() - 1)) ? "    %1_%2_%3=%4\n" : "    %1_%2_%3=%4,\n";
                enums.append(s
                             .arg(info->name.toUpper())
                             .arg(info->fields[n]->name.toUpper())
                             .arg(options[m].toUpper().replace(QRegExp(ENUM_SPECIAL_CHARS), ""))
                             .arg(m));
            }
            enums.append(QString("} %1%2Options;\n")
                         .arg(info->name)
                         .arg(info->fields[n]->name));
        }
        // Generate element names (only if field has more than one element)
        if (info->fields[n]->numElements > 1 && !info->fields[n]->defaultElementNames) {
            enums.append(QString("\n// Array element names for field %1\n").arg(info->fields[n]->name));
            enums.append("typedef enum {\n");
            // Go through the element names
            QStringList elemNames = info->fields[n]->elementNames;
            for (int m = 0; m < elemNames.length(); ++m) {
                QString s = (m != (elemNames.length() - 1)) ? "    %1_%2_%3=%4,\n" : "    %1_%2_%3=%4\n";
                enums.append(s
                             .arg(info->name.toUpper())
                             .arg(info->fields[n]->name.toUpper())
                             .arg(elemNames[m].toUpper())
                             .arg(m));
            }
            enums.append(QString("} %1%2Elem;\n")
                         .arg(info->name)
                         .arg(info->fields[n]->name));
        }
        // Generate array information
        if (info->fields[n]->numElements > 1) {
            enums.append(QString("\n// Number of elements for field %1\n").arg(info->fields[n]->name));
            enums.append(QString("#define %1_%2_NUMELEM %3\n")
                         .arg(info->name.toUpper())
                         .arg(info->fields[n]->name.toUpper())
                         .arg(info->fields[n]->numElements));
        }

        enums.append(QString("\n"));
    }
    outInclude.replace(QString("$(DATAFIELDINFO)"), enums);

    // Replace the $(INITFIELDS) tag
    QString initfields;
    for (int n = 0; n < info->fields.length(); ++n) {
        if (!info->fields[n]->defaultValues.isEmpty()) {
            // For non-array fields
            if (info->fields[n]->numElements == 1) {
                if (info->fields[n]->type == FIELDTYPE_ENUM) {
                    initfields.append(QString("    data.%1 = %2;\n")
                                      .arg(info->fields[n]->name)
                                      .arg(info->fields[n]->options.indexOf(info->fields[n]->defaultValues[0])));
                } else if (info->fields[n]->type == FIELDTYPE_FLOAT32) {
                    initfields.append(QString("    data.%1 = %2f;\n")
                                      .arg(info->fields[n]->name)
                                      .arg(info->fields[n]->defaultValues[0].toFloat(), 0, 'e', 6));
                } else {
                    initfields.append(QString("    data.%1 = %2;\n")
                                      .arg(info->fields[n]->name)
                                      .arg(info->fields[n]->defaultValues[0].toInt()));
                }
            } else {
                // Initialize all fields in the array
                for (int idx = 0; idx < info->fields[n]->numElements; ++idx) {
                    if (info->fields[n]->elementNames[0].compare(QString("0")) == 0) {
                        initfields.append(QString("    data.%1[%2] = ")
                                          .arg(info->fields[n]->name)
                                          .arg(idx));
                    } else {
                        initfields.append(QString("    data.%1.%2 = ")
                                          .arg(info->fields[n]->name)
                                          .arg(info->fields[n]->elementNames[idx]));
                    }


                    if (info->fields[n]->type == FIELDTYPE_ENUM) {
                        initfields.append(QString("%1;\n")
                                          .arg(info->fields[n]->options.indexOf(info->fields[n]->defaultValues[idx])));
                    } else if (info->fields[n]->type == FIELDTYPE_FLOAT32) {
                        initfields.append(QString("%1f;\n")
                                          .arg(info->fields[n]->defaultValues[idx].toFloat(), 0, 'e', 6));
                    } else {
                        initfields.append(QString("%1;\n")
                                          .arg(info->fields[n]->defaultValues[idx].toInt()));
                    }
                }
            }
        }
    }
    outCode.replace(QString("$(INITFIELDS)"), initfields);

    // Replace the $(SETGETFIELDS) tag
    QString setgetfields;
    for (int n = 0; n < info->fields.length(); ++n) {
        // if (!info->fields[n]->defaultValues.isEmpty() )
        {
            // For non-array fields
            if (info->fields[n]->numElements == 1) {
                /* Set */
                setgetfields.append(QString("void %2%3Set(%1 *New%3)\n")
                                    .arg(fieldTypeStrC[info->fields[n]->type])
                                    .arg(info->name)
                                    .arg(info->fields[n]->name));
                setgetfields.append(QString("{\n"));
                setgetfields.append(QString("    UAVObjSetDataField(%1Handle(), (void *)New%2, offsetof(%1Data, %2), sizeof(%3));\n")
                                    .arg(info->name)
                                    .arg(info->fields[n]->name)
                                    .arg(fieldTypeStrC[info->fields[n]->type]));
                setgetfields.append(QString("}\n"));

                /* GET */
                setgetfields.append(QString("void %2%3Get(%1 *New%3)\n")
                                    .arg(fieldTypeStrC[info->fields[n]->type])
                                    .arg(info->name)
                                    .arg(info->fields[n]->name));
                setgetfields.append(QString("{\n"));
                setgetfields.append(QString("    UAVObjGetDataField(%1Handle(), (void *)New%2, offsetof(%1Data, %2), sizeof(%3));\n")
                                    .arg(info->name)
                                    .arg(info->fields[n]->name)
                                    .arg(fieldTypeStrC[info->fields[n]->type]));
                setgetfields.append(QString("}\n"));
            } else {
                // When no struct accessor is available for a field array accessor is the default.
                QString suffix = QString("");

                if (info->fields[n]->elementNames[0].compare(QString("0")) != 0) {
                    // struct based field accessor
                    QString structTypeName = QString("%1%2Data").arg(info->name).arg(info->fields[n]->name);
                    /* SET */
                    setgetfields.append(QString("void %2%3Set( %1 *New%3 )\n")
                                        .arg(structTypeName)
                                        .arg(info->name)
                                        .arg(info->fields[n]->name));
                    setgetfields.append(QString("{\n"));
                    setgetfields.append(QString("    UAVObjSetDataField(%1Handle(), (void *)New%2, offsetof(%1Data, %2), %3*sizeof(%4));\n")
                                        .arg(info->name)
                                        .arg(info->fields[n]->name)
                                        .arg(info->fields[n]->numElements)
                                        .arg(fieldTypeStrC[info->fields[n]->type]));
                    setgetfields.append(QString("}\n"));

                    /* GET */
                    setgetfields.append(QString("void %2%3Get( %1 *New%3 )\n")
                                        .arg(structTypeName)
                                        .arg(info->name)
                                        .arg(info->fields[n]->name));
                    setgetfields.append(QString("{\n"));
                    setgetfields.append(QString("    UAVObjGetDataField(%1Handle(), (void *)New%2, offsetof(%1Data, %2), %3*sizeof(%4));\n")
                                        .arg(info->name)
                                        .arg(info->fields[n]->name)
                                        .arg(info->fields[n]->numElements)
                                        .arg(fieldTypeStrC[info->fields[n]->type]));
                    setgetfields.append(QString("}\n"));

                    // Append array suffix to array accessors
                    suffix = QString("Array");
                }

                // array based field accessor
                /* SET */
                setgetfields.append(QString("void %2%3%4Set( %1 *New%3 )\n")
                                    .arg(fieldTypeStrC[info->fields[n]->type])
                                    .arg(info->name)
                                    .arg(info->fields[n]->name)
                                    .arg(suffix));
                setgetfields.append(QString("{\n"));
                setgetfields.append(QString("    UAVObjSetDataField(%1Handle(), (void *)New%2, offsetof(%1Data, %2), %3*sizeof(%4));\n")
                                    .arg(info->name)
                                    .arg(info->fields[n]->name)
                                    .arg(info->fields[n]->numElements)
                                    .arg(fieldTypeStrC[info->fields[n]->type]));
                setgetfields.append(QString("}\n"));

                /* GET */
                setgetfields.append(QString("void %2%3%4Get( %1 *New%3 )\n")
                                    .arg(fieldTypeStrC[info->fields[n]->type])
                                    .arg(info->name)
                                    .arg(info->fields[n]->name)
                                    .arg(suffix));
                setgetfields.append(QString("{\n"));
                setgetfields.append(QString("    UAVObjGetDataField(%1Handle(), (void *)New%2, offsetof(%1Data, %2), %3*sizeof(%4));\n")
                                    .arg(info->name)
                                    .arg(info->fields[n]->name)
                                    .arg(info->fields[n]->numElements)
                                    .arg(fieldTypeStrC[info->fields[n]->type]));
                setgetfields.append(QString("}\n"));
            }
        }
    }
    outCode.replace(QString("$(SETGETFIELDS)"), setgetfields);

    // Replace the $(SETGETFIELDSEXTERN) tag
    QString setgetfieldsextern;
    for (int n = 0; n < info->fields.length(); ++n) {
        // if (!info->fields[n]->defaultValues.isEmpty() )
        {
            QString suffix = QString("");
            if (info->fields[n]->elementNames[0].compare(QString("0")) != 0) {
                // struct based field accessor
                QString structTypeName = QString("%1%2Data").arg(info->name).arg(info->fields[n]->name);

                /* SET */
                setgetfieldsextern.append(QString("extern void %2%3Set(%1 *New%3);\n")
                                          .arg(structTypeName)
                                          .arg(info->name)
                                          .arg(info->fields[n]->name));

                /* GET */
                setgetfieldsextern.append(QString("extern void %2%3Get(%1 *New%3);\n")
                                          .arg(structTypeName)
                                          .arg(info->name)
                                          .arg(info->fields[n]->name));
                suffix = QString("Array");
            }
            /* SET */
            setgetfieldsextern.append(QString("extern void %2%3%4Set(%1 *New%3);\n")
                                      .arg(fieldTypeStrC[info->fields[n]->type])
                                      .arg(info->name)
                                      .arg(info->fields[n]->name)
                                      .arg(suffix));

            /* GET */
            setgetfieldsextern.append(QString("extern void %2%3%4Get(%1 *New%3);\n")
                                      .arg(fieldTypeStrC[info->fields[n]->type])
                                      .arg(info->name)
                                      .arg(info->fields[n]->name)
                                      .arg(suffix));
        }
    }
    outInclude.replace(QString("$(SETGETFIELDSEXTERN)"), setgetfieldsextern);

    // Write the flight code
    bool res = writeFileIfDiffrent(flightOutputPath.absolutePath() + "/" + info->namelc + ".c", outCode);
    if (!res) {
        cout << "Error: Could not write flight code files" << endl;
        return false;
    }

    res = writeFileIfDiffrent(flightOutputPath.absolutePath() + "/" + info->namelc + ".h", outInclude);
    if (!res) {
        cout << "Error: Could not write flight include files" << endl;
        return false;
    }

    return true;
}
