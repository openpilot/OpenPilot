/**
 ******************************************************************************
 *
 * @file       uavobjecttreemodel.h
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

#ifndef UAVOBJECTTREEMODEL_H
#define UAVOBJECTTREEMODEL_H

#include "treeitem.h"
#include <QAbstractItemModel>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtGui/QColor>

class TopTreeItem;
class ObjectTreeItem;
class DataObjectTreeItem;
class UAVObject;
class UAVDataObject;
class UAVMetaObject;
class UAVObjectField;
class UAVObjectManager;
class QSignalMapper;
class QTimer;

class UAVObjectTreeModel : public QAbstractItemModel
{
Q_OBJECT
public:
    explicit UAVObjectTreeModel(QObject *parent = 0, bool categorize=true, bool useScientificNotation=false);
    ~UAVObjectTreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant & value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void setRecentlyUpdatedColor(QColor color) { m_recentlyUpdatedColor = color; }
    void setManuallyChangedColor(QColor color) { m_manuallyChangedColor = color; }
    void setRecentlyUpdatedTimeout(int timeout) {
        m_recentlyUpdatedTimeout = timeout;
        TreeItem::setHighlightTime(timeout);
    }
    void setOnlyHilightChangedValues(bool hilight) {m_onlyHilightChangedValues = hilight; }

    QList<QModelIndex> getMetaDataIndexes();

signals:

public slots:
    void newObject(UAVObject *obj);

private slots:
    void highlightUpdatedObject(UAVObject *obj);
    void updateHighlight(TreeItem*);

private:
    void setupModelData(UAVObjectManager *objManager, bool categorize = true);
    QModelIndex index(TreeItem *item);
    void addDataObject(UAVDataObject *obj, bool categorize = true);
    MetaObjectTreeItem *addMetaObject(UAVMetaObject *obj, TreeItem *parent);
    void addArrayField(UAVObjectField *field, TreeItem *parent);
    void addSingleField(int index, UAVObjectField *field, TreeItem *parent);
    void addInstance(UAVObject *obj, TreeItem *parent);

    TreeItem *createCategoryItems(QStringList categoryPath, TreeItem *root);

    QString updateMode(quint8 updateMode);
    ObjectTreeItem *findObjectTreeItem(UAVObject *obj);
    DataObjectTreeItem *findDataObjectTreeItem(UAVDataObject *obj);
    MetaObjectTreeItem *findMetaObjectTreeItem(UAVMetaObject *obj);

    TreeItem *m_rootItem;
    TopTreeItem *m_settingsTree;
    TopTreeItem *m_nonSettingsTree;
    int m_recentlyUpdatedTimeout;
    QColor m_recentlyUpdatedColor;
    QColor m_manuallyChangedColor;
    bool m_onlyHilightChangedValues;
    bool m_useScientificFloatNotation;

    // Highlight manager to handle highlighting of tree items.
    HighLightManager *m_highlightManager;
};

#endif // UAVOBJECTTREEMODEL_H
