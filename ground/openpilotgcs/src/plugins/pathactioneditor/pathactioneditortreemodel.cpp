/**
 ******************************************************************************
 *
 * @file       pathactioneditortreemodel.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectBrowserPlugin UAVObject Browser Plugin
 * @{
 * @brief The UAVObject Browser gadget plugin
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

#include "pathactioneditortreemodel.h"
#include "pathactioneditorgadget.h"
#include "fieldtreeitem.h"
#include "uavobjectmanager.h"
#include "uavdataobject.h"
#include "uavmetaobject.h"
#include "uavobjectfield.h"
#include "extensionsystem/pluginmanager.h"
#include <QtGui/QColor>
//#include <QtGui/QIcon>
#include <QMessageBox>
#include <QtCore/QSignalMapper>
#include <QtCore/QDebug>
#include "waypoint.h"
#include "waypointactive.h"
#include "pathaction.h"

PathActionEditorTreeModel::PathActionEditorTreeModel(QObject *parent) :
        QAbstractItemModel(parent),
        m_recentlyUpdatedColor(QColor(255, 230, 230)),
        m_manuallyChangedColor(QColor(230, 230, 255))
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    m_objManager = pm->getObject<UAVObjectManager>();

    connect(m_objManager, SIGNAL(newInstance(UAVObject*)), this, SLOT(newInstance(UAVObject*)));
    connect(m_objManager->getObject("WaypointActive"),SIGNAL(objectUpdated(UAVObject*)), this, SLOT(objUpdated(UAVObject*)));
    connect(m_objManager->getObject("PathAction"),SIGNAL(objectUpdated(UAVObject*)), this, SLOT(objUpdated(UAVObject*)));
    connect(m_objManager->getObject("Waypoint"),SIGNAL(objectUpdated(UAVObject*)), this, SLOT(objUpdated(UAVObject*)));

    setupModelData();
}

PathActionEditorTreeModel::~PathActionEditorTreeModel()
{
    delete m_rootItem;
}

void PathActionEditorTreeModel::setupModelData()
{

    m_actions = new QStringList();
    updateActions();

    // root
    QList<QVariant> rootData;
    rootData << tr("Property") << tr("Value") << tr("Unit");
    m_rootItem = new TreeItem(rootData);

    m_pathactionsTree = new TopTreeItem(tr("PathActions"), m_rootItem);
    m_rootItem->appendChild(m_pathactionsTree);
    m_waypointsTree = new TopTreeItem(tr("Waypoints"), m_rootItem);
    m_rootItem->appendChild(m_waypointsTree);
    connect(m_rootItem, SIGNAL(updateHighlight(TreeItem*)), this, SLOT(updateHighlight(TreeItem*)));
    connect(m_pathactionsTree, SIGNAL(updateHighlight(TreeItem*)), this, SLOT(updateHighlight(TreeItem*)));
    connect(m_waypointsTree, SIGNAL(updateHighlight(TreeItem*)), this, SLOT(updateHighlight(TreeItem*)));

    {
        QList<UAVObject*> list = m_objManager->getObjectInstances("PathAction");
        foreach (UAVObject* obj, list) {
            addInstance(obj,m_pathactionsTree);
        }
    }
    {
        QList<UAVObject*> list = m_objManager->getObjectInstances("Waypoint");
        foreach (UAVObject* obj, list) {
            addInstance(obj,m_waypointsTree);
        }
    }
}

void PathActionEditorTreeModel::updateActions() {
	m_actions->clear();
        QList<UAVObject*> list = m_objManager->getObjectInstances("PathAction");
        foreach (UAVObject* obj, list) {
	    QString title;
	    title.append((QVariant(obj->getInstID()).toString()));
	    title.append(" ");
	    title.append((obj->getField("Mode")->getValue().toString()));
	    title.append(" ");
	    title.append((obj->getField("Command")->getValue().toString()));
	    title.append(":");
	    title.append((obj->getField("EndCondition")->getValue().toString()));
	    title.append(" ");
	    m_actions->append(title);
        }
}

void PathActionEditorTreeModel::addInstance(UAVObject *obj, TreeItem *parent)
{
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(highlightUpdatedObject(UAVObject*)));
    TreeItem *item;
    QString name = QString::number(obj->getInstID());
    item = new InstanceTreeItem(obj, name);
    connect(item, SIGNAL(updateHighlight(TreeItem*)), this, SLOT(updateHighlight(TreeItem*)));
    parent->appendChild(item);
    foreach (UAVObjectField *field, obj->getFields()) {
        if (field->getNumElements() > 1) {
            addArrayField(field, item);
        } else {
            addSingleField(0, field, item);
        }
    }
}


void PathActionEditorTreeModel::addArrayField(UAVObjectField *field, TreeItem *parent)
{
    TreeItem *item = new ArrayFieldTreeItem(field->getName());
    connect(item, SIGNAL(updateHighlight(TreeItem*)), this, SLOT(updateHighlight(TreeItem*)));
    for (uint i = 0; i < field->getNumElements(); ++i) {
        addSingleField(i, field, item);
    }
    parent->appendChild(item);
}

void PathActionEditorTreeModel::addSingleField(int index, UAVObjectField *field, TreeItem *parent)
{
    QList<QVariant> data;
    if (field->getNumElements() == 1)
        data.append(field->getName());
    else
        data.append( QString("[%1]").arg((field->getElementNames())[index]) );

    FieldTreeItem *item;
    UAVObjectField::FieldType type = field->getType();
    // hack: list available actions in an enum
    if (field->getName().compare("Action")==0 && type==UAVObjectField::UINT8) {
	data.append( field->getValue(index).toInt());
        data.append( field->getUnits());
        item = new ActionFieldTreeItem(field, index, data, m_actions);
    } else {
    switch (type) {
    case UAVObjectField::ENUM: {
        QStringList options = field->getOptions();
        QVariant value = field->getValue();
        data.append( options.indexOf(value.toString()) );
        data.append(field->getUnits());
        item = new EnumFieldTreeItem(field, index, data);
        break;
    }
    case UAVObjectField::INT8:
    case UAVObjectField::INT16:
    case UAVObjectField::INT32:
    case UAVObjectField::UINT8:
    case UAVObjectField::UINT16:
    case UAVObjectField::UINT32:
        data.append(field->getValue(index));
        data.append(field->getUnits());
        item = new IntFieldTreeItem(field, index, data);
        break;
    case UAVObjectField::FLOAT32:
        data.append(field->getValue(index));
        data.append(field->getUnits());
        item = new FloatFieldTreeItem(field, index, data);
        break;
    default:
        Q_ASSERT(false);
    }
    }
    connect(item, SIGNAL(updateHighlight(TreeItem*)), this, SLOT(updateHighlight(TreeItem*)));
    parent->appendChild(item);
}

QModelIndex PathActionEditorTreeModel::index(int row, int column, const QModelIndex &parent)
        const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex PathActionEditorTreeModel::index(TreeItem *item)
{
    if (item->parent() == 0)
        return QModelIndex();

    QModelIndex root = index(item->parent());

    for (int i = 0; i < rowCount(root); ++i) {
        QModelIndex childIndex = index(i, 0, root);
        TreeItem *child = static_cast<TreeItem*>(childIndex.internalPointer());
        if (child == item)
            return childIndex;
    }
    Q_ASSERT(false);
    return QModelIndex();
}

QModelIndex PathActionEditorTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int PathActionEditorTreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int PathActionEditorTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    else
        return m_rootItem->columnCount();
}

QVariant PathActionEditorTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() == TreeItem::dataColumn && role == Qt::EditRole) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        return item->data(index.column());
    }
//    if (role == Qt::DecorationRole)
//        return QIcon(":/core/images/openpilot_logo_128.png");

    if (role == Qt::ToolTipRole) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        return item->description();
    }

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    if (index.column() == 0 && role == Qt::BackgroundRole) {
        ObjectTreeItem *objItem = dynamic_cast<ObjectTreeItem*>(item);
        if (objItem && objItem->highlighted())
            return QVariant(m_recentlyUpdatedColor);
    }
    if (index.column() == TreeItem::dataColumn && role == Qt::BackgroundRole) {
        FieldTreeItem *fieldItem = dynamic_cast<FieldTreeItem*>(item);
        if (fieldItem && fieldItem->highlighted())
            return QVariant(m_recentlyUpdatedColor);
        if (fieldItem && fieldItem->changed())
            return QVariant(m_manuallyChangedColor);
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() == TreeItem::dataColumn) {
        EnumFieldTreeItem *fieldItem = dynamic_cast<EnumFieldTreeItem*>(item);
        if (fieldItem) {
            int enumIndex = fieldItem->data(index.column()).toInt();
            return fieldItem->enumOptions(enumIndex);
        } else {
            ActionFieldTreeItem *afieldItem = dynamic_cast<ActionFieldTreeItem*>(item);
            if (afieldItem) {
                int enumIndex = afieldItem->data(index.column()).toInt();
                return afieldItem->enumOptions(enumIndex);
            }
	}
    }

    return item->data(index.column());
}

bool PathActionEditorTreeModel::setData(const QModelIndex &index, const QVariant & value, int role)
{
    Q_UNUSED(role)
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    item->setData(value, index.column());
    return true;
}


Qt::ItemFlags PathActionEditorTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    if (index.column() == TreeItem::dataColumn) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item->isEditable())
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PathActionEditorTreeModel::headerData(int section, Qt::Orientation orientation,
                                        int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_rootItem->data(section);

    return QVariant();
}

void PathActionEditorTreeModel::updateHighlight(TreeItem *item)
{
    QModelIndex itemIndex = index(item);
    Q_ASSERT(itemIndex != QModelIndex());
    emit dataChanged(itemIndex, itemIndex.sibling(itemIndex.row(), TreeItem::dataColumn));

    // update uavobject if any - go up the tree until there is
    ObjectTreeItem *objItem = 0;
    TreeItem *searchItem = item;
    while (searchItem) {
        objItem = dynamic_cast<ObjectTreeItem*>(searchItem);
        if (objItem)
            break;
        searchItem = searchItem->parent();
    }
    if (objItem) {
    item->apply();
    objItem->apply();
        UAVObject *obj = objItem->object();
        Q_ASSERT(obj);
        obj->updated();
    }
}

void PathActionEditorTreeModel::highlightUpdatedObject(UAVObject *obj)
{
    Q_ASSERT(obj);
    if (obj->getName().compare("Waypoint")==0) {
        m_waypointsTree->update();
        //emit dataChanged(index(m_waypointsTree), index(m_waypointsTree));
    } else if (obj->getName().compare("PathAction")==0) {
        m_pathactionsTree->update();
        //emit dataChanged(index(m_pathactionsTree), index(m_pathactionsTree));
    }
}

void PathActionEditorTreeModel::newInstance(UAVObject *obj)
{
   
    if (obj->getName().compare("Waypoint")==0) {
        addInstance(obj,m_waypointsTree);
        m_waypointsTree->update();
    } else if (obj->getName().compare("PathAction")==0) {
        addInstance(obj,m_pathactionsTree);
        m_pathactionsTree->update();
    }
    updateActions();
    emit layoutChanged();
}

void PathActionEditorTreeModel::objUpdated(UAVObject *obj)
{
    quint16 index = m_objManager->getObject("WaypointActive")->getField("Index")->getValue().toInt();
    quint16 action;
    foreach (TreeItem *child,m_waypointsTree->treeChildren()) {
        ObjectTreeItem *objItem = dynamic_cast<ObjectTreeItem*>(child);
        if (index == objItem->object()->getInstID()) {
            child->setActive(true);
            action = objItem->object()->getField("Action")->getValue().toInt();
        } else {
            child->setActive(false);
        }
    }
    foreach (TreeItem *child,m_pathactionsTree->treeChildren()) {
        ObjectTreeItem *objItem = dynamic_cast<ObjectTreeItem*>(child);
        if (action == objItem->object()->getInstID()) {
            child->setActive(true);
        } else {
            child->setActive(false);
        }
    }
    updateActions();
    emit layoutChanged();
}

