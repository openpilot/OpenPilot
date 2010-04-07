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
     void setData(int column, QVariant value);
     int row() const;
     TreeItem *parent() { return m_parent; }
     void setParent(TreeItem *parent) { m_parent = parent; }
     virtual bool isEditable() { return false; }

 private:
     QList<TreeItem*> m_children;
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
    EnumFieldTreeItem(int index, const QList<QVariant> &data, QStringList enumOptions, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), enumOptions(enumOptions) { }
    EnumFieldTreeItem(int index, const QVariant &data, QStringList enumOptions, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), enumOptions(enumOptions) { }
    bool isEnum() { return true; }
    QStringList enumOptions;
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

class FloatFieldTreeItem : public FieldTreeItem
{
public:
    FloatFieldTreeItem(int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent) { }
    FloatFieldTreeItem(int index, const QVariant &data, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent) { }
    bool isFloatType() { return true; }
};


#endif // TREEITEM_H
