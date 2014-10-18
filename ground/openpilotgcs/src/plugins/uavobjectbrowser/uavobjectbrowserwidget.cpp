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
#include "utils/mustache.h"

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
    m_mustacheTemplate = loadFileIntoString(QString(":/uavobjectbrowser/resources/uavodescription.mustache"));
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
    connect(m_browser->splitter, SIGNAL(splitterMoved(int, int)), this, SLOT(splitterMoved()));
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

void UAVObjectBrowserWidget::setSplitterState(QByteArray state)
{
    m_browser->splitter->restoreState(state);
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
    m_model->setUnknowObjectColor(m_unknownObjectColor);
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
    m_model->setUnknowObjectColor(m_unknownObjectColor);
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

QString UAVObjectBrowserWidget::loadFileIntoString(QString fileName)
{
    QFile file(fileName);

    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);
    QString line = stream.readAll();
    file.close();
    return line;
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
    emit splitterChanged(m_browser->splitter->saveState());
}

QString UAVObjectBrowserWidget::createObjectDescription(UAVObject *object)
{
    QString mustache(m_mustacheTemplate);

    QVariantHash uavoHash;

    uavoHash["OBJECT_NAME_TITLE"] = tr("Name");
    uavoHash["OBJECT_NAME"] = object->getName();
    uavoHash["CATEGORY_TITLE"]    = tr("Category");
    uavoHash["CATEGORY"]          = object->getCategory();
    uavoHash["TYPE_TITLE"]        = tr("Type");
    uavoHash["TYPE"] = object->isMetaDataObject() ? tr("Metadata") : object->isSettingsObject() ? tr("Setting") : tr("Data");
    uavoHash["SIZE_TITLE"]        = tr("Size");
    uavoHash["SIZE"] = object->getNumBytes();
    uavoHash["DESCRIPTION_TITLE"] = tr("Description");
    uavoHash["DESCRIPTION"]       = object->getDescription().replace("@ref", "");
    uavoHash["MULTI_INSTANCE_TITLE"] = tr("Multi");
    uavoHash["MULTI_INSTANCE"]    = object->isSingleInstance() ? tr("No") : tr("Yes");
    uavoHash["FIELDS_NAME_TITLE"] = tr("Fields");
    QVariantList fields;
    foreach(UAVObjectField * field, object->getFields()) {
        QVariantHash fieldHash;

        fieldHash["FIELD_NAME_TITLE"] = tr("Name");
        fieldHash["FIELD_NAME"] = field->getName();
        fieldHash["FIELD_TYPE_TITLE"] = tr("Type");
        fieldHash["FIELD_TYPE"] = QString("%1%2").arg(field->getTypeAsString(),
                                                      (field->getNumElements() > 1 ? QString("[%1]").arg(field->getNumElements()) : QString()));
        if (!field->getUnits().isEmpty()) {
            fieldHash["FIELD_UNIT_TITLE"] = tr("Unit");
            fieldHash["FIELD_UNIT"] = field->getUnits();
        }
        if (!field->getOptions().isEmpty()) {
            fieldHash["FIELD_OPTIONS_TITLE"] = tr("Options");
            QVariantList options;
            foreach(QString option, field->getOptions()) {
                QVariantHash optionHash;

                optionHash["FIELD_OPTION"] = option;
                if (!options.isEmpty()) {
                    optionHash["FIELD_OPTION_DELIM"] = ", ";
                }
                options.append(optionHash);
            }
            fieldHash["FIELD_OPTIONS"] = options;
        }
        if (field->getElementNames().count() > 1) {
            fieldHash["FIELD_ELEMENTS_TITLE"] = tr("Elements");
            QVariantList elements;
            for (int i = 0; i < field->getElementNames().count(); i++) {
                QString element = field->getElementNames().at(i);
                QVariantHash elementHash;
                elementHash["FIELD_ELEMENT"] = element;
                QString limitsString = field->getLimitsAsString(i);
                if (!limitsString.isEmpty()) {
                    elementHash["FIELD_ELEMENT_LIMIT"] = limitsString.prepend(" (").append(")");
                }
                if (!elements.isEmpty()) {
                    elementHash["FIELD_ELEMENT_DELIM"] = ", ";
                }
                elements.append(elementHash);
            }
            fieldHash["FIELD_ELEMENTS"] = elements;
        } else if (!field->getLimitsAsString(0).isEmpty()) {
            fieldHash["FIELD_LIMIT_TITLE"] = tr("Limits");
            fieldHash["FIELD_LIMIT"] = field->getLimitsAsString(0);
        }

        if (!field->getDescription().isEmpty()) {
            fieldHash["FIELD_DESCRIPTION_TITLE"] = tr("Description");
            fieldHash["FIELD_DESCRIPTION"] = field->getDescription();
        }

        fields.append(fieldHash);
    }
    uavoHash["FIELDS"] = fields;
    Mustache::QtVariantContext context(uavoHash);
    Mustache::Renderer renderer;
    return renderer.render(mustache, &context);
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
