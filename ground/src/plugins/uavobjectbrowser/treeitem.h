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
#include <limits>

#define QINT8MIN std::numeric_limits<qint8>::min()
#define QINT8MAX std::numeric_limits<qint8>::max()
#define QUINTMIN std::numeric_limits<quint8>::min()
#define QUINT8MAX std::numeric_limits<quint8>::max()
#define QINT16MIN std::numeric_limits<qint16>::min()
#define QINT16MAX std::numeric_limits<qint16>::max()
#define QUINT16MAX std::numeric_limits<quint16>::max()
#define QINT32MIN std::numeric_limits<qint32>::min()
#define QINT32MAX std::numeric_limits<qint32>::max()
#define QUINT32MAX std::numeric_limits<qint32>::max()

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
     QVariant data(int column = 1) const;
     // only column 1 is changed with setData currently
     // other columns are initialized in constructor
     virtual void setData(QVariant value, int column = 1);
     int row() const;
     TreeItem *parent() { return m_parent; }
     void setParent(TreeItem *parent) { m_parent = parent; }
     inline virtual bool isEditable() { return false; }
     virtual void update() {
         foreach(TreeItem *child, m_children)
             child->update();
     }
     virtual void setHighlight(bool highlight) { m_highlight = highlight; }
     inline virtual bool highlight() { return m_highlight; }

 private:
     QList<TreeItem*> m_children;
     // m_data contains: [0] property name, [1] value, and [2] unit
     QList<QVariant> m_data;
     TreeItem *m_parent;
     bool m_highlight;
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
    DataObjectTreeItem(const QList<QVariant> &data, TreeItem *parent = 0) :
            TreeItem(data, parent), m_obj(0), m_highlight(false) { }
    DataObjectTreeItem(const QVariant &data, TreeItem *parent = 0) :
            TreeItem(data, parent), m_obj(0), m_highlight(false) { }
    void setObject(UAVObject *obj) { m_obj = obj; }
    inline UAVObject *object() { return m_obj; }
private:
    UAVObject *m_obj;
};

class MetaObjectTreeItem : public TreeItem
{
public:
    MetaObjectTreeItem(const QList<QVariant> &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
    MetaObjectTreeItem(const QVariant &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
    void update() { }
};

class InstanceTreeItem : public DataObjectTreeItem
{
public:
    InstanceTreeItem(UAVObject *obj, const QList<QVariant> &data, TreeItem *parent = 0) :
            DataObjectTreeItem(data, parent) { setObject(obj); }
    InstanceTreeItem(UAVObject *obj, const QVariant &data, TreeItem *parent = 0) :
            DataObjectTreeItem(data, parent) { setObject(obj); }
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
            TreeItem(data, parent), m_index(index), m_changed(false) { }
    FieldTreeItem(int index, const QVariant &data, TreeItem *parent = 0) :
            TreeItem(data, parent), m_index(index), m_changed(false) { }
    bool isEditable() { return true; }
    bool isChanged() { return m_changed; }
    void setChanged(bool changed) { m_changed = changed; }
    virtual bool isIntType() { return false; }
    virtual bool isEnum() { return false; }
    virtual bool isFloatType() { return false; }
    virtual void apply(int column)  { }
protected:
    int m_index;
private:
    bool m_changed;
};

class EnumFieldTreeItem : public FieldTreeItem
{
public:
    EnumFieldTreeItem(UAVObjectFieldEnum *field, int index, const QList<QVariant> &data,
                      TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), enumOptions(field->getOptions()), m_field(field) { }
    EnumFieldTreeItem(UAVObjectFieldEnum *field, int index, const QVariant &data,
                      TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), enumOptions(field->getOptions()), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getSelectedIndex(m_index) != value);
        TreeItem::setData(value, column);
        apply(column);
    }
    void apply(int column) {
        int value = data(column).toInt();
        m_field->setSelectedIndex(value, m_index);
        setChanged(false);
    }
    void update() {
        int value = m_field->getSelectedIndex(m_index);
        if (data() != value) {
            setHighlight(true);
            TreeItem::setData(value);
        } else {
            setHighlight(false);
        }
    }

    bool isEnum() { return true; }
    QStringList enumOptions;
private:
    UAVObjectFieldEnum *m_field;
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
    Int8FieldTreeItem(UAVObjectFieldInt8 *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QINT8MIN, QINT8MAX, parent), m_field(field) { }
    Int8FieldTreeItem(UAVObjectFieldInt8 *field, int index, const QVariant &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QINT8MIN, QINT8MAX, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply(int column) {
        m_field->setValue(data(column).toInt());
    }
private:
    UAVObjectFieldInt8 *m_field;
};

class Int16FieldTreeItem : public IntFieldTreeItem
{
public:
    Int16FieldTreeItem(UAVObjectFieldInt16 *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QINT16MIN, QINT16MAX, parent), m_field(field) { }
    Int16FieldTreeItem(UAVObjectFieldInt16 *field, int index, const QVariant &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QINT16MIN, QINT16MAX, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply(int column) {
        m_field->setValue(data(column).toInt());
    }
private:
    UAVObjectFieldInt16 *m_field;
};

class Int32FieldTreeItem : public IntFieldTreeItem
{
public:
    Int32FieldTreeItem(UAVObjectFieldInt32 *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QINT32MIN, QINT32MAX, parent), m_field(field) { }
    Int32FieldTreeItem(UAVObjectFieldInt32 *field, int index, const QVariant &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QINT32MIN, QINT32MAX, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply(int column) {
        int value = data(column).toInt();
        m_field->setValue(value);
    }
private:
    UAVObjectFieldInt32 *m_field;
};

class UInt8FieldTreeItem : public IntFieldTreeItem
{
public:
    UInt8FieldTreeItem(UAVObjectFieldUInt8 *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QUINTMIN, QUINT8MAX, parent), m_field(field) { }
    UInt8FieldTreeItem(UAVObjectFieldUInt8 *field, int index, const QVariant &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QUINTMIN, QUINT8MAX, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply(int column) {
        m_field->setValue(data(column).toInt());
    }
private:
    UAVObjectFieldUInt8 *m_field;
};

class UInt16FieldTreeItem : public IntFieldTreeItem
{
public:
    UInt16FieldTreeItem(UAVObjectFieldUInt16 *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QUINTMIN, QUINT16MAX, parent), m_field(field) { }
    UInt16FieldTreeItem(UAVObjectFieldUInt16 *field, int index, const QVariant &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QUINTMIN, QUINT16MAX, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply(int column) {
        m_field->setValue(data(column).toInt());
    }
private:
    UAVObjectFieldUInt16 *m_field;
};

class UInt32FieldTreeItem : public IntFieldTreeItem
{
public:
    UInt32FieldTreeItem(UAVObjectFieldUInt32 *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QUINTMIN, QUINT32MAX, parent), m_field(field) { }
    UInt32FieldTreeItem(UAVObjectFieldUInt32 *field, int index, const QVariant &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QUINTMIN, QUINT32MAX, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply(int column) {
        m_field->setValue(data(column).toInt());
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
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply(int column) {
        m_field->setValue(data(column).toDouble(), m_index);
    }
private:
    UAVObjectFieldFloat *m_field;
};


#endif // TREEITEM_H
