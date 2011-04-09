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

#include "uavobjectgeneratorjava.h"
using namespace std;

bool UAVObjectGeneratorJava::generate(UAVObjectParser* parser,QString templatepath,QString outputpath) {
    QDir javaTemplatePath = QDir( templatepath + QString(JAVA_TEMPLATE_DIR));

    javaCodePath = QDir ( outputpath + QString(JAVA_GENERATED_DIR));

    javaCodeTemplate = readFile( javaTemplatePath.absoluteFilePath("uavobjecttemplate.java") );
    QString javaInitCodeTemplate = readFile( javaTemplatePath.absoluteFilePath("uavobjectsinittemplate.java") );

    if (javaCodeTemplate.isEmpty() || javaInitCodeTemplate.isEmpty()) {
        std::cerr << "Problem reading java templates" << endl;
        return false;
    }

    QString javaObjInit,javaObjGetter;
    javaCodePath.mkpath(javaCodePath.absolutePath() + "/uavobjects");

    for (int objidx = 0; objidx < parser->getNumObjects(); ++objidx) {
        ObjectInfo* info=parser->getObjectByIndex(objidx);
        process_object(info);

        if (!javaObjInit.isNull())
            javaObjInit.append(",\n");

        javaObjInit.append("            new " + info->name + "()");
        javaObjGetter.append(QString("    public static %1 get%1() { return (%1)uavobjects[%2];}\n").arg( info->name).arg(objidx));
    }

    javaInitCodeTemplate.replace("$(OBJINIT)",javaObjInit);
    javaInitCodeTemplate.replace("$(OBJGETTER)",javaObjGetter);

    replaceCommonTags(javaInitCodeTemplate);
    
    // Write the java object inialization files
    bool res = writeFileIfDiffrent(javaCodePath.absolutePath() + "/UAVObjects.java", javaInitCodeTemplate );
    if (!res) {
        cerr << "Error: Could not write java init output files" << endl;
        return false;
    }

    return true; // if we come here everything should be fine
}

QString UAVObjectGeneratorJava::getFieldTypeStr(int type,bool as_obj) {

    switch(type) {
    case FIELDTYPE_INT8:
        return (QString(as_obj?"Byte":"byte"));
    case FIELDTYPE_INT16:
    case FIELDTYPE_INT32:
    case FIELDTYPE_UINT8:
    case FIELDTYPE_UINT16:
        return (QString(as_obj?"Integer":"int"));
    case FIELDTYPE_UINT32:
        return (QString(as_obj?"Long":"long"));
    case FIELDTYPE_FLOAT32:
        return (QString(as_obj?"Float":"float"));
    case FIELDTYPE_ENUM:
        return (QString(as_obj?"Byte":"byte"));
    default:
        return QString("error: unknown type");
    }
}

/**
 *
 * format a value from FieldInfo with a given index (idx) to a java code snippet
 *
 */
QString UAVObjectGeneratorJava::formatJavaValue(FieldInfo* info,int idx) {
    switch(info->type) {
    case  FIELDTYPE_ENUM:
      return (info->name.toUpper() + QString("_") + info->defaultValues[idx].toUpper().replace(QRegExp(ENUM_SPECIAL_CHARS), ""));
    case FIELDTYPE_FLOAT32:
      return QString("%1f").arg( info->defaultValues[idx].toFloat() );
    default:
      return QString("%1").arg( info->defaultValues[idx].toInt() );
    }
}

QString UAVObjectGeneratorJava::QStringList2JavaArray(QStringList strl) {
    QString csv=QString();
    for (int i = 0; i < strl.length(); i++){
        if (i!=0)
            csv.append(QString(","));
        csv.append(QString("\"%1\"").arg(strl[i]));
    }
    return QString("new String[] { %1 }").arg(csv);
}

QString UAVObjectGeneratorJava::serializeJavaValue(int type,QString name) 
{
    QString res; 
    switch(type) {
    case FIELDTYPE_ENUM: // OK

    case FIELDTYPE_INT8:
        res=name;
        break;

    case FIELDTYPE_UINT8:
        res=QString("(byte)(%1&0xFF)").arg(name);
        break;

    case FIELDTYPE_INT16:
    case FIELDTYPE_UINT16:
        res=QString("(byte)(("+name+">>0)&0xFF) , (byte)(("+name+">>8)&0xFF)");
        break;

    case FIELDTYPE_UINT32:
    case FIELDTYPE_INT32:
        res=QString("(byte)(("+name+">>0)&0xFF) , (byte)(("+name+">>8)&0xFF) , (byte)(("+name+">>16)&0xFF) , (byte)(("+name+">>24)&0xFF)");
        break;

    case FIELDTYPE_FLOAT32:
        res=QString("(byte)((Float.floatToIntBits("+name+")>>0)&0xFF) , (byte)((Float.floatToIntBits("+name+")>>8)&0xFF) , (byte)((Float.floatToIntBits("+name+")>>16)&0xFF) , (byte)((Float.floatToIntBits("+name+")>>24)&0xFF)  "); // OK
        break;

    default:
        res=QString("unresolved type");
        break;
    }

    return res;
}

/**
 * this function is used to build the deserializing code
 */
QString UAVObjectGeneratorJava::deSerializeJavaValue(int type,QString name)
{
    switch(type) {
        case FIELDTYPE_ENUM: // OK
        case FIELDTYPE_INT8:
            return (name.append("=arr[pos++];\n"));

        case FIELDTYPE_UINT8:
            return (name.append("=arr[pos++]&0xFF;\n")); /*test */

        case FIELDTYPE_INT16:
        case FIELDTYPE_UINT16:
            return (name.append("=((arr[pos++]&0xff)<<0) | ((arr[pos++]&0xff)<<8) ;\n")); /* test */

        case FIELDTYPE_UINT32:
        case FIELDTYPE_INT32:
            return (name.append("=((arr[pos++]&0xff)<<0) | ((arr[pos++]&0xff)<<8) | ((arr[pos++]&0xff)<<16) | ((arr[pos++]&0xff)<<24) ;\n")); /*test */

        case FIELDTYPE_FLOAT32:
	  return (name.append("=Float.intBitsToFloat(((arr[pos++]&0xff)<<0) | ((arr[pos++]&0xff)<<8) | ((arr[pos++]&0xff)<<16) | ((arr[pos++]&0xff)<<24) );  ")); // OK

        default:
            cout << "Warning: unresolved type " << type << " for " << name.toStdString() << endl;
            return(QString("unresolved type")); // will throw an arr when compiling then
      }
}

/**
 * Generate the java object files
 */
bool UAVObjectGeneratorJava::process_object(ObjectInfo* info)
{
    if (info == NULL) return false;

    // Prepare output strings
    QString outCode = javaCodeTemplate;

    // Replace common tags
    replaceCommonTags(outCode, info);

    QString type,fieldsinit,serialize_code,serialize_code_inner,deserialize_code,gettersetter;

    QString fielddesc=QString("    public final UAVObjectFieldDescription[] getFieldDescriptions() { return new UAVObjectFieldDescription[] {");
    QString fieldgetter=QString("    public Object getField(int fieldid,int arr_pos) { switch(fieldid) {\n");
    QString fieldsetter=QString("    public void setField(int fieldid,int arr_pos,Object obj) { switch(fieldid) {\n");

    for (int n = 0; n < info->fields.length(); ++n) {
        FieldInfo* act_field_info=info->fields[n];

        bool is_array = info->fields[n]->numElements > 1;
        type = getFieldTypeStr(act_field_info->type,false); // Determine type

        for (int enum_n = 0; enum_n < act_field_info->options.length(); ++enum_n)
            fieldsinit.append(QString("    public final static byte %1 = %2;\n").arg(act_field_info->name.toUpper() + QString("_") + act_field_info->options[enum_n].toUpper().replace(QRegExp(ENUM_SPECIAL_CHARS), "")).arg(enum_n) );

        fieldsinit.append(QString("    private ") + type);

        if ( is_array )
            fieldsinit.append(QString("[]"));

        fieldsinit.append(QString(" %1").arg(act_field_info->name));

        if (!info->fields[n]->defaultValues.isEmpty()) { // if we have default vals
            fieldsinit.append(QString("="));
            if ( is_array  )  {
                fieldsinit.append(QString("{"));
                for (int val_n = 0; val_n < act_field_info->defaultValues.length(); ++val_n)
                    fieldsinit.append( ((val_n>0)?QString(","):QString() ) + formatJavaValue(act_field_info,val_n)  );
                fieldsinit.append(QString("}"));
            }
            else
                fieldsinit.append(formatJavaValue(act_field_info,0));
        }
        else {
            if ( is_array  ) // when it is an array
                fieldsinit.append(QString("= new ") + type + QString("[%1]").arg(info->fields[n]->numElements));
        }

        fieldsinit.append(QString(";\n"));

        if (n!=0)
            serialize_code_inner.append(QString(","));

        if ( is_array ) {
            for (int el=0;el<info->fields[n]->numElements;el++) {
                if (el!=0)
                    serialize_code_inner.append(QString(","));

                serialize_code_inner.append(serializeJavaValue(info->fields[n]->type,info->fields[n]->name+ QString("[%1]").arg(el)));
                deserialize_code.append(deSerializeJavaValue(info->fields[n]->type,info->fields[n]->name+ QString("[%1]").arg(el)));
            }
        }
        else { // no array
            serialize_code_inner.append(serializeJavaValue(info->fields[n]->type,info->fields[n]->name));
            deserialize_code.append(deSerializeJavaValue(info->fields[n]->type,info->fields[n]->name));
        }

        serialize_code_inner.append(QString("//%1\n" ).arg(info->fields[n]->name));

        // Determine type
        type = getFieldTypeStr(act_field_info->type,false);

        QString typespec=type;

        if ( is_array ) {
            typespec.append(QString("[]"));
            fieldgetter.append(QString("case %1: return (Object)%2[arr_pos];\n").arg(n).arg(act_field_info->name));
            fieldsetter.append(QString("case %1: %2[arr_pos]=(%3)obj;break;\n").arg(n).arg(act_field_info->name).arg( getFieldTypeStr(act_field_info->type,true)));
        }
        else { // no array
            fieldgetter.append(QString("case %1: return (Object)%2;\n").arg(n).arg(act_field_info->name));
            fieldsetter.append(QString("case %1: %2=(%3)obj;break;\n").arg(n).arg(act_field_info->name).arg(getFieldTypeStr(act_field_info->type,true)));
        }


        if (!act_field_info->units.isEmpty()) // when we have unit info
            gettersetter.append(QString("\n    /**\n    * unit: %1\n    */\n").arg(act_field_info->units));

        gettersetter.append(QString("    public void set%1( %2 _%1 ) { %1=_%1; }\n").arg(act_field_info->name).arg(typespec));

        if (n!=0)
            fielddesc.append(QString("\t,"));

        fielddesc.append(QString("new UAVObjectFieldDescription(\"%1\",this.getObjID(),(byte)%2,(byte)%3,\"%4\",%5,%6)\n").arg(act_field_info->name).arg(n).arg(act_field_info->type).arg(act_field_info->units).arg(QStringList2JavaArray( act_field_info->options)).arg(QStringList2JavaArray( act_field_info->elementNames)) );

        if (!act_field_info->units.isEmpty())
            gettersetter.append(QString("\n    /**\n    * unit: %1\n    */\n").arg(act_field_info->units));

        gettersetter.append(QString("    public ") + typespec);

        gettersetter.append(QString(" get%1() { return %1; }\n").arg(act_field_info->name));

        if ( act_field_info->options.length()!=0) {
            QString enumOptionsGetter=QString("    public final static String[] get%1EnumOptions() { return new String[] {").arg(act_field_info->name);

            //	    gettersetter.append(QString(" get%1s() { return %1; }\n").arg(act_field_info->name));
            for (int enum_n = 0; enum_n < act_field_info->options.length(); ++enum_n) {
                if (enum_n!=0)
                    enumOptionsGetter.append(QString(","));
                enumOptionsGetter.append(QString("\"")+act_field_info->options[enum_n] +QString("\""));
            }

            gettersetter.append(enumOptionsGetter+QString("};}\n"));
        }

        if ( is_array ) { // when it is an array create getter for the element names
            QString elementListGetter=QString("    public final static String[] get%1ElementNames() { return new String[] {").arg(act_field_info->name);
            QStringList elemNames = info->fields[n]->elementNames;
            for (int m = 0; m < elemNames.length(); ++m) {
                gettersetter.append(QString("    public ") + type);
                gettersetter.append(QString(" get%1%2() { return %1[%3]; }\n").arg(act_field_info->name).arg(elemNames[m]).arg(m));
                if (m!=0)
                    elementListGetter.append(QString(","));

                elementListGetter.append(QString("\"") + elemNames[m]+QString("\""));
            }
            gettersetter.append(elementListGetter+QString("};}\n"));
        }
    }

    serialize_code.append(QString("    public byte[] serialize() { return new byte[]{%1 } ;};   \n ").arg(serialize_code_inner));

    serialize_code.append(QString("    public void deserialize(byte[] arr,int offset) {\nsuper.deserialize(arr,offset);\n        int pos=offset;\n%1    };\n").arg(deserialize_code));

    outCode.replace(QString("$(FIELDSINIT)"), fieldsinit);

    fielddesc.append(QString("};}"));
    fieldgetter.append(QString("};\n return null;\n}\n"));
    fieldsetter.append(QString("};\n}\n"));
    outCode.replace(QString("$(GETTERSETTER)"), gettersetter + serialize_code+ fielddesc + fieldgetter + fieldsetter);

    bool res = writeFileIfDiffrent(javaCodePath.absolutePath() + "/uavobjects/" + info->name + ".java", outCode );
    if (!res) {
        cerr << "Error: Could not write java init output file " << info->name.toStdString()<< endl;
        return false;
     }

    return true;
}

