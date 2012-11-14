/**
 ******************************************************************************
 *
 * @file       uavobjectbrowserwidget.cpp
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
#include "uavobjectbrowserwidget.h"
#include "uavobjecttreemodel.h"
#include "browseritemdelegate.h"
#include "treeitem.h"
#include "ui_uavobjectbrowser.h"
#include "ui_viewoptions.h"
#include "uavobjectmanager.h"
#include <QStringList>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtCore/QDebug>
#include <QtGui/QItemEditorFactory>
#include "extensionsystem/pluginmanager.h"

UAVObjectBrowserWidget::UAVObjectBrowserWidget(QWidget *parent) : QWidget(parent)
{
    m_browser = new Ui_UAVObjectBrowser();
    m_viewoptions = new Ui_viewoptions();
    m_viewoptionsDialog = new QDialog(this);
    m_viewoptions->setupUi(m_viewoptionsDialog);
    m_browser->setupUi(this);
    m_model = new UAVObjectTreeModel();
    m_browser->treeView->setModel(m_model);
    m_browser->treeView->setColumnWidth(0, 300);
    //m_browser->treeView->expandAll();
    BrowserItemDelegate *m_delegate = new BrowserItemDelegate();
    m_browser->treeView->setItemDelegate(m_delegate);
    m_browser->treeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_browser->treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    showMetaData(m_viewoptions->cbMetaData->isChecked());
    connect(m_browser->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentChanged(QModelIndex,QModelIndex)));
    connect(m_viewoptions->cbMetaData, SIGNAL(toggled(bool)), this, SLOT(showMetaData(bool)));
    connect(m_viewoptions->cbCategorized, SIGNAL(toggled(bool)), this, SLOT(categorize(bool)));
    connect(m_browser->saveSDButton, SIGNAL(clicked()), this, SLOT(saveObject()));
    connect(m_browser->readSDButton, SIGNAL(clicked()), this, SLOT(loadObject()));
    connect(m_browser->eraseSDButton, SIGNAL(clicked()), this, SLOT(eraseObject()));
    connect(m_browser->sendButton, SIGNAL(clicked()), this, SLOT(sendUpdate()));
    connect(m_browser->requestButton, SIGNAL(clicked()), this, SLOT(requestUpdate()));
    connect(m_browser->tbView,SIGNAL(clicked()),this,SLOT(viewSlot()));
    connect(m_viewoptions->cbScientific, SIGNAL(toggled(bool)), this, SLOT(useScientificNotation(bool)));
    connect(m_viewoptions->cbScientific, SIGNAL(toggled(bool)), this, SLOT(viewOptionsChangedSlot()));
    connect(m_viewoptions->cbMetaData, SIGNAL(toggled(bool)), this, SLOT(viewOptionsChangedSlot()));
    connect(m_viewoptions->cbCategorized, SIGNAL(toggled(bool)), this, SLOT(viewOptionsChangedSlot()));
    enableSendRequest(false);
}

UAVObjectBrowserWidget::~UAVObjectBrowserWidget()
{
    delete m_browser;
}

void UAVObjectBrowserWidget::setViewOptions(bool categorized, bool scientific, bool metadata)
{
    m_viewoptions->cbCategorized->setChecked(categorized);
    m_viewoptions->cbMetaData->setChecked(metadata);
    m_viewoptions->cbScientific->setChecked(scientific);
}

void UAVObjectBrowserWidget::showMetaData(bool show)
{
    QList<QModelIndex> metaIndexes = m_model->getMetaDataIndexes();
    foreach(QModelIndex index , metaIndexes)
    {
        m_browser->treeView->setRowHidden(index.row(), index.parent(), !show);
    }
}

void UAVObjectBrowserWidget::categorize(bool categorize)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager);

    UAVObjectTreeModel* tmpModel = m_model;
    m_model = new UAVObjectTreeModel(0, categorize,m_viewoptions->cbScientific->isChecked());
    m_model->setRecentlyUpdatedColor(m_recentlyUpdatedColor);
    m_model->setManuallyChangedColor(m_manuallyChangedColor);
    m_model->setRecentlyUpdatedTimeout(m_recentlyUpdatedTimeout);
    m_model->setOnlyHilightChangedValues(m_onlyHilightChangedValues);
    m_browser->treeView->setModel(m_model);
    showMetaData(m_viewoptions->cbMetaData->isChecked());

    delete tmpModel;
}

void UAVObjectBrowserWidget::useScientificNotation(bool scientific)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objManager);

    UAVObjectTreeModel* tmpModel = m_model;
    m_model = new UAVObjectTreeModel(0, m_viewoptions->cbCategorized,scientific);
    m_model->setRecentlyUpdatedColor(m_recentlyUpdatedColor);
    m_model->setManuallyChangedColor(m_manuallyChangedColor);
    m_model->setRecentlyUpdatedTimeout(m_recentlyUpdatedTimeout);
    m_browser->treeView->setModel(m_model);
    showMetaData(m_viewoptions->cbMetaData->isChecked());

    delete tmpModel;
}

void UAVObjectBrowserWidget::sendUpdate()
{
    this->setFocus();
    ObjectTreeItem *objItem = findCurrentObjectTreeItem();
    Q_ASSERT(objItem);
    objItem->apply();
    UAVObject *obj = objItem->object();
    Q_ASSERT(obj);
    obj->updated();
}

void UAVObjectBrowserWidget::requestUpdate()
{
    ObjectTreeItem *objItem = findCurrentObjectTreeItem();
    Q_ASSERT(objItem);
    UAVObject *obj = objItem->object();
    Q_ASSERT(obj);
    obj->requestUpdate();
}

ObjectTreeItem *UAVObjectBrowserWidget::findCurrentObjectTreeItem()
{
    QModelIndex current = m_browser->treeView->currentIndex();
    TreeItem *item = static_cast<TreeItem*>(current.internalPointer());
    ObjectTreeItem *objItem = 0;
    while (item) {
        objItem = dynamic_cast<ObjectTreeItem*>(item);
        if (objItem)
            break;
        item = item->parent();
    }
    return objItem;
}

void UAVObjectBrowserWidget::saveObject()
{
    this->setFocus();
    // Send update so that the latest value is saved
    sendUpdate();
    // Save object
    ObjectTreeItem *objItem = findCurrentObjectTreeItem();
    Q_ASSERT(objItem);
    UAVObject *obj = objItem->object();
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_SAVE, obj);
}

void UAVObjectBrowserWidget::loadObject()
{
    // Load object
    ObjectTreeItem *objItem = findCurrentObjectTreeItem();
    Q_ASSERT(objItem);
    UAVObject *obj = objItem->object();
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_LOAD, obj);
    // Retrieve object so that latest value is displayed
    requestUpdate();
}

void UAVObjectBrowserWidget::eraseObject()
{
    ObjectTreeItem *objItem = findCurrentObjectTreeItem();
    Q_ASSERT(objItem);
    UAVObject *obj = objItem->object();
    Q_ASSERT(obj);
    updateObjectPersistance(ObjectPersistence::OPERATION_DELETE, obj);
}

void UAVObjectBrowserWidget::updateObjectPersistance(ObjectPersistence::OperationOptions op, UAVObject *obj)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    ObjectPersistence* objper = dynamic_cast<ObjectPersistence*>( objManager->getObject(ObjectPersistence::NAME) );
    if (obj != NULL)
    {
        ObjectPersistence::DataFields data;
        data.Operation = op;
        data.Selection = ObjectPersistence::SELECTION_SINGLEOBJECT;
        data.ObjectID = obj->getObjID();
        data.InstanceID = obj->getInstID();
        objper->setData(data);
        objper->updated();
    }
}

void UAVObjectBrowserWidget::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    TreeItem *item = static_cast<TreeItem*>(current.internalPointer());
    bool enable = true;
    if (current == QModelIndex())
        enable = false;
    TopTreeItem *top = dynamic_cast<TopTreeItem*>(item);
    ObjectTreeItem *data = dynamic_cast<ObjectTreeItem*>(item);
    if (top || (data && !data->object()))
        enable = false;
    enableSendRequest(enable);
}

void UAVObjectBrowserWidget::viewSlot()
{
    if(m_viewoptionsDialog->isVisible())
        m_viewoptionsDialog->setVisible(false);
    else
    {
        QPoint pos=QCursor::pos();
        pos.setX(pos.x()-m_viewoptionsDialog->width());
        m_viewoptionsDialog->move(pos);
        m_viewoptionsDialog->show();
    }
}

void UAVObjectBrowserWidget::viewOptionsChangedSlot()
{
    emit viewOptionsChanged(m_viewoptions->cbCategorized->isChecked(),m_viewoptions->cbScientific->isChecked(),m_viewoptions->cbMetaData->isChecked());
}

void UAVObjectBrowserWidget::enableSendRequest(bool enable)
{
    m_browser->sendButton->setEnabled(enable);
    m_browser->requestButton->setEnabled(enable);
    m_browser->saveSDButton->setEnabled(enable);
    m_browser->readSDButton->setEnabled(enable);
    m_browser->eraseSDButton->setEnabled(enable);
}


