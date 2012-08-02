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

#include "uavobjectgeneratorwireshark.h"

using namespace std;

bool UAVObjectGeneratorWireshark::generate(UAVObjectParser* parser,QString templatepath,QString outputpath) {

    fieldTypeStrHf << "FT_INT8" << "FT_INT16" << "FT_INT32" <<"FT_UINT8"
            <<"FT_UINT16" << "FT_UINT32" << "FT_FLOAT" << "FT_UINT8";
    fieldTypeStrGlib << "gint8" << "gint16" << "gint32" <<"guint8"
            <<"guint16" << "guint32" << "gfloat" << "guint8";

    wiresharkCodePath = QDir( templatepath + QString("ground/openpilotgcs/src/plugins/uavobjects/wireshark"));

    wiresharkOutputPath = QDir( outputpath + QString("wireshark") );
    wiresharkOutputPath.mkpath(wiresharkOutputPath.absolutePath());

    wiresharkCodeTemplate = readFile( wiresharkCodePath.absoluteFilePath("op-uavobjects/packet-op-uavobjects-template.c") );
    wiresharkMakeTemplate = readFile( wiresharkCodePath.absoluteFilePath("op-uavobjects/Makefile.common-template") );

    if ( wiresharkCodeTemplate.isNull() || wiresharkMakeTemplate.isNull()) {
      cerr << "Error: Could not open wireshark template files." << endl;
      return false;
    }

    /* Copy static files for wireshark plugins root directory into output directory */
    QStringList topstaticfiles;
    topstaticfiles << "Custom.m4" << "Custom.make" << "Custom.nmake";
    for (int i = 0; i < topstaticfiles.length(); ++i) {
      QFile::copy(wiresharkCodePath.absoluteFilePath(topstaticfiles[i]),
		  wiresharkOutputPath.absoluteFilePath(topstaticfiles[i]));
    }

    /* Copy static files for op-uavtalk dissector into output directory */
    QDir uavtalkOutputPath = QDir( outputpath + QString("wireshark/op-uavtalk") );
    uavtalkOutputPath.mkpath(uavtalkOutputPath.absolutePath());
    QStringList uavtalkstaticfiles;
    uavtalkstaticfiles << "AUTHORS" << "COPYING" << "ChangeLog";
    uavtalkstaticfiles << "CMakeLists.txt" << "Makefile.nmake";
    uavtalkstaticfiles << "Makefile.am" << "moduleinfo.h" << "moduleinfo.nmake";
    uavtalkstaticfiles << "plugin.rc.in";
    uavtalkstaticfiles << "Makefile.common" << "packet-op-uavtalk.c";
    for (int i = 0; i < uavtalkstaticfiles.length(); ++i) {
      QFile::copy(wiresharkCodePath.absoluteFilePath("op-uavtalk/" + uavtalkstaticfiles[i]),
		  uavtalkOutputPath.absoluteFilePath(uavtalkstaticfiles[i]));
    }

    /* Copy static files for op-uavobjects dissector into output directory */
    QDir uavobjectsOutputPath = QDir( outputpath + QString("wireshark/op-uavobjects") );
    uavobjectsOutputPath.mkpath(uavobjectsOutputPath.absolutePath());
    QStringList uavostaticfiles;
    uavostaticfiles << "AUTHORS" << "COPYING" << "ChangeLog";
    uavostaticfiles << "CMakeLists.txt" << "Makefile.nmake";
    uavostaticfiles << "Makefile.am" << "moduleinfo.h" << "moduleinfo.nmake";
    uavostaticfiles << "plugin.rc.in";
    for (int i = 0; i < uavostaticfiles.length(); ++i) {
      QFile::copy(wiresharkCodePath.absoluteFilePath("op-uavobjects/" + uavostaticfiles[i]),
		  uavobjectsOutputPath.absoluteFilePath(uavostaticfiles[i]));
    }

    /* Generate the per-object files from the templates, and keep track of the list of generated filenames */
    QString objFileNames;
    for (int objidx = 0; objidx < parser->getNumObjects(); ++objidx) {
      ObjectInfo* info = parser->getObjectByIndex(objidx);
      process_object(info, uavobjectsOutputPath);
      objFileNames.append(" packet-op-uavobjects-" + info->namelc + ".c");
    }

    /* Write the uavobject dissector's Makefile.common */
    wiresharkMakeTemplate.replace( QString("$(UAVOBJFILENAMES)"), objFileNames);
    bool res = writeFileIfDiffrent( uavobjectsOutputPath.absolutePath() + "/Makefile.common",
                     wiresharkMakeTemplate );
    if (!res) {
      cout << "Error: Could not write wireshark Makefile" << endl;
      return false;
    }

    return true;
}


/**
 * Generate the Flight object files
**/
bool UAVObjectGeneratorWireshark::process_object(ObjectInfo* info, QDir outputpath)
{
    if (info == NULL)
        return false;

    // Prepare output strings
    QString outCode = wiresharkCodeTemplate;

    // Replace common tags
    replaceCommonTags(outCode, info);

    // Replace the $(SUBTREES) and $(SUBTREESTATICS) tags
    QString subtrees;
    QString subtreestatics;
    subtreestatics.append( QString("static gint ett_uavo = -1;\r\n") );
    subtrees.append( QString("&ett_uavo,\r\n") );
    for (int n = 0; n < info->fields.length(); ++n) {
      if (info->fields[n]->numElements > 1) {
	/* Reserve a subtree for each array */
	subtreestatics.append( QString("static gint ett_%1_%2 = -1;\r\n")
			       .arg(info->namelc)
			       .arg(info->fields[n]->name) );
	subtrees.append( QString("&ett_%1_%2,\r\n")
			 .arg(info->namelc)
			 .arg(info->fields[n]->name) );
      }
    }
    outCode.replace(QString("$(SUBTREES)"), subtrees);
    outCode.replace(QString("$(SUBTREESTATICS)"), subtreestatics);

    // Replace the $(FIELDHANDLES) tag
    QString type;
    QString fields;
    for (int n = 0; n < info->fields.length(); ++n) {
      fields.append( QString("static int hf_op_uavobjects_%1_%2 = -1;\r\n")
		     .arg(info->namelc)
		     .arg(info->fields[n]->name));
      if (info->fields[n]->numElements > 1) {
	QStringList elemNames = info->fields[n]->elementNames;
	for (int m = 0; m < elemNames.length(); ++m) {
	  fields.append( QString("static int hf_op_uavobjects_%1_%2_%3 = -1;\r\n")
			 .arg(info->namelc)
			 .arg(info->fields[n]->name)
			 .arg(elemNames[m]) );
	}
      }
    }
    outCode.replace(QString("$(FIELDHANDLES)"), fields);

    // Replace the $(ENUMFIELDNAMES) tag
    QString enums;
    for (int n = 0; n < info->fields.length(); ++n) {
      // Only for enum types
      if (info->fields[n]->type == FIELDTYPE_ENUM) {
	enums.append(QString("/* Field %1 information */\r\n").arg(info->fields[n]->name) );
	enums.append(QString("/* Enumeration options for field %1 */\r\n").arg(info->fields[n]->name));
	enums.append( QString("static const value_string uavobjects_%1_%2[]= {\r\n")
		      .arg(info->namelc)
		      .arg(info->fields[n]->name) );
	// Go through each option
	QStringList options = info->fields[n]->options;
	for (int m = 0; m < options.length(); ++m) {
	  enums.append ( QString("\t{ %1, \"%2\" },\r\n")
			 .arg(m)
			 .arg(options[m].replace(QRegExp(ENUM_SPECIAL_CHARS), "") ) );
	}
	enums.append( QString("\t{ 0, NULL }\r\n") );
	enums.append( QString("};\r\n") );
      }
    }
    outCode.replace(QString("$(ENUMFIELDNAMES)"), enums);

    // Replace the $(POPULATETREE) tag
    QString treefields;
    for (int n = 0; n < info->fields.length(); ++n) {
      if ( info->fields[n]->numElements == 1 ) {
	treefields.append( QString("    ptvcursor_add(cursor, hf_op_uavobjects_%1_%2, sizeof(%3), ENC_LITTLE_ENDIAN);\r\n")
			   .arg(info->namelc)
			   .arg(info->fields[n]->name)
			   .arg(fieldTypeStrGlib[info->fields[n]->type]) );
      } else {
	treefields.append( QString("    {\r\n") );
	treefields.append( QString("      proto_item * it = NULL;\r\n") );
	treefields.append( QString("      it = ptvcursor_add_no_advance(cursor, hf_op_uavobjects_%1_%2, 1, ENC_NA);\r\n")
			   .arg(info->namelc)
			   .arg(info->fields[n]->name) );
	treefields.append( QString("      ptvcursor_push_subtree(cursor, it, ett_%1_%2);\r\n")
			   .arg(info->namelc)
			   .arg(info->fields[n]->name) );
	/* Populate each array element into the table */
	QStringList elemNames = info->fields[n]->elementNames;
	for (int m = 0; m < elemNames.length(); ++m) {
	  treefields.append( QString("    ptvcursor_add(cursor, hf_op_uavobjects_%1_%2_%3, sizeof(%4), ENC_LITTLE_ENDIAN);\r\n")
			     .arg(info->namelc)
			     .arg(info->fields[n]->name)
			     .arg(elemNames[m])
			     .arg(fieldTypeStrGlib[info->fields[n]->type]) );
	}
	treefields.append( QString("      ptvcursor_pop_subtree(cursor);\r\n") );
	treefields.append( QString("    }\r\n") );
      }
    }
    outCode.replace(QString("$(POPULATETREE)"), treefields);

    // Replace the $(HEADERFIELDS) tag
    QString headerfields;
    headerfields.append( QString("   static hf_register_info hf[] = {\r\n") );
    for (int n = 0; n < info->fields.length(); ++n) {
      // For non-array fields
      if ( info->fields[n]->numElements == 1) {
	  headerfields.append( QString("\t { &hf_op_uavobjects_%1_%2,\r\n")
			       .arg( info->namelc )
			       .arg( info->fields[n]->name ) );
	  headerfields.append( QString("\t   { \"%1\", \"%2.%3\", %4,\r\n")
			       .arg( info->fields[n]->name )
			       .arg( info->namelc )
			       .arg( info->fields[n]->name )
			       .arg( fieldTypeStrHf[info->fields[n]->type] ) );
	  if ( info->fields[n]->type == FIELDTYPE_ENUM ) {
	    headerfields.append( QString("\t     BASE_DEC, VALS(uavobjects_%1_%2), 0x0, NULL, HFILL \r\n")
				 .arg( info->namelc )
				 .arg( info->fields[n]->name ) );
	  } else if ( info->fields[n]->type == FIELDTYPE_FLOAT32 ) {
	    headerfields.append( QString("\t     BASE_NONE, NULL, 0x0, NULL, HFILL \r\n") );
	  } else {
	    headerfields.append( QString("\t     BASE_DEC_HEX, NULL, 0x0, NULL, HFILL\r\n") );
	  }
	  headerfields.append( QString("\t   },\r\n") );
	  headerfields.append( QString("\t },\r\n") );
      } else {
	headerfields.append( QString("\t { &hf_op_uavobjects_%1_%2,\r\n")
			     .arg( info->namelc )
			     .arg( info->fields[n]->name ) );
	headerfields.append( QString("\t   { \"%1\", \"%2.%3\", FT_NONE,\r\n")
			     .arg( info->fields[n]->name )
			     .arg( info->namelc )
			     .arg( info->fields[n]->name ) );
	headerfields.append( QString("\t     BASE_NONE, NULL, 0x0, NULL, HFILL\r\n") );
	headerfields.append( QString("\t   },\r\n") );
	headerfields.append( QString("\t },\r\n") );

	QStringList elemNames = info->fields[n]->elementNames;
	for (int m = 0; m < elemNames.length(); ++m) {
	  headerfields.append( QString("\t { &hf_op_uavobjects_%1_%2_%3,\r\n")
			       .arg( info->namelc )
			       .arg( info->fields[n]->name )
			       .arg( elemNames[m]) );
	  headerfields.append( QString("\t   { \"%1\", \"%2.%3.%4\", %5,\r\n")
			       .arg( elemNames[m] )
			       .arg( info->namelc )
			       .arg( info->fields[n]->name )
			       .arg( elemNames[m] )
			       .arg( fieldTypeStrHf[info->fields[n]->type] ) );
	  if ( info->fields[n]->type == FIELDTYPE_ENUM ) {
	    headerfields.append( QString("\t     BASE_DEC, VALS(uavobjects_%1_%2), 0x0, NULL, HFILL \r\n")
				 .arg( info->namelc )
				 .arg( info->fields[n]->name ) );
	  } else if ( info->fields[n]->type == FIELDTYPE_FLOAT32 ) {
	    headerfields.append( QString("\t     BASE_NONE, NULL, 0x0, NULL, HFILL \r\n") );
	  } else {
	    headerfields.append( QString("\t     BASE_DEC_HEX, NULL, 0x0, NULL, HFILL\r\n") );
	  }
	  headerfields.append( QString("\t   },\r\n") );
	  headerfields.append( QString("\t },\r\n") );
	}
      }
    }
    headerfields.append( QString("   };\r\n") );
    outCode.replace(QString("$(HEADERFIELDS)"), headerfields);

    // Write the flight code
    bool res = writeFileIfDiffrent( outputpath.absolutePath() + "/packet-op-uavobjects-" + info->namelc + ".c", outCode );
    if (!res) {
        cout << "Error: Could not write wireshark code files" << endl;
        return false;
    }

    return true;
}


