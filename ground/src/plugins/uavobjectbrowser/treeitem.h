/**
 ******************************************************************************
 *
 * @file       treeitem.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   uavobjectbrowserplugin
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
#include "uavobjects/uavobjectfield.h"
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QTimer>
#include <QtCore/QObject>


class TreeItem : public QObject
{
Q_OBJECT
public:
    TreeItem(const QList<QVariant> &data, TreeItem *parent = 0);
    TreeItem(const QVariant &data, TreeItem *parent = 0);
    virtual ~TreeItem();

    void appendChild(TreeItem *child);
    void insert(int index, TreeItem *child);

    TreeItem *child(int row);
    inline QList<TreeItem*> treeChildren() const { return m_children; }
    int childCount() const;
    int columnCount() const;
    QVariant data(int column = 1) const;
    // only column 1 (TreeItem::dataColumn) is changed with setData currently
    // other columns are initialized in constructor
    virtual void setData(QVariant value, int column = 1);
    int row() const;
    TreeItem *parent() { return m_parent; }
    void setParentTree(TreeItem *parent) { m_parent = parent; }
    inline virtual bool isEditable() { return false; }
    virtual void update();
    virtual void apply();

    inline bool highlighted() { return m_highlight; }
    void setHighlight(bool highlight);
    static void setHighlightTime(int time) { m_highlightTimeMs = time; }

    inline bool changed() { return m_changed; }
    inline void setChanged(bool changed) { m_changed = changed; }

signals:
    void updateHighlight(TreeItem*);

private slots:
    void removeHighlight();

private:
    QList<TreeItem*> m_children;
    // m_data contains: [0] property name, [1] value, and [2] unit
    QList<QVariant> m_data;
    TreeItem *m_parent;
    bool m_highlight;
    bool m_changed;
    QTimer m_timer;
public:
    static const int dataColumn = 1;
private:
    static int m_highlightTimeMs;
};

class TopTreeItem : public TreeItem
{
Q_OBJECT
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

class ObjectTreeItem : public TreeItem
{
Q_OBJECT
public:
    ObjectTreeItem(const QList<QVariant> &data, TreeItem *parent = 0) :
            TreeItem(data, parent), m_obj(0) { }
    ObjectTreeItem(const QVariant &data, TreeItem *parent = 0) :
            TreeItem(data, parent), m_obj(0) { }
    void setObject(UAVObject *obj) { m_obj = obj; }
    inline UAVObject *object() { return m_obj; }
private:
    UAVObject *m_obj;
};

class MetaObjectTreeItem : public ObjectTreeItem
{
Q_OBJECT
public:
    MetaObjectTreeItem(UAVObject *obj, const QList<QVariant> &data, TreeItem *parent = 0) :
            ObjectTreeItem(data, parent) { setObject(obj); }
    MetaObjectTreeItem(UAVObject *obj, const QVariant &data, TreeItem *parent = 0) :
            ObjectTreeItem(data, parent) { setObject(obj); }
};

class DataObjectTreeItem : public ObjectTreeItem
{
Q_OBJECT
public:
    DataObjectTreeItem(const QList<QVariant> &data, TreeItem *parent = 0) :
            ObjectTreeItem(data, parent) { }
    DataObjectTreeItem(const QVariant &data, TreeItem *parent = 0) :
            ObjectTreeItem(data, parent) { }
    virtual void apply() {
        foreach(TreeItem *child, treeChildren()) {
            MetaObjectTreeItem *metaChild = dynamic_cast<MetaObjectTreeItem*>(child);
            if (!metaChild)
                child->apply();
        }
    }
    virtual void update() {
        foreach(TreeItem *child, treeChildren()) {
            MetaObjectTreeItem *metaChild = dynamic_cast<MetaObjectTreeItem*>(child);
            if (!metaChild)
                child->update();
        }
    }
};

class InstanceTreeItem : public DataObjectTreeItem
{
Q_OBJECT
public:
    InstanceTreeItem(UAVObject *obj, const QList<QVariant> &data, TreeItem *parent = 0) :
            DataObjectTreeItem(data, parent) { setObject(obj); }
    InstanceTreeItem(UAVObject *obj, const QVariant &data, TreeItem *parent = 0) :
            DataObjectTreeItem(data, parent) { setObject(obj); }
    virtual void apply() { TreeItem::apply(); }
    virtual void update() { TreeItem::update(); }
};

class ArrayFieldTreeItem : public TreeItem
{
Q_OBJECT
public:
    ArrayFieldTreeItem(const QList<QVariant> &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
    ArrayFieldTreeItem(const QVariant &data, TreeItem *parent = 0) : TreeItem(data, parent) { }
};

#endif // TREEITEM_H
