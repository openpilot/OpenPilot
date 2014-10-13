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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QtCore/QDebug>
#include <QItemEditorFactory>
#include "extensionsystem/pluginmanager.h"

UAVObjectBrowserWidget::UAVObjectBrowserWidget(QWidget *parent) : QWidget(parent)
{
    m_browser     = new Ui_UAVObjectBrowser();
    m_viewoptions = new Ui_viewoptions();
    m_viewoptionsDialog = new QDialog(this);
    m_viewoptions->setupUi(m_viewoptionsDialog);
    m_browser->setupUi(this);
    m_model = new UAVObjectTreeModel();
    m_browser->treeView->setModel(m_model);
    m_browser->treeView->setColumnWidth(0, 300);
    BrowserItemDelegate *m_delegate = new BrowserItemDelegate();
    m_browser->treeView->setItemDelegate(m_delegate);
    m_browser->treeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_browser->treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    showMetaData(m_viewoptions->cbMetaData->isChecked());
    showDescription(m_viewoptions->cbDescription->isChecked());
    connect(m_browser->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(currentChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);
    connect(m_viewoptions->cbMetaData, SIGNAL(toggled(bool)), this, SLOT(showMetaData(bool)));
    connect(m_viewoptions->cbCategorized, SIGNAL(toggled(bool)), this, SLOT(categorize(bool)));
    connect(m_viewoptions->cbDescription, SIGNAL(toggled(bool)), this, SLOT(showDescription(bool)));
    connect(m_browser->saveSDButton, SIGNAL(clicked()), this, SLOT(saveObject()));
    connect(m_browser->readSDButton, SIGNAL(clicked()), this, SLOT(loadObject()));
    connect(m_browser->eraseSDButton, SIGNAL(clicked()), this, SLOT(eraseObject()));
    connect(m_browser->sendButton, SIGNAL(clicked()), this, SLOT(sendUpdate()));
    connect(m_browser->requestButton, SIGNAL(clicked()), this, SLOT(requestUpdate()));
    connect(m_browser->tbView, SIGNAL(clicked()), this, SLOT(viewSlot()));
    connect(m_viewoptions->cbScientific, SIGNAL(toggled(bool)), this, SLOT(useScientificNotation(bool)));
    connect(m_viewoptions->cbScientific, SIGNAL(toggled(bool)), this, SLOT(viewOptionsChangedSlot()));
    connect(m_viewoptions->cbMetaData, SIGNAL(toggled(bool)), this, SLOT(viewOptionsChangedSlot()));
    connect(m_viewoptions->cbCategorized, SIGNAL(toggled(bool)), this, SLOT(viewOptionsChangedSlot()));
    connect(m_viewoptions->cbDescription, SIGNAL(toggled(bool)), this, SLOT(viewOptionsChangedSlot()));
    connect(m_browser->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved()));
    enableSendRequest(false);
}

UAVObjectBrowserWidget::~UAVObjectBrowserWidget()
{
    delete m_browser;
}

void UAVObjectBrowserWidget::setViewOptions(bool categorized, bool scientific, bool metadata, bool description)
{
    m_viewoptions->cbCategorized->setChecked(categorized);
    m_viewoptions->cbMetaData->setChecked(metadata);
    m_viewoptions->cbScientific->setChecked(scientific);
    m_viewoptions->cbDescription->setChecked(description);
}

void UAVObjectBrowserWidget::setSplitterSizes(QList<QVariant> sizes)
{
    QList<int> sizs;
    foreach (QVariant size, sizes) {
        sizs << size.toInt();
    }
    m_browser->splitter->setSizes(sizs);
}

void UAVObjectBrowserWidget::showMetaData(bool show)
{
    QList<QModelIndex> metaIndexes = m_model->getMetaDataIndexes();
    foreach(QModelIndex index, metaIndexes) {
        m_browser->treeView->setRowHidden(index.row(), index.parent(), !show);
    }
}

void UAVObjectBrowserWidget::showDescription(bool show)
{
    m_browser->descriptionText->setVisible(show);
}

void UAVObjectBrowserWidget::categorize(bool categorize)
{
    UAVObjectTreeModel *tmpModel = m_model;

    m_model = new UAVObjectTreeModel(0, categorize, m_viewoptions->cbScientific->isChecked());
    m_model->setRecentlyUpdatedColor(m_recentlyUpdatedColor);
    m_model->setManuallyChangedColor(m_manuallyChangedColor);
    m_model->setRecentlyUpdatedTimeout(m_recentlyUpdatedTimeout);
    m_model->setOnlyHilightChangedValues(m_onlyHilightChangedValues);
    m_browser->treeView->setModel(m_model);
    showMetaData(m_viewoptions->cbMetaData->isChecked());
    connect(m_browser->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(currentChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);

    delete tmpModel;
}

void UAVObjectBrowserWidget::useScientificNotation(bool scientific)
{
    UAVObjectTreeModel *tmpModel = m_model;

    m_model = new UAVObjectTreeModel(0, m_viewoptions->cbCategorized->isChecked(), scientific);
    m_model->setRecentlyUpdatedColor(m_recentlyUpdatedColor);
    m_model->setManuallyChangedColor(m_manuallyChangedColor);
    m_model->setRecentlyUpdatedTimeout(m_recentlyUpdatedTimeout);
    m_browser->treeView->setModel(m_model);
    showMetaData(m_viewoptions->cbMetaData->isChecked());
    connect(m_browser->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(currentChanged(QModelIndex, QModelIndex)), Qt::UniqueConnection);

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
    QModelIndex current     = m_browser->treeView->currentIndex();
    TreeItem *item = static_cast<TreeItem *>(current.internalPointer());
    ObjectTreeItem *objItem = 0;

    while (item) {
        objItem = dynamic_cast<ObjectTreeItem *>(item);
        if (objItem) {
            break;
        }
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
    ObjectPersistence *objper    = dynamic_cast<ObjectPersistence *>(objManager->getObject(ObjectPersistence::NAME));

    if (obj != NULL) {
        ObjectPersistence::DataFields data;
        data.Operation  = op;
        data.Selection  = ObjectPersistence::SELECTION_SINGLEOBJECT;
        data.ObjectID   = obj->getObjID();
        data.InstanceID = obj->getInstID();
        objper->setData(data);
        objper->updated();
    }
}

void UAVObjectBrowserWidget::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    TreeItem *item = static_cast<TreeItem *>(current.internalPointer());
    bool enable    = true;
    if (current == QModelIndex()) {
        enable = false;
    }
    TopTreeItem *top     = dynamic_cast<TopTreeItem *>(item);
    ObjectTreeItem *data = dynamic_cast<ObjectTreeItem *>(item);
    if (top || (data && !data->object())) {
        enable = false;
    }
    enableSendRequest(enable);
    updateDescription();
}

void UAVObjectBrowserWidget::viewSlot()
{
    if (m_viewoptionsDialog->isVisible()) {
        m_viewoptionsDialog->setVisible(false);
    } else {
        QPoint pos = QCursor::pos();
        pos.setX(pos.x() - m_viewoptionsDialog->width());
        m_viewoptionsDialog->move(pos);
        m_viewoptionsDialog->show();
    }
}

void UAVObjectBrowserWidget::viewOptionsChangedSlot()
{
    emit viewOptionsChanged(m_viewoptions->cbCategorized->isChecked(), m_viewoptions->cbScientific->isChecked(),
                            m_viewoptions->cbMetaData->isChecked(), m_viewoptions->cbDescription->isChecked());
}

void UAVObjectBrowserWidget::splitterMoved()
{
    QList<QVariant> sizs;
    foreach (int size, m_browser->splitter->sizes()) {
        sizs << QVariant(size);
    }

    emit splitterChanged(sizs);
}

QString UAVObjectBrowserWidget::createObjectDescription(UAVObject *object)
{
    QString indent("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
    QString description;
    description.append("<html><head></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">");
    description.append("<b>").append(tr("Name:")).append(" </b>").append(object->getName())
            .append("<br>");
    description.append("<b>").append(tr("Type:")).append(" </b>")
            .append(object->isSettingsObject() ? tr("Settings") : object->isMetaDataObject() ? tr("Metadata") : tr("Data"))
            .append(indent);
    description.append("<b>").append(tr("Category:")).append(" </b>").append(object->getCategory())
            .append("<p>");
    description.append("<b>").append(tr("Description:")).append(" </b>").append(object->getDescription().replace("@ref", ""))
            .append("<hr><br>");
    description.append("<b>").append(tr("Fields:")).append(" </b>")
            .append("<br>");

    foreach (UAVObjectField *field, object->getFields()) {
        description.append(indent).append("<b>").append(tr("Name:")).append(" </b>").append(field->getName())
                .append(indent);
        description.append("<b>").append(tr("Size:")).append(" </b>").append(tr("%1 bytes").arg(field->getNumBytes()))
                .append(indent);

        description.append("<b>").append(tr("Type:")).append(" </b>").append(field->getTypeAsString());
        int elements = field->getNumElements();
        if (elements > 1) {
            description.append("[").append(QString("%1").arg(field->getNumElements())).append("]");
        }

        description.append(indent).append("<b>").append(tr("Unit:")).append(" </b>").append(field->getUnits());

        description.append("<br>");
        if (field->getDescription() != "") {
            description.append(indent).append("<b>").append(tr("Description:")).append(" </b>").append(field->getDescription());
            description.append("<br>");
        }

        if (elements > 1) {
            description.append(indent).append("<b>").append(tr("Elements:")).append(" </b>").append("<br>");
            QStringList names = field->getElementNames();
            for (uint i = 0; i < field->getNumElements(); i++) {
                description.append(indent).append(indent).append("<b>").append(tr("Name:")).append(" </b>").append(names.at(i))
                        .append(indent);
                if (field->getMinLimit(i).toString() != "" && field->getMaxLimit(i).toString() != "") {
                    description.append("<b>").append(tr("Limits:")).append(" </b>")
                            .append(QString("%1 - %2").arg(field->getMinLimit(i).toString(), field->getMaxLimit(i).toString()));
                }
                description.append("<br>");
            }
            description.append("<p>");
        } else {
            if (field->getMinLimit(0).toString() != "" && field->getMaxLimit(0).toString() != "") {
                description.append(indent).append("<b>").append(tr("Limits:")).append(" </b>")
                        .append(QString("%1 - %2").arg(field->getMinLimit(0).toString(), field->getMaxLimit(0).toString()));
                description.append("<p>");
            }
        }
    }

    description.append("</body></html>");
    return description;
}

void UAVObjectBrowserWidget::enableSendRequest(bool enable)
{
    m_browser->sendButton->setEnabled(enable);
    m_browser->requestButton->setEnabled(enable);
    m_browser->saveSDButton->setEnabled(enable);
    m_browser->readSDButton->setEnabled(enable);
    m_browser->eraseSDButton->setEnabled(enable);
}

void UAVObjectBrowserWidget::updateDescription()
{
    ObjectTreeItem *objItem = findCurrentObjectTreeItem();
    if (objItem) {
        UAVObject *obj = objItem->object();
        if (obj) {
            m_browser->descriptionText->setText(createObjectDescription(obj));
            return;
        }
    }
    m_browser->descriptionText->setText("");
}
