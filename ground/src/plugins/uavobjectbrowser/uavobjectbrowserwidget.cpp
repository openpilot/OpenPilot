/**
 ******************************************************************************
 *
 * @file       uavobjectbrowserwidget.cpp
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
#include "uavobjectbrowserwidget.h"
#include "uavobjecttreemodel.h"
#include "browseritemdelegate.h"
#include "ui_uavobjectbrowser.h"
#include "uavobjects/uavobjectmanager.h"
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
    m_browser->setupUi(this);
    m_model = new UAVObjectTreeModel();
    m_browser->treeView->setModel(m_model);
    m_browser->treeView->setColumnWidth(0, 300);
    m_browser->treeView->expandAll();
    BrowserItemDelegate *m_delegate = new BrowserItemDelegate();
    m_browser->treeView->setItemDelegate(m_delegate);
    m_browser->treeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_browser->treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    showMetaData(m_browser->metaCheckBox->isChecked());
    connect(m_browser->metaCheckBox, SIGNAL(toggled(bool)), this, SLOT(showMetaData(bool)));
    connect(m_browser->saveSDButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
    connect(m_browser->readSDButton, SIGNAL(clicked()), this, SLOT(readSettings()));
}

UAVObjectBrowserWidget::~UAVObjectBrowserWidget()
{
   delete m_browser;
}

void UAVObjectBrowserWidget::showMetaData(bool show)
{
    int topRowCount = m_model->rowCount(QModelIndex());
    for (int i = 0; i < topRowCount; ++i) {
        QModelIndex index = m_model->index(i, 0, QModelIndex());
        int subRowCount = m_model->rowCount(index);
        for (int j = 0; j < subRowCount; ++j) {
            m_browser->treeView->setRowHidden(0, index.child(j,0), !show);
        }
    }
}

void UAVObjectBrowserWidget::sendUpdate()
{

}

void UAVObjectBrowserWidget::requestUpdate()
{

}

void UAVObjectBrowserWidget::saveSettings()
{
    updateSettings(SettingsPersistence::OPERATION_SAVE);
}

void UAVObjectBrowserWidget::readSettings()
{
    updateSettings(SettingsPersistence::OPERATION_LOAD);
}

void UAVObjectBrowserWidget::updateSettings(SettingsPersistence::OperationEnum op)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    SettingsPersistence* obj = dynamic_cast<SettingsPersistence*>( objManager->getObject(SettingsPersistence::NAME) );
    if (obj != NULL)
    {
        SettingsPersistence::DataFields data;
        data.Operation = op;
        obj->setData(data);
        obj->updated();
    }
}

