/**
 ******************************************************************************
 *
 * @file       uavobjecttreemodel.cpp
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

#include "uavobjecttreemodel.h"
#include "treeitem.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavdataobject.h"
#include "uavobjects/uavmetaobject.h"
#include "uavobjects/uavobjectfieldenum.h"
#include "uavobjects/uavobjectfielduint8.h"
#include "uavobjects/uavobjectfielduint16.h"
#include "uavobjects/uavobjectfielduint32.h"
#include "uavobjects/uavobjectfieldint8.h"
#include "uavobjects/uavobjectfieldint16.h"
#include "uavobjects/uavobjectfieldint32.h"
#include "uavobjects/uavobjectfieldfloat.h"
#include "extensionsystem/pluginmanager.h"
#include <QtGui/QColor>
#include <QtCore/QDebug>
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


UAVObjectTreeModel::UAVObjectTreeModel(QObject *parent) :
        QAbstractItemModel(parent)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    connect(objManager, SIGNAL(newObject(UAVObject*)), this, SLOT(newObject(UAVObject*)));
    connect(objManager, SIGNAL(newInstance(UAVObject*)), this, SLOT(newObject(UAVObject*)));

    setupModelData(objManager);
}

void UAVObjectTreeModel::setupModelData(UAVObjectManager *objManager)
{
    // root
    QList<QVariant> rootData;
    rootData << tr("Property") << tr("Value") << tr("Unit");
    rootItem = new TreeItem(rootData);

    m_settingsTree = new TopTreeItem(tr("Settings"), rootItem);
    rootItem->appendChild(m_settingsTree);
    m_nonSettingsTree = new TopTreeItem(tr("Data Objects"), rootItem);
    rootItem->appendChild(m_nonSettingsTree);

    QList< QList<UAVDataObject*> > objList = objManager->getDataObjects();
    foreach (QList<UAVDataObject*> list, objList) {
        foreach (UAVDataObject* obj, list) {
            addDataObject(obj);
        }
    }
}

void UAVObjectTreeModel::newObject(UAVObject *obj)
{
    UAVDataObject *dobj = qobject_cast<UAVDataObject*>(obj);
    if (dobj)
        addDataObject(dobj);
}

void UAVObjectTreeModel::addDataObject(UAVDataObject *obj)
{
    TopTreeItem *root = obj->isSettings() ? m_settingsTree : m_nonSettingsTree;
    if (root->objIds().contains(obj->getObjID())) {
        int index = root->objIds().indexOf(obj->getObjID());
        addInstance(obj, root->child(index));
    } else {
        DataObjectTreeItem *data = new DataObjectTreeItem(obj->getName());
        int index = root->nameIndex(obj->getName());
        root->insert(index, data);
        root->insertObjId(index, obj->getObjID());
        UAVMetaObject *meta = obj->getMetaObject();
        addMetaObject(meta, data);
        addInstance(obj, data);
    }
}

void UAVObjectTreeModel::addMetaObject(UAVMetaObject *obj, TreeItem *parent)
{
    MetaObjectTreeItem *meta = new MetaObjectTreeItem(tr("Meta Data"));
    UAVObject::Metadata md = obj->getMetadata();
    QStringList boolList;
    boolList << tr("False") << tr("True");
    QStringList updateModeList;
    updateModeList << tr("Periodic") << tr("On Change") << tr("Manual") << tr("Never");
    QList<QVariant> data;
    QString msUnit = tr("ms");
    data << tr("Flight Telemetry Acked") << md.flightTelemetryAcked;
    meta->appendChild(new EnumFieldTreeItem(0, data, boolList));
    data.clear();
    data << tr("Flight Telemetry Update Mode") << md.flightTelemetryUpdateMode;
    meta->appendChild(new EnumFieldTreeItem(0, data, updateModeList));
    data.clear();
    data << tr("Flight Telemetry Update Period") << md.flightTelemetryUpdatePeriod << msUnit;
    meta->appendChild(new IntFieldTreeItem(0, data, QINT32MIN, QINT32MAX));
    data.clear();
    data << tr("GCS Telemetry Acked") << md.gcsTelemetryAcked;
    meta->appendChild(new EnumFieldTreeItem(0, data, boolList));
    data.clear();
    data << tr("GCS Telemetry Update Mode") << md.gcsTelemetryUpdateMode;
    meta->appendChild(new EnumFieldTreeItem(0, data, updateModeList));
    data.clear();
    data << tr("GCS Telemetry Update Period") << md.gcsTelemetryUpdatePeriod << msUnit;
    meta->appendChild(new IntFieldTreeItem(0, data, QINT32MIN, QINT32MAX));
    data.clear();
    data << tr("Logging Update Mode") << md.loggingUpdateMode;
    meta->appendChild(new EnumFieldTreeItem(0, data, updateModeList));
    data.clear();
    data << tr("Logging Update Period") << md.loggingUpdatePeriod << msUnit;
    meta->appendChild(new IntFieldTreeItem(0, data, QINT32MIN, QINT32MAX));
    parent->appendChild(meta);
}

void UAVObjectTreeModel::addInstance(UAVObject *obj, TreeItem *parent)
{
    TreeItem *item;
    if (!obj->isSingleInstance()) {
        QString name = tr("Instance") +  " " + QString::number(obj->getInstID());
        item = new InstanceTreeItem(name);
    } else {
        item = parent;
    }
    foreach (UAVObjectField *field, obj->getFields()) {
        if (field->getNumElements() > 1) {
            addArrayField(field, item);
        } else {
            addSingleField(0, field, item);
        }
    }
    if (item != parent)
        parent->appendChild(item);
}


void UAVObjectTreeModel::addArrayField(UAVObjectField *field, TreeItem *parent)
{
    TreeItem *item = new ArrayFieldTreeItem(field->getName());
//    UAVObjectFieldEnum *enumField = qobject_cast<UAVObjectFieldEnum*>(field);
    for (uint i = 0; i < field->getNumElements(); ++i) {
        addSingleField(i, field, item);
    }
    parent->appendChild(item);
}

void UAVObjectTreeModel::addSingleField(int index, UAVObjectField *field, TreeItem *parent)
{
    QList<QVariant> data;
    if (field->getNumElements() == 1)
        data.append(field->getName());
    else
        data.append(QString("[%1]").arg(index));
    UAVObjectFieldEnum *enumField = dynamic_cast<UAVObjectFieldEnum*>(field);
    UAVObjectFieldInt8 *int8Field = dynamic_cast<UAVObjectFieldInt8*>(field);
    UAVObjectFieldInt16 *int16Field = dynamic_cast<UAVObjectFieldInt16*>(field);
    UAVObjectFieldInt32 *int32Field = dynamic_cast<UAVObjectFieldInt32*>(field);
    UAVObjectFieldUInt8 *uInt8Field = dynamic_cast<UAVObjectFieldUInt8*>(field);
    UAVObjectFieldUInt16 *uInt16Field = dynamic_cast<UAVObjectFieldUInt16*>(field);
    UAVObjectFieldUInt32 *uInt32Field = dynamic_cast<UAVObjectFieldUInt32*>(field);
    UAVObjectFieldFloat *floatField = dynamic_cast<UAVObjectFieldFloat*>(field);

    FieldTreeItem *item;
    if (enumField) {
        data.append(enumField->getSelectedIndex());
        data.append(field->getUnits());
        item = new EnumFieldTreeItem(index, data, enumField->getOptions());
    } else if (int8Field) {
        data.append(int8Field->getValue());
        data.append(field->getUnits());
        item = new IntFieldTreeItem(index, data, QINT8MIN, QINT8MAX);
    } else if (int16Field) {
        data.append(int16Field->getValue());
        data.append(field->getUnits());
        item = new IntFieldTreeItem(index, data, QINT16MIN, QINT16MAX);
    } else if (int32Field) {
        data.append(int32Field->getValue());
        data.append(field->getUnits());
        item = new IntFieldTreeItem(index, data, QINT32MIN, QINT32MAX);
    } else if (uInt8Field) {
        data.append(uInt8Field->getValue());
        data.append(field->getUnits());
        item = new IntFieldTreeItem(index, data, QUINTMIN, QUINT8MAX);
    } else if (uInt16Field) {
        data.append(uInt16Field->getValue());
        data.append(field->getUnits());
        item = new IntFieldTreeItem(index, data, QUINTMIN, QUINT16MAX);
    } else if (uInt32Field) {
        data.append(uInt32Field->getValue());
        data.append(field->getUnits());
        item = new IntFieldTreeItem(index, data, QUINTMIN, QUINT32MAX);
    } else if (floatField) {
        data.append(floatField->getValue());
        data.append(field->getUnits());
        item = new FloatFieldTreeItem(index, data);
    } else {
        data.append("Data Error");
        data.append(field->getUnits());
        item = new FieldTreeItem(index, data);
    }
    parent->appendChild(item);
}

QModelIndex UAVObjectTreeModel::index(int row, int column, const QModelIndex &parent)
        const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex UAVObjectTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int UAVObjectTreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int UAVObjectTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant UAVObjectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() == 1 && role == Qt::EditRole) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        return item->data(index.column());
    }

    //XXX
//    if (index.column() == 1 && role == Qt::ForegroundRole)
//        return QVariant( QColor( Qt::red ) );


    if (role != Qt::DisplayRole)
        return QVariant();

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    if (index.column() == 1) {
        EnumFieldTreeItem *fieldItem = dynamic_cast<EnumFieldTreeItem*>(item);
        if (fieldItem) {
            int enumIndex = fieldItem->data(index.column()).toInt();
            return fieldItem->enumOptions.at(enumIndex);
        }
    }

    return item->data(index.column());
}

bool UAVObjectTreeModel::setData(const QModelIndex &index, const QVariant & value, int role)
{
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    item->setData(index.column(), value);
    return true;
}


Qt::ItemFlags UAVObjectTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    if (index.column() == 1) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item->isEditable())
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant UAVObjectTreeModel::headerData(int section, Qt::Orientation orientation,
                                        int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

