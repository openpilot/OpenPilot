/**
 ******************************************************************************
 *
 * @file       uavgadgetinstancemanager.h
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

#ifndef UAVGADGETINSTANCEMANAGER_H
#define UAVGADGETINSTANCEMANAGER_H

#include <QObject>
#include <QSettings>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtGui/QIcon>
#include "core_global.h"
#include "uavconfiginfo.h"

namespace ExtensionSystem {
    class PluginManager;
}

namespace Core
{

namespace Internal {
class SettingsDialog;
}

class IUAVGadget;
class IUAVGadgetConfiguration;
class IOptionsPage;
class IUAVGadgetFactory;

class CORE_EXPORT UAVGadgetInstanceManager : public QObject
{

Q_OBJECT
public:
    explicit UAVGadgetInstanceManager(QObject *parent = 0);
    ~UAVGadgetInstanceManager();
    void readSettings(QSettings *qs);
    void saveSettings(QSettings *qs);
    IUAVGadget *createGadget(QString classId, QWidget *parent);
    void removeGadget(IUAVGadget *gadget);
    void removeAllGadgets();
    bool canDeleteConfiguration(IUAVGadgetConfiguration *config);
    void deleteConfiguration(IUAVGadgetConfiguration *config);
    void cloneConfiguration(IUAVGadgetConfiguration *config);
    void applyChanges(IUAVGadgetConfiguration *config);
    void configurationNameEdited(QString text, bool hasText = true);
    QStringList classIds() const { return m_classIdNameMap.keys(); }
    QStringList configurationNames(QString classId) const;
    QString gadgetName(QString classId) const;
    QIcon gadgetIcon(QString classId) const;

signals:
    void configurationChanged(IUAVGadgetConfiguration* config);
    void configurationAdded(IUAVGadgetConfiguration* config);
    void configurationToBeDeleted(IUAVGadgetConfiguration* config);
    void configurationNameChanged(IUAVGadgetConfiguration* config, QString oldName, QString newName);

public slots:
    void settingsDialogShown(Core::Internal::SettingsDialog* settingsDialog);
    void settingsDialogRemoved();

private:
    IUAVGadgetFactory *factory(QString classId) const;
    void createOptionsPages();
    QList<IUAVGadgetConfiguration*> *configurations(QString classId) const;
    QString suggestName(QString classId, QString name);
    QList<IUAVGadget*> m_gadgetInstances;
    QList<IUAVGadgetFactory*> m_factories;
    QList<IUAVGadgetConfiguration*> m_configurations;
    QList<IOptionsPage*> m_optionsPages;
    QMap<QString, QString> m_classIdNameMap;
    QMap<QString, QIcon> m_classIdIconMap;
    QMap<QString, QStringList> m_takenNames;
    QList<IUAVGadgetConfiguration*> m_provisionalConfigs;
    QList<IUAVGadgetConfiguration*> m_provisionalDeletes;
    QList<IOptionsPage*> m_provisionalOptionsPages;
    Core::Internal::SettingsDialog *m_settingsDialog;
    ExtensionSystem::PluginManager *m_pm;
    int indexForConfig(QList<IUAVGadgetConfiguration*> configurations,
                       QString classId, QString configName);
    void readConfigs_1_1_0(QSettings *qs);
    void readConfigs_1_2_0(QSettings *qs);
};

} // namespace Core

#endif // UAVGADGETINSTANCEMANAGER_H
