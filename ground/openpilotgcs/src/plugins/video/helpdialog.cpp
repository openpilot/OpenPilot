/**
 ******************************************************************************
 *
 * @file       settingsdialog.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup CorePlugin Core Plugin
 * @{
 * @brief The Core GCS plugin
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

#include "helpdialog.h"

#include "gst_global.h"

//#include <extensionsystem/pluginmanager.h>
//#include "icore.h"
//#include "coreplugin/uavgadgetinstancemanager.h"
//#include "coreplugin/uavgadgetoptionspagedecorator.h"
//#include "coreimpl.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>

//namespace {
//    struct PageData {
//        int index;
//        QString category;
//        QString id;
//    };
//}

//Q_DECLARE_METATYPE(::PageData)

//using namespace Core;
//using namespace Core::Internal;

HelpDialog::HelpDialog(QWidget *parent, const QString &elementId)
    : QDialog(parent),
    m_windowWidth(0),
    m_windowHeight(0)
{
    setupUi(this);
    setWindowTitle(tr("GStreamer Help"));
//    if (elementId.isEmpty()) {
//        QSettings *settings = ICore::instance()->settings();
//        initialCategory = settings->value("General/LastPreferenceCategory", QVariant(QString())).toString();
//        initialPage = settings->value("General/LastPreferencePage", QVariant(QString())).toString();
//        qDebug() << "HelpDialog settings initial category: " << initialCategory << ", initial page: " << initialPage;
//        m_windowWidth = settings->value("General/HelpWindowWidth", 0).toInt();
//        m_windowHeight = settings->value("General/HelpWindowHeight", 0).toInt();
//    }
    if (m_windowWidth > 0 && m_windowHeight > 0)
        resize(m_windowWidth, m_windowHeight);

    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    connect(buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close()));
   
//    connect(this, SIGNAL(settingsDialogShown(Core::Internal::HelpDialog*)), m_instanceManager, SLOT(settingsDialogShown(Core::Internal::HelpDialog*)));
//    connect(this, SIGNAL(settingsDialogRemoved()), m_instanceManager, SLOT(settingsDialogRemoved()));

    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
//    pageTree->header()->setVisible(false);
//    pageTree->setIconSize(QSize(24, 24));

    connect(elementListWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
        this, SLOT(pageSelected()));

    QList<QString> plugins = gst::pluginList();

    foreach (QString pluginName, plugins) {
        new QListWidgetItem(pluginName, elementListWidget);
    }
//    foreach (IOptionsPage *page, pages) {
//        PageData pageData;
//        pageData.index = index;
//        pageData.category = page->category();
//        pageData.id = page->id();
//
//        QTreeWidgetItem *item = new QTreeWidgetItem;
//        item->setText(0, page->trName());
//        item->setData(0, Qt::UserRole, qVariantFromValue(pageData));
//
//        QString trCategories = page->trCategory();
//        QString currentCategory = page->category();
//
//        QTreeWidgetItem *categoryItem;
//        if (!categories.contains(currentCategory)) {
//            // Above the first gadget option we insert a separator
//            if (!firstUavGadgetOptionsPageFound) {
//                UAVGadgetOptionsPageDecorator *pd = qobject_cast<UAVGadgetOptionsPageDecorator*>(page);
//                if (pd) {
//                    firstUavGadgetOptionsPageFound = true;
//                    QTreeWidgetItem *separator = new QTreeWidgetItem(pageTree);
//                    separator->setFlags(separator->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
//                    separator->setText(0, QString(30, 0xB7));
//                }
//            }
//            categoryItem = new QTreeWidgetItem(pageTree);
//            categoryItem->setIcon(0, page->icon());
//            categoryItem->setText(0, trCategories);
//            categoryItem->setData(0, Qt::UserRole, qVariantFromValue(pageData));
//            categories.insert(currentCategory, categoryItem);
//        }
//
//        QList<QTreeWidgetItem *> *categoryItemList = m_categoryItemsMap.value(currentCategory);
//        if (!categoryItemList) {
//             categoryItemList = new QList<QTreeWidgetItem *>();
//             m_categoryItemsMap.insert(currentCategory, categoryItemList);
//        }
//        categoryItemList->append(item);
//
//        m_pages.append(page);
//
//        // creating all option pages upfront is slow, so we create place holder widgets instead
//        // the real option page widget will be created later when the user selects it
//        // the place holder is a QLabel and we assume that no option page will be a QLabel...
//        QLabel * placeholderWidget = new QLabel(stackedPages);
//        stackedPages->addWidget(placeholderWidget);
//
//        if (page->id() == initialPage && currentCategory == initialCategory) {
//            initialItem = item;
//        }
//
//        index++;
//    }

//    foreach(QString category, m_categoryItemsMap.keys()) {
//        QList<QTreeWidgetItem *> *categoryItemList = m_categoryItemsMap.value(category);
//        if (categoryItemList->size() > 1) {
//            foreach (QTreeWidgetItem *item, *categoryItemList) {
//                QTreeWidgetItem *categoryItem = categories.value(category);
//                categoryItem->addChild(item);
//            }
//        }
//    }
//
//    if (initialItem) {
//        if (!initialItem->parent()) {
//            // item has no parent, meaning it is single child
//            // so select category item instead as single child are not added to the tree
//            initialItem = categories.value(initialCategory);
//        }
//        pageTree->setCurrentItem(initialItem);
//    }

//    QList<int> sizes;
//    sizes << 150 << 300;
//    splitter->setSizes(sizes);
//
//    splitter->setStretchFactor(splitter->indexOf(pageTree), 0);
//    splitter->setStretchFactor(splitter->indexOf(layoutWidget), 1);
}

HelpDialog::~HelpDialog()
{
//    foreach(QString category, m_categoryItemsMap.keys()) {
//        QList<QTreeWidgetItem *> *categoryItemList = m_categoryItemsMap.value(category);
//        delete categoryItemList;
//    }
}

void HelpDialog::itemSelected()
{
//    QTreeWidgetItem *item = pageTree->currentItem();
//    if (!item)
//        return;
//
//    PageData data = item->data(0, Qt::UserRole).value<PageData>();
//    int index = data.index;
}

//void HelpDialog::categoryItemSelectedShowChildInstead()
//{
//    QTreeWidgetItem *item = pageTree->currentItem();
//    item->setExpanded(true);
//    pageTree->setCurrentItem(item->child(0), 0, QItemSelectionModel::SelectCurrent);
//}

//void HelpDialog::disableApplyOk(bool disable)
//{
//    buttonBox->button(QDialogButtonBox::Apply)->setDisabled(disable);
//    buttonBox->button(QDialogButtonBox::Ok)->setDisabled(disable);
//}

void HelpDialog::close()
{
}

bool HelpDialog::execDialog()
{
//    m_applied = false;
//    emit settingsDialogShown(this);
    exec();
//    emit settingsDialogRemoved();
//    return m_applied;
    return true;
}

//void HelpDialog::done(int val)
//{
//    QHelp *settings = ICore::instance()->settings();
//    settings->setValue("General/LastPreferenceCategory", m_currentCategory);
//    settings->setValue("General/LastPreferencePage", m_currentPage);
//    settings->setValue("General/HelpWindowWidth", this->width());
//    settings->setValue("General/HelpWindowHeight", this->height());
//    QDialog::done(val);
//}
