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

#include "settingsdialog.h"

#include <extensionsystem/pluginmanager.h>
#include "icore.h"
#include "coreplugin/uavgadgetinstancemanager.h"
#include "coreplugin/uavgadgetoptionspagedecorator.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>

using namespace Core;
using namespace Core::Internal;

namespace {
struct PageData {
    int     index;
    QString category;
    QString id;
};

// helper to sort by translated category and name
bool compareOptionsPageByCategoryAndNameTr(const IOptionsPage *p1, const IOptionsPage *p2)
{
    const UAVGadgetOptionsPageDecorator *gp1 = qobject_cast<const UAVGadgetOptionsPageDecorator *>(p1);
    const UAVGadgetOptionsPageDecorator *gp2 = qobject_cast<const UAVGadgetOptionsPageDecorator *>(p2);

    if (gp1 && !gp2) {
        return false;
    }
    if (gp2 && !gp1) {
        return true;
    }
    if (const int cc = QString::localeAwareCompare(p1->trCategory(), p2->trCategory())) {
        return cc < 0;
    }
    return QString::localeAwareCompare(p1->trName(), p2->trName()) < 0;
}

// helper to sort by category and id
bool compareOptionsPageByCategoryAndId(const IOptionsPage *p1, const IOptionsPage *p2)
{
    const UAVGadgetOptionsPageDecorator *gp1 = qobject_cast<const UAVGadgetOptionsPageDecorator *>(p1);
    const UAVGadgetOptionsPageDecorator *gp2 = qobject_cast<const UAVGadgetOptionsPageDecorator *>(p2);

    if (gp1 && !gp2) {
        return false;
    }
    if (gp2 && !gp1) {
        return true;
    }
    if (const int cc = QString::localeAwareCompare(p1->category(), p2->category())) {
        return cc < 0;
    }
    return QString::localeAwareCompare(p1->id(), p2->id()) < 0;
}
}

Q_DECLARE_METATYPE(::PageData) SettingsDialog::SettingsDialog(QWidget *parent, const QString &categoryId, const QString &pageId) :
    QDialog(parent), m_applied(false)
{
    setupUi(this);
#ifdef Q_OS_MAC
    setWindowTitle(tr("Preferences"));
#else
    setWindowTitle(tr("Options"));
#endif

    QSettings *settings = ICore::instance()->settings();

    settings->beginGroup("General");
    settings->beginGroup("Settings");

    // restore last displayed category and page
    // this is done only if no category or page was provided through the constructor
    QString initialCategory = categoryId;
    QString initialPage     = pageId;
    qDebug() << "SettingsDialog constructor initial category:" << initialCategory << ", initial page:" << initialPage;
    if (initialCategory.isEmpty() && initialPage.isEmpty()) {
        initialCategory = settings->value("LastPreferenceCategory", QVariant(QString())).toString();
        initialPage     = settings->value("LastPreferencePage", QVariant(QString())).toString();
        qDebug() << "SettingsDialog settings initial category:" << initialCategory << ", initial page: " << initialPage;
    }

    // restore window size
    int windowWidth  = settings->value("WindowWidth", 0).toInt();
    int windowHeight = settings->value("WindowHeight", 0).toInt();
    qDebug() << "SettingsDialog window width :" << windowWidth << ", height:" << windowHeight;
    if (windowWidth > 0 && windowHeight > 0) {
        resize(windowWidth, windowHeight);
    }

    // restore splitter size
    int splitterPosition = settings->value("SplitterPosition", 350).toInt();
    qDebug() << "SettingsDialog splitter position:" << splitterPosition;
    QList<int> sizes;
    sizes << splitterPosition << 400;
    splitter->setSizes(sizes);

    settings->endGroup();
    settings->endGroup();

    // all extra space must go to the option page and none to the tree
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));

    m_instanceManager = Core::ICore::instance()->uavGadgetInstanceManager();

    connect(this, SIGNAL(settingsDialogShown(Core::Internal::SettingsDialog *)), m_instanceManager,
            SLOT(settingsDialogShown(Core::Internal::SettingsDialog *)));
    connect(this, SIGNAL(settingsDialogRemoved()), m_instanceManager, SLOT(settingsDialogRemoved()));

    // needs to be queued to be able to change the selection from the selection change signal call
    connect(this, SIGNAL(categoryItemSelected()), this, SLOT(onCategorySelected()), Qt::QueuedConnection);

    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
    pageTree->header()->setVisible(false);

    connect(pageTree, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(onItemSelected()));

    QList<Core::IOptionsPage *> pluginPages;
    QList<Core::IOptionsPage *> gadgetPages;

    // get all pages and split them between plugin and gadget list
    QList<Core::IOptionsPage *> pages = ExtensionSystem::PluginManager::instance()->getObjects<IOptionsPage>();
    foreach(IOptionsPage * page, pages) {
        if (qobject_cast<UAVGadgetOptionsPageDecorator *>(page)) {
            gadgetPages.append(page);
        } else {
            pluginPages.append(page);
        }
    }

    // the plugin options page list is sorted by untranslated category and names
    // this is done to facilitate access to the language settings when GCS is not running in a language understood by the user.
    qStableSort(pluginPages.begin(), pluginPages.end(), compareOptionsPageByCategoryAndId);

    // the plugin options page list is sorted by translated names
    qStableSort(gadgetPages.begin(), gadgetPages.end(), compareOptionsPageByCategoryAndNameTr);

    // will hold the initially selected item if any
    QTreeWidgetItem *initialItem = 0;

    // add plugin pages
    foreach(IOptionsPage * page, pluginPages) {
        QTreeWidgetItem *item = addPage(page);

        // automatically expand all plugin categories
        item->parent()->setExpanded(true);
        if (page->id() == initialPage && page->category() == initialCategory) {
            initialItem = item;
        }
    }

    // insert separator between plugin and gadget pages
    QTreeWidgetItem *separator = new QTreeWidgetItem(pageTree);
    separator->setFlags(separator->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
    separator->setText(0, QString(30, 0xB7));

    // add gadget pages
    foreach(IOptionsPage * page, gadgetPages) {
        QTreeWidgetItem *item = addPage(page);

        if (page->id() == initialPage && page->category() == initialCategory) {
            initialItem = item;
        }
    }

    // handle initially selected item
    if (initialItem) {
        if (initialItem->isHidden()) {
            // item is hidden, meaning it is single child
            // so select parent category item instead
            initialItem = initialItem->parent();
        }
        pageTree->setCurrentItem(initialItem);
    }
}

SettingsDialog::~SettingsDialog()
{
    foreach(QString category, m_categoryItemsMap.keys()) {
        QList<QTreeWidgetItem *> *categoryItemList = m_categoryItemsMap.value(category);
        delete categoryItemList;
    }
    // delete place holders
    for (int i = 0; i < stackedPages->count(); i++) {
        QLabel *widget = dynamic_cast<QLabel *>(stackedPages->widget(i));
        if (widget) {
            delete widget;
        }
    }
}

QTreeWidgetItem *SettingsDialog::addPage(IOptionsPage *page)
{
    PageData pageData;

    pageData.index    = m_pages.count();
    pageData.category = page->category();
    pageData.id = page->id();

    QString category = page->category();

    QList<QTreeWidgetItem *> *categoryItemList = m_categoryItemsMap.value(category);
    if (!categoryItemList) {
        categoryItemList = new QList<QTreeWidgetItem *>();
        m_categoryItemsMap.insert(category, categoryItemList);
    }

    QTreeWidgetItem *categoryItem = NULL;
    for (int i = 0; i < pageTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *tw = pageTree->topLevelItem(i);
        PageData data = tw->data(0, Qt::UserRole).value<PageData>();
        if (data.category == page->category()) {
            categoryItem = tw;
            break;
        }
    }
    if (!categoryItem) {
        categoryItem = new QTreeWidgetItem(pageTree);
        categoryItem->setIcon(0, page->icon());
        categoryItem->setText(0, page->trCategory());
        categoryItem->setData(0, Qt::UserRole, qVariantFromValue(pageData));
    }

    QTreeWidgetItem *item = new QTreeWidgetItem(categoryItem);
    item->setText(0, page->trName());
    item->setData(0, Qt::UserRole, qVariantFromValue(pageData));

    switch (categoryItemList->size()) {
    case 0:
        item->setHidden(true);
        break;
    case 1:
        categoryItemList->at(0)->setHidden(false);
        break;
    default:
        break;
    }

    categoryItemList->append(item);

    m_pages.append(page);

    // creating all option pages upfront is slow, so we create place holder widgets instead
    // the real option page widget will be created later when the user selects it
    // the place holder is a QLabel and we assume that no option page will be a QLabel...
    QLabel *placeholderWidget = new QLabel(stackedPages);
    stackedPages->addWidget(placeholderWidget);

    return item;
}

void SettingsDialog::onItemSelected()
{
    QTreeWidgetItem *item = pageTree->currentItem();

    if (!item) {
        return;
    }

    if (pageTree->indexOfTopLevelItem(item) >= 0) {
        if (item->childCount() == 1) {
            // single child : category will not be expanded
            item = item->child(0);
        } else if (item->childCount() > 1) {
            // multiple children : expand category and select 1st child
            emit categoryItemSelected();
            return;
        }
    }

    // get user data
    PageData data = item->data(0, Qt::UserRole).value<PageData>();
    int index     = data.index;
    m_currentCategory = data.category;
    m_currentPage     = data.id;

    // check if we are looking at a place holder or not
    QWidget *widget = dynamic_cast<QLabel *>(stackedPages->widget(index));
    if (widget) {
        // place holder found, get rid of it...
        stackedPages->removeWidget(widget);
        delete widget;
        // and replace place holder with actual option page
        IOptionsPage *page = m_pages.at(index);
        stackedPages->insertWidget(index, page->createPage(stackedPages));
    }

    IOptionsPage *page = m_pages.at(index);
    page->updateState();

    stackedPages->setCurrentIndex(index);
}

void SettingsDialog::onCategorySelected()
{
    QTreeWidgetItem *item = pageTree->currentItem();

    if (item->childCount() > 1) {
        item->setExpanded(true);
        pageTree->setCurrentItem(item->child(0), 0, QItemSelectionModel::SelectCurrent);
    }
}

void SettingsDialog::deletePage()
{
    QTreeWidgetItem *item = pageTree->currentItem();

    PageData data    = item->data(0, Qt::UserRole).value<PageData>();
    QString category = data.category;

    QList<QTreeWidgetItem *> *categoryItemList = m_categoryItemsMap.value(category);
    if (categoryItemList) {
        categoryItemList->removeOne(item);
        QTreeWidgetItem *parentItem = item->parent();
        parentItem->removeChild(item);
        if (parentItem->childCount() == 1) {
            parentItem->child(0)->setHidden(true);
            pageTree->setCurrentItem(parentItem, 0, QItemSelectionModel::SelectCurrent);
        }
    }
}

// TODO duplicates a lot of the addPage code...
void SettingsDialog::insertPage(IOptionsPage *page)
{
    PageData pageData;

    pageData.index    = m_pages.count();
    pageData.category = page->category();
    pageData.id = page->id();

    QTreeWidgetItem *categoryItem = 0;
    for (int i = 0; i < pageTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *tw = pageTree->topLevelItem(i);
        PageData data = tw->data(0, Qt::UserRole).value<PageData>();
        if (data.category == page->category()) {
            categoryItem = tw;
            break;
        }
    }
    if (!categoryItem) {
        return;
    }

    // If this category has no child right now
    // we need to add the "default child"
    QList<QTreeWidgetItem *> *categoryItemList = m_categoryItemsMap.value(page->category());
    if (categoryItem->childCount() == 1) {
        QTreeWidgetItem *defaultItem = categoryItemList->at(0);
        defaultItem->setHidden(false);
    }

    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, page->trName());
    item->setData(0, Qt::UserRole, qVariantFromValue(pageData));

    categoryItem->addChild(item);
    categoryItemList->append(item);

    m_pages.append(page);
    stackedPages->addWidget(page->createPage(stackedPages));

    stackedPages->setCurrentIndex(stackedPages->count());
    pageTree->setCurrentItem(item);
}

void SettingsDialog::updateText(QString text)
{
    QTreeWidgetItem *item = pageTree->currentItem();

    item->setText(0, text);
}

void SettingsDialog::disableApplyOk(bool disable)
{
    buttonBox->button(QDialogButtonBox::Apply)->setDisabled(disable);
    buttonBox->button(QDialogButtonBox::Ok)->setDisabled(disable);
}

void SettingsDialog::accept()
{
    m_applied = true;
    for (int i = 0; i < m_pages.size(); i++) {
        QWidget *widget = dynamic_cast<QLabel *>(stackedPages->widget(i));
        if (!widget) {
            IOptionsPage *page = m_pages.at(i);
            page->apply();
            page->finish();
        }
    }
    done(QDialog::Accepted);
}

void SettingsDialog::reject()
{
    for (int i = 0; i < m_pages.size(); i++) {
        QWidget *widget = dynamic_cast<QLabel *>(stackedPages->widget(i));
        if (!widget) {
            IOptionsPage *page = m_pages.at(i);
            page->finish();
        }
    }
    done(QDialog::Rejected);
}

void SettingsDialog::apply()
{
    for (int i = 0; i < m_pages.size(); i++) {
        QWidget *widget = dynamic_cast<QLabel *>(stackedPages->widget(i));
        if (!widget) {
            IOptionsPage *page = m_pages.at(i);
            page->apply();
        }
    }
    m_applied = true;
}

bool SettingsDialog::execDialog()
{
    m_applied = false;
    emit settingsDialogShown(this);
    exec();
    emit settingsDialogRemoved();
    return m_applied;
}

void SettingsDialog::done(int val)
{
    QSettings *settings = ICore::instance()->settings();

    settings->beginGroup("General");
    settings->beginGroup("Settings");

    settings->setValue("LastPreferenceCategory", m_currentCategory);
    settings->setValue("LastPreferencePage", m_currentPage);

    settings->setValue("WindowWidth", this->width());
    settings->setValue("WindowHeight", this->height());

    QList<int> sizes = splitter->sizes();
    settings->setValue("SplitterPosition", sizes[0]);

    settings->endGroup();
    settings->endGroup();

    QDialog::done(val);
}
