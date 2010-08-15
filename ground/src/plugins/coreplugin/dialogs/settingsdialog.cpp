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
//#include "coreimpl.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>

namespace {
    struct PageData {
        int index;
        QString category;
        QString id;
    };
}

Q_DECLARE_METATYPE(::PageData)

using namespace Core;
using namespace Core::Internal;

// Helpers to sort by category. id
bool optionsPageLessThan(const IOptionsPage *p1, const IOptionsPage *p2)
{
    const UAVGadgetOptionsPageDecorator *gp1 = qobject_cast<const UAVGadgetOptionsPageDecorator*>(p1);
    const UAVGadgetOptionsPageDecorator *gp2 = qobject_cast<const UAVGadgetOptionsPageDecorator*>(p2);
    if (gp1 && (gp2 == NULL))
        return false;
    if (gp2 && (gp1 == NULL))
        return true;
    if (const int cc = QString::localeAwareCompare(p1->trCategory(), p2->trCategory()))
        return cc < 0;
    return QString::localeAwareCompare(p1->trName(), p2->trName()) < 0;
}

static inline QList<Core::IOptionsPage*> sortedOptionsPages()
{
    QList<Core::IOptionsPage*> rc = ExtensionSystem::PluginManager::instance()->getObjects<IOptionsPage>();
    qStableSort(rc.begin(), rc.end(), optionsPageLessThan);
    return rc;
}

SettingsDialog::SettingsDialog(QWidget *parent, const QString &categoryId,
                               const QString &pageId)
    : QDialog(parent),
    m_applied(false),
    m_windowWidth(0),
    m_windowHeight(0)
{
    setupUi(this);
#ifdef Q_OS_MAC
    setWindowTitle(tr("Preferences"));
#else
    setWindowTitle(tr("Options"));
#endif
    QString initialCategory = categoryId;
    QString initialPage = pageId;
    if (initialCategory.isEmpty() && initialPage.isEmpty()) {
        QSettings *settings = ICore::instance()->settings();
        initialCategory = settings->value("General/LastPreferenceCategory", QVariant(QString())).toString();
        initialPage = settings->value("General/LastPreferencePage", QVariant(QString())).toString();
        m_windowWidth = settings->value("General/SettingsWindowWidth", 0).toInt();
        m_windowHeight = settings->value("General/SettingsWindowHeight", 0).toInt();
    }
    if (m_windowWidth > 0 && m_windowHeight > 0)
        resize(m_windowWidth, m_windowHeight);
    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));
   
    m_instanceManager = Core::ICore::instance()->uavGadgetInstanceManager();
    
    connect(this, SIGNAL(settingsDialogShown(Core::Internal::SettingsDialog*)), m_instanceManager, SLOT(settingsDialogShown(Core::Internal::SettingsDialog*)));
    connect(this, SIGNAL(settingsDialogRemoved()), m_instanceManager, SLOT(settingsDialogRemoved()));

    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
    pageTree->header()->setVisible(false);

    connect(pageTree, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
        this, SLOT(pageSelected()));

    QMap<QString, QTreeWidgetItem *> categories;

    QList<IOptionsPage*> pages = sortedOptionsPages();

    int index = 0;
    bool firstUavGadgetOptionsPageFound = false;
    foreach (IOptionsPage *page, pages) {
        PageData pageData;
        pageData.index = index;
        pageData.category = page->category();
        pageData.id = page->id();

        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, page->trName());
        item->setData(0, Qt::UserRole, qVariantFromValue(pageData));

        QStringList categoriesId = page->category().split(QLatin1Char('|'));
        QStringList trCategories = page->trCategory().split(QLatin1Char('|'));
        QString currentCategory = categoriesId.at(0);

        QTreeWidgetItem *treeitem;
        if (!categories.contains(currentCategory)) {
            if (!firstUavGadgetOptionsPageFound)
            {
                UAVGadgetOptionsPageDecorator *pd = qobject_cast<UAVGadgetOptionsPageDecorator*>(page);
                if (pd)
                {
                    firstUavGadgetOptionsPageFound = true;
                    QTreeWidgetItem *separator = new QTreeWidgetItem(pageTree);
                    separator->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
                    separator->setText(0, QString(30, 0xB7));
                }
            }
            treeitem = new QTreeWidgetItem(pageTree);
            treeitem->setText(0, trCategories.at(0));
            treeitem->setData(0, Qt::UserRole, qVariantFromValue(pageData));
            categories.insert(currentCategory, treeitem);
        }

        int catCount = 1;
        while (catCount < categoriesId.count()) {
            if (!categories.contains(currentCategory + QLatin1Char('|') + categoriesId.at(catCount))) {
                treeitem = new QTreeWidgetItem(categories.value(currentCategory));
                currentCategory +=  QLatin1Char('|') + categoriesId.at(catCount);
                treeitem->setText(0, trCategories.at(catCount));
                treeitem->setData(0, Qt::UserRole, qVariantFromValue(pageData));
                categories.insert(currentCategory, treeitem);
            } else {
                currentCategory +=  QLatin1Char('|') + categoriesId.at(catCount);
            }
            ++catCount;
        }

        categories.value(currentCategory)->addChild(item);

        m_pages.append(page);
        stackedPages->addWidget(page->createPage(stackedPages));

        if (page->id() == initialPage && currentCategory == initialCategory) {
            stackedPages->setCurrentIndex(stackedPages->count());
            pageTree->setCurrentItem(item);
        }

        index++;
    }

    QList<int> sizes;
    sizes << 150 << 300;
    splitter->setSizes(sizes);

    splitter->setStretchFactor(splitter->indexOf(pageTree), 0);
    splitter->setStretchFactor(splitter->indexOf(layoutWidget), 1);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::pageSelected()
{
    QTreeWidgetItem *item = pageTree->currentItem();
    PageData data = item->data(0, Qt::UserRole).value<PageData>();
    int index = data.index;
    m_currentCategory = data.category;
    m_currentPage = data.id;
    stackedPages->setCurrentIndex(index);
}

void SettingsDialog::deletePage()
{
    QTreeWidgetItem *item = pageTree->currentItem();
    item->parent()->removeChild(item);
    pageSelected();
}

void SettingsDialog::insertPage(IOptionsPage* page)
{
    PageData pageData;
    pageData.index = m_pages.count();
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
    if (!categoryItem)
        return;

    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, page->trName());
    item->setData(0, Qt::UserRole, qVariantFromValue(pageData));

    categoryItem->addChild(item);

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
    foreach (IOptionsPage *page, m_pages) {
            page->apply();
        page->finish();
    }
    done(QDialog::Accepted);
}

void SettingsDialog::reject()
{
    foreach (IOptionsPage *page, m_pages)
        page->finish();
    done(QDialog::Rejected);
}

void SettingsDialog::apply()
{
    foreach (IOptionsPage *page, m_pages) {
            page->apply();
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
    settings->setValue("General/LastPreferenceCategory", m_currentCategory);
    settings->setValue("General/LastPreferencePage", m_currentPage);
    settings->setValue("General/SettingsWindowWidth", this->width());
    settings->setValue("General/SettingsWindowHeight", this->height());
    QDialog::done(val);
}
