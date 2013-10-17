/**
 ******************************************************************************
 *
 * @file       workspacesettings.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#include "workspacesettings.h"
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uavgadgetmanager/uavgadgetmanager.h>
#include <QtCore/QSettings>

#include "ui_workspacesettings.h"

using namespace Core;
using namespace Core::Internal;

const int WorkspaceSettings::MAX_WORKSPACES = 10;

WorkspaceSettings::WorkspaceSettings(QObject *parent) :
    IOptionsPage(parent)
{}

WorkspaceSettings::~WorkspaceSettings()
{}

// IOptionsPage

QString WorkspaceSettings::id() const
{
    return QLatin1String("Workspaces");
}

QString WorkspaceSettings::trName() const
{
    return tr("Workspaces");
}

QString WorkspaceSettings::category() const
{
    return QLatin1String("Environment");
}

QString WorkspaceSettings::trCategory() const
{
    return tr("Environment");
}

QWidget *WorkspaceSettings::createPage(QWidget *parent)
{
    m_page = new Ui::WorkspaceSettings();
    QWidget *w = new QWidget(parent);
    m_page->setupUi(w);

    m_page->numberOfWorkspacesSpinBox->setMaximum(MAX_WORKSPACES);
    m_page->numberOfWorkspacesSpinBox->setValue(m_numberOfWorkspaces);
    for (int i = 0; i < m_numberOfWorkspaces; ++i) {
        m_page->workspaceComboBox->addItem(QIcon(m_iconNames.at(i)), m_names.at(i));
    }

    m_page->iconPathChooser->setExpectedKind(Utils::PathChooser::File);
    m_page->iconPathChooser->setPromptDialogFilter(tr("Images (*.png *.jpg *.bmp *.xpm)"));
    m_page->iconPathChooser->setPromptDialogTitle(tr("Choose icon"));

    connect(m_page->workspaceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectWorkspace(int)));
    connect(m_page->numberOfWorkspacesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(numberOfWorkspacesChanged(int)));
    connect(m_page->nameEdit, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)));
    connect(m_page->iconPathChooser, SIGNAL(browsingFinished()), this, SLOT(iconChanged()));

    m_currentIndex = 0;
    selectWorkspace(m_currentIndex);

    if (0 <= m_tabBarPlacementIndex && m_tabBarPlacementIndex < m_page->comboBoxTabBarPlacement->count()) {
        m_page->comboBoxTabBarPlacement->setCurrentIndex(m_tabBarPlacementIndex);
    }
    m_page->checkBoxAllowTabMovement->setChecked(m_allowTabBarMovement);
    m_page->checkBoxRestoreSelectedOnStartup->setChecked(m_restoreSelectedOnStartup);

    return w;
}

void WorkspaceSettings::readSettings(QSettings *qs)
{
    m_names.clear();
    m_iconNames.clear();
    m_modeNames.clear();

    qs->beginGroup(QLatin1String("Workspace"));
    m_numberOfWorkspaces = qs->value(QLatin1String("NumberOfWorkspaces"), 2).toInt();
    m_previousNumberOfWorkspaces = m_numberOfWorkspaces;
    for (int i = 1; i <= MAX_WORKSPACES; ++i) {
        QString numberString    = QString::number(i);
        QString defaultName     = "Workspace" + numberString;
        QString defaultIconName = "Icon" + numberString;
        QString name     = qs->value(defaultName, defaultName).toString();
        QString iconName = qs->value(defaultIconName, ":/core/images/openpilot_logo_64.png").toString();
        m_names.append(name);
        m_iconNames.append(iconName);
        m_modeNames.append(QString("Mode") + QString::number(i));
    }
    m_tabBarPlacementIndex     = qs->value(QLatin1String("TabBarPlacementIndex"), 1).toInt(); // 1 == "Bottom"
    m_allowTabBarMovement      = qs->value(QLatin1String("AllowTabBarMovement"), false).toBool();
    m_restoreSelectedOnStartup = qs->value(QLatin1String("RestoreSelectedOnStartup"), false).toBool();

    qs->endGroup();

    QTabWidget::TabPosition pos = m_tabBarPlacementIndex == 0 ? QTabWidget::North : QTabWidget::South;
    emit tabBarSettingsApplied(pos, m_allowTabBarMovement);
}

void WorkspaceSettings::saveSettings(QSettings *qs)
{
    qs->beginGroup(QLatin1String("Workspace"));
    qs->setValue(QLatin1String("NumberOfWorkspaces"), m_numberOfWorkspaces);
    for (int i = 0; i < MAX_WORKSPACES; ++i) {
        QString mode = QString("Mode") + QString::number(i + 1);
        int j = m_modeNames.indexOf(mode);
        QString numberString    = QString::number(i + 1);
        QString defaultName     = "Workspace" + numberString;
        QString defaultIconName = "Icon" + numberString;
        qs->setValue(defaultName, m_names.at(j));
        qs->setValue(defaultIconName, m_iconNames.at(j));
    }
    qs->setValue(QLatin1String("TabBarPlacementIndex"), m_tabBarPlacementIndex);
    qs->setValue(QLatin1String("AllowTabBarMovement"), m_allowTabBarMovement);
    qs->setValue(QLatin1String("RestoreSelectedOnStartup"), m_restoreSelectedOnStartup);
    qs->endGroup();
}

void WorkspaceSettings::apply()
{
    selectWorkspace(m_currentIndex, true);

    saveSettings(Core::ICore::instance()->settings());

    if (m_numberOfWorkspaces != m_previousNumberOfWorkspaces) {
        Core::ICore::instance()->readMainSettings(Core::ICore::instance()->settings(), true);
        m_previousNumberOfWorkspaces = m_numberOfWorkspaces;
    }

    ModeManager *modeManager = Core::ICore::instance()->modeManager();
    for (int i = 0; i < MAX_WORKSPACES; ++i) {
        IMode *baseMode = modeManager->mode(modeName(i));
        Core::UAVGadgetManager *mode = qobject_cast<Core::UAVGadgetManager *>(baseMode);
        if (mode) {
            modeManager->updateModeNameIcon(mode, QIcon(iconName(i)), name(i));
        }
    }
    m_tabBarPlacementIndex     = m_page->comboBoxTabBarPlacement->currentIndex();
    m_allowTabBarMovement      = m_page->checkBoxAllowTabMovement->isChecked();
    m_restoreSelectedOnStartup = m_page->checkBoxRestoreSelectedOnStartup->isChecked();

    QTabWidget::TabPosition pos = m_tabBarPlacementIndex == 0 ? QTabWidget::North : QTabWidget::South;
    emit tabBarSettingsApplied(pos, m_allowTabBarMovement);
}

void WorkspaceSettings::finish()
{
    delete m_page;
}

void WorkspaceSettings::textEdited(QString name)
{
    Q_UNUSED(name);
    m_page->workspaceComboBox->setItemText(m_currentIndex, m_page->nameEdit->text());
}

void WorkspaceSettings::iconChanged()
{
    QString iconName = m_page->iconPathChooser->path();

    m_page->workspaceComboBox->setItemIcon(m_currentIndex, QIcon(iconName));
}

void WorkspaceSettings::numberOfWorkspacesChanged(int value)
{
    m_numberOfWorkspaces = value;
    int count = m_page->workspaceComboBox->count();
    if (value > count) {
        for (int i = count; i < value; ++i) {
            m_page->workspaceComboBox->addItem(QIcon(m_iconNames.at(i)), m_names.at(i));
        }
    } else if (value < count) {
        for (int i = count - 1; i >= value; --i) {
            m_page->workspaceComboBox->removeItem(i);
        }
    }
}

void WorkspaceSettings::selectWorkspace(int index, bool store)
{
    if (store || (index != m_currentIndex)) {
        // write old values of workspace not shown anymore
        m_iconNames.replace(m_currentIndex, m_page->iconPathChooser->path());
        m_names.replace(m_currentIndex, m_page->nameEdit->text());
        m_page->workspaceComboBox->setItemIcon(m_currentIndex, QIcon(m_iconNames.at(m_currentIndex)));
        m_page->workspaceComboBox->setItemText(m_currentIndex, m_names.at(m_currentIndex));
    }

    // display current workspace
    QString iconName = m_iconNames.at(index);
    m_page->iconPathChooser->setPath(iconName);
    m_page->nameEdit->setText(m_names.at(index));
    m_currentIndex = index;
}

void WorkspaceSettings::newModeOrder(QVector<IMode *> modes)
{
    QList<int> priorities;
    QStringList modeNames;
    for (int i = 0; i < modes.count(); ++i) {
        Core::UAVGadgetManager *mode = qobject_cast<Core::UAVGadgetManager *>(modes.at(i));
        if (mode) {
            priorities.append(mode->priority());
            modeNames.append(mode->uniqueModeName());
        }
    }
    // Bubble sort
    bool swapped = false;
    do {
        swapped = false;
        for (int i = 0; i < m_names.count() - 1; ++i) {
            int j = i + 1;
            int p = modeNames.indexOf(m_modeNames.at(i));
            int q = modeNames.indexOf(m_modeNames.at(j));
            bool nonShowingMode = (p == -1 && q >= 0);
            bool pqBothFound    = (p >= 0 && q >= 0);
            if (nonShowingMode || (pqBothFound && (priorities.at(q) > priorities.at(p)))) {
                m_names.swap(i, j);
                m_iconNames.swap(i, j);
                m_modeNames.swap(i, j);
                swapped = true;
            }
        }
    } while (swapped);
}
