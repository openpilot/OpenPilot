/**
 ******************************************************************************
 *
 * @file       pathactioneditortreemodel.h
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

#ifndef PATHACTIONEDITORTREEMODEL_H
#define PATHACTIONEDITORTREEMODEL_H

#include "treeitem.h"
#include <QAbstractItemModel>
#include <QtCore/QMap>
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

class PathActionEditorTreeModel : public QAbstractItemModel
{
Q_OBJECT
public:
    explicit PathActionEditorTreeModel(QObject *parent = 0);
    ~PathActionEditorTreeModel();

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

signals:

public slots:
    void newInstance(UAVObject *obj);
    void objUpdated(UAVObject *obj);

private slots:
    void highlightUpdatedObject(UAVObject *obj);
    void updateHighlight(TreeItem*);

private:
    QModelIndex index(TreeItem *item);
    void addArrayField(UAVObjectField *field, TreeItem *parent);

    void addSingleField(int index, UAVObjectField *field, TreeItem *parent);
    void addInstance(UAVObject *obj, TreeItem *parent);
    //QString updateMode(quint8 updateMode);
    void setupModelData();
    void updateActions();

    UAVObjectManager *m_objManager;
    QStringList *m_actions;

    TreeItem *m_rootItem;
    TopTreeItem *m_pathactionsTree;
    TopTreeItem *m_waypointsTree;
    QColor m_recentlyUpdatedColor;
    QColor m_manuallyChangedColor;
};

#endif // PATHACTIONEDITORTREEMODEL_H
