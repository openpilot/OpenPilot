/**
 ******************************************************************************
 *
 * @file       treeitem.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   uavobjectbrowser
 * @{
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

#ifndef TREEITEM_H
#define TREEITEM_H

#include "uavobjects/uavobject.h"
#include "uavobjects/uavmetaobject.h"
#include "uavobjects/uavobjectfieldenum.h"
#include "uavobjects/uavobjectfieldint8.h"
#include "uavobjects/uavobjectfieldint16.h"
#include "uavobjects/uavobjectfieldint32.h"
#include "uavobjects/uavobjectfielduint8.h"
#include "uavobjects/uavobjectfielduint16.h"
#include "uavobjects/uavobjectfielduint32.h"
#include "uavobjects/uavobjectfieldfloat.h"
#include "uavobjects/uavobjectfieldenum.h"
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

class TreeItem
 {
 public:
     TreeItem(const QList<QVariant> &data, TreeItem *parent = 0);
     TreeItem(const QVariant &data, TreeItem *parent = 0);
     ~TreeItem();

     void appendChild(TreeItem *child);
     void insert(int index, TreeItem *child);

     TreeItem *child(int row);
     QList<TreeItem*> children() const { return m_children; }
     int childCount() const;
     int columnCount() const;
     QVariant data(int column) const;
     virtual void setData(int column, QVariant value);
     int row() const;
     TreeItem *parent() { return m_parent; }
     void setParent(TreeItem *parent) { m_parent = parent; }
     virtual bool isEditable() { return false; }

 private:
     QList<TreeItem*> m_children;
     // m_data contains: [0] property name, [1] value, and [2] unit
     QList<QVariant> m_data;
     TreeItem *m_parent;
 };

class TopTreeItem : public TreeItem
{
public:
    TopTreeItem(const QList<QVariant> &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
    TopTreeItem(const QVariant &data, TreeItem *parent = 0) : TreeItem(data, parent) { }

    QList<quint32> objIds() { return m_objIds; }
    void addObjId(quint32 objId) { m_objIds.append(objId); }
    void insertObjId(int index, quint32 objId) { m_objIds.insert(index, objId); }
    int nameIndex(QString name) {
        for (int i = 0; i < childCount(); ++i) {
            if (name < child(i)->data(0).toString())
                return i;
        }
        return childCount();
    }

private:
    QList<quint32> m_objIds;
};

class DataObjectTreeItem : public TreeItem
{
public:
    DataObjectTreeItem(const QList<QVariant> &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
    DataObjectTreeItem(const QVariant &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
};

class MetaObjectTreeItem : public TreeItem
{
public:
    MetaObjectTreeItem(const QList<QVariant> &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
    MetaObjectTreeItem(const QVariant &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
};

class InstanceTreeItem : public TreeItem
{
public:
    InstanceTreeItem(const QList<QVariant> &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
    InstanceTreeItem(const QVariant &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
};

class ArrayFieldTreeItem : public TreeItem
{
public:
    ArrayFieldTreeItem(const QList<QVariant> &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
    ArrayFieldTreeItem(const QVariant &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
};

class FieldTreeItem : public TreeItem
{
public:
    FieldTreeItem(int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            TreeItem(data, parent), m_index(index) { }
    FieldTreeItem(int index, const QVariant &data, TreeItem *parent = 0) :
            TreeItem(data, parent), m_index(index) { }
    inline int index() { return m_index; }
    bool isEditable() { return true; }
    virtual bool isIntType() { return false; }
    virtual bool isEnum() { return false; }
    virtual bool isFloatType() { return false; }
private:
    int m_index;
};

class EnumFieldTreeItem : public FieldTreeItem
{
public:
    EnumFieldTreeItem(UAVMetaObject *mobj, int index, const QList<QVariant> &data,
                      QStringList enumOptions, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), enumOptions(enumOptions), m_mobj(mobj), m_field(0) { }
    EnumFieldTreeItem(UAVObjectFieldEnum *field, int index, const QList<QVariant> &data,
                      QStringList enumOptions, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), enumOptions(enumOptions), m_field(field), m_mobj(0) { }
    EnumFieldTreeItem(UAVObjectFieldEnum *field, int index, const QVariant &data,
                      QStringList enumOptions, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), enumOptions(enumOptions), m_field(field), m_mobj(0) { }
    void setData(int column, QVariant value) {
        TreeItem::setData(column, value);
        if (m_field) {
            m_field->setSelectedIndex(value.toInt(), index());
        } else {
            UAVObject::Metadata md = m_mobj->getData();
            switch(index()) {
            case 0:
                md.flightTelemetryAcked = value.toInt();
                break;
            case 1:
                md.flightTelemetryUpdateMode = value.toInt();
                break;
            case 2:
                md.flightTelemetryUpdatePeriod = value.toInt();
                break;
            case 3:
                md.gcsTelemetryAcked = value.toInt();
                break;
            case 4:
                md.gcsTelemetryUpdateMode = value.toInt();
                break;
            case 5:
                md.gcsTelemetryUpdatePeriod = value.toInt();
                break;
            case 6:
                md.loggingUpdateMode = value.toInt();
                break;
            case 7:
                md.loggingUpdatePeriod = value.toInt();
                break;
            }
            m_mobj->setData(md);
        }
    }
    bool isEnum() { return true; }
    QStringList enumOptions;
private:
    UAVObjectFieldEnum *m_field;
    UAVMetaObject *m_mobj;
};

class IntFieldTreeItem : public FieldTreeItem
{
public:
    IntFieldTreeItem(int index, const QList<QVariant> &data, int min, int max, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), MIN(min), MAX(max) { }
    IntFieldTreeItem(int index, const QVariant &data, int min, int max, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), MIN(min), MAX(max) { }
    bool isIntType() { return true; }
    int MIN;
    int MAX;
};

class Int8FieldTreeItem : public IntFieldTreeItem
{
public:
    Int8FieldTreeItem(UAVObjectFieldInt8 *field, int index, const QList<QVariant> &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_field(field) { }
    Int8FieldTreeItem(UAVObjectFieldInt8 *field, int index, const QVariant &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_field(field) { }
    void setData(int column, QVariant value) {
        TreeItem::setData(column, value);
        m_field->setValue(value.toInt());
    }
private:
    UAVObjectFieldInt8 *m_field;
};

class Int16FieldTreeItem : public IntFieldTreeItem
{
public:
    Int16FieldTreeItem(UAVObjectFieldInt16 *field, int index, const QList<QVariant> &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_field(field) { }
    Int16FieldTreeItem(UAVObjectFieldInt16 *field, int index, const QVariant &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_field(field) { }
    void setData(int column, QVariant value) {
        TreeItem::setData(column, value);
        m_field->setValue(value.toInt());
    }
private:
    UAVObjectFieldInt16 *m_field;
};

class Int32FieldTreeItem : public IntFieldTreeItem
{
public:
    Int32FieldTreeItem(UAVMetaObject *mobj, int index, const QList<QVariant> &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_mobj(mobj), m_field(0) { }
    Int32FieldTreeItem(UAVObjectFieldInt32 *field, int index, const QList<QVariant> &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_field(field), m_mobj(0) { }
    Int32FieldTreeItem(UAVObjectFieldInt32 *field, int index, const QVariant &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_field(field), m_mobj(0) { }
    void setData(int column, QVariant value) {
        TreeItem::setData(column, value);
        if (m_field) {
            m_field->setValue(value.toInt());
        } else {
            UAVObject::Metadata md = m_mobj->getData();
            switch(index()) {
            case 0:
                md.flightTelemetryAcked = value.toInt();
                break;
            case 1:
                md.flightTelemetryUpdateMode = value.toInt();
                break;
            case 2:
                md.flightTelemetryUpdatePeriod = value.toInt();
                break;
            case 3:
                md.gcsTelemetryAcked = value.toInt();
                break;
            case 4:
                md.gcsTelemetryUpdateMode = value.toInt();
                break;
            case 5:
                md.gcsTelemetryUpdatePeriod = value.toInt();
                break;
            case 6:
                md.loggingUpdateMode = value.toInt();
                break;
            case 7:
                md.loggingUpdatePeriod = value.toInt();
                break;
            }
            m_mobj->setData(md);
        }
    }
private:
    UAVObjectFieldInt32 *m_field;
    UAVMetaObject *m_mobj;
};

class UInt8FieldTreeItem : public IntFieldTreeItem
{
public:
    UInt8FieldTreeItem(UAVObjectFieldUInt8 *field, int index, const QList<QVariant> &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_field(field) { }
    UInt8FieldTreeItem(UAVObjectFieldUInt8 *field, int index, const QVariant &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_field(field) { }
    void setData(int column, QVariant value) {
        TreeItem::setData(column, value);
        m_field->setValue(value.toInt());
    }
private:
    UAVObjectFieldUInt8 *m_field;
};

class UInt16FieldTreeItem : public IntFieldTreeItem
{
public:
    UInt16FieldTreeItem(UAVObjectFieldUInt16 *field, int index, const QList<QVariant> &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_field(field) { }
    UInt16FieldTreeItem(UAVObjectFieldUInt16 *field, int index, const QVariant &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_field(field) { }
    void setData(int column, QVariant value) {
        TreeItem::setData(column, value);
        m_field->setValue(value.toInt());
    }
private:
    UAVObjectFieldUInt16 *m_field;
};

class UInt32FieldTreeItem : public IntFieldTreeItem
{
public:
    UInt32FieldTreeItem(UAVObjectFieldUInt32 *field, int index, const QList<QVariant> &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_field(field) { }
    UInt32FieldTreeItem(UAVObjectFieldUInt32 *field, int index, const QVariant &data, int min, int max, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, min, max, parent), m_field(field) { }
    void setData(int column, QVariant value) {
        TreeItem::setData(column, value);
        m_field->setValue(value.toInt());
    }
private:
    UAVObjectFieldUInt32 *m_field;
};


class FloatFieldTreeItem : public FieldTreeItem
{
public:
    FloatFieldTreeItem(UAVObjectFieldFloat *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), m_field(field) { }
    FloatFieldTreeItem(UAVObjectFieldFloat *field, int index, const QVariant &data, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), m_field(field) { }
    bool isFloatType() { return true; }
    void setData(int column, QVariant value) {
        TreeItem::setData(column, value);
        m_field->setValue(value.toDouble());
    }
private:
    UAVObjectFieldFloat *m_field;
};


#endif // TREEITEM_H
