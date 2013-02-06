/**
 ******************************************************************************
 *
 * @file       uavgadgetinstancemanager.cpp
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

#include "uavgadgetinstancemanager.h"
#include "iuavgadget.h"
#include "uavgadgetdecorator.h"
#include "iuavgadgetfactory.h"
#include "iuavgadgetconfiguration.h"
#include "uavgadgetoptionspagedecorator.h"
#include "coreplugin/dialogs/ioptionspage.h"
#include "coreplugin/dialogs/settingsdialog.h"
#include "icore.h"

#include <extensionsystem/pluginmanager.h>
#include <QtCore/QStringList>
#include <QtCore/QSettings>
#include <QtCore/QDebug>
#include <QtGui/QMessageBox>


using namespace Core;

static const UAVConfigVersion m_versionUAVGadgetConfigurations = UAVConfigVersion("1.2.0");

UAVGadgetInstanceManager::UAVGadgetInstanceManager(QObject *parent) :
    QObject(parent)
{
    m_pm = ExtensionSystem::PluginManager::instance();
    QList<IUAVGadgetFactory*> factories = m_pm->getObjects<IUAVGadgetFactory>();
    foreach (IUAVGadgetFactory *f, factories) {
        if (!m_factories.contains(f)) {
            m_factories.append(f);
            QString classId = f->classId();
            QString name = f->name();
            QIcon icon = f->icon();
            m_classIdNameMap.insert(classId, name);
            m_classIdIconMap.insert(classId, icon);
        }
    }
}

UAVGadgetInstanceManager::~UAVGadgetInstanceManager()
{
    foreach (IOptionsPage *page, m_optionsPages) {
        m_pm->removeObject(page);
        delete page;
    }

}

void UAVGadgetInstanceManager::readSettings(QSettings *qs)
{
    while ( !m_configurations.isEmpty() ){
       emit configurationToBeDeleted(m_configurations.takeLast());
    }
    qs->beginGroup("UAVGadgetConfigurations");
    UAVConfigInfo configInfo(qs);
    configInfo.setNameOfConfigurable("UAVGadgetConfigurations");
    if ( configInfo.version() == UAVConfigVersion() ){
        // If version is not set, assume its a old version before readable config (1.0.0).
        // however compatibility to 1.0.0 is broken.
        configInfo.setVersion("1.0.0");
    }

    if ( configInfo.version() == UAVConfigVersion("1.1.0") ){
        configInfo.notify(tr("Migrating UAVGadgetConfigurations from version 1.1.0 to ")
                          + m_versionUAVGadgetConfigurations.toString());
        readConfigs_1_1_0(qs); // this is fully compatible with 1.2.0
    }
    else if ( !configInfo.standardVersionHandlingOK(m_versionUAVGadgetConfigurations) ){
        // We are in trouble now. User wants us to quit the import, but when he saves
        // the GCS, his old config will be lost.
        configInfo.notify(
                tr("You might want to save your old config NOW since it might be replaced by broken one when you exit the GCS!")
                );
    }
    else{
        readConfigs_1_2_0(qs);
    }

    qs->endGroup();
    createOptionsPages();
}

void UAVGadgetInstanceManager::readConfigs_1_2_0(QSettings *qs)
{
    UAVConfigInfo configInfo;

    foreach (QString classId, m_classIdNameMap.keys())
    {
        IUAVGadgetFactory *f = factory(classId);
        qs->beginGroup(classId);

        QStringList configs = QStringList();

        configs = qs->childGroups();
        foreach (QString configName, configs) {
            qDebug() << "Loading config: " << classId << "," <<  configName;
            qs->beginGroup(configName);
            configInfo.read(qs);
            configInfo.setNameOfConfigurable(classId+"-"+configName);
            qs->beginGroup("data");
            IUAVGadgetConfiguration *config = f->createConfiguration(qs, &configInfo);
            if (config){
                config->setName(configName);
                config->setProvisionalName(configName);
                config->setLocked(configInfo.locked());
                int idx = indexForConfig(m_configurations, classId, configName);
                if ( idx >= 0 ){
                    // We should replace the config, but it might be used, so just
                    // throw it out of the list. The GCS should be reinitialised soon.
                    m_configurations[idx] = config;
                }
                else{
                    m_configurations.append(config);
                }
            }
            qs->endGroup();
            qs->endGroup();
        }

        if (configs.count() == 0) {
            IUAVGadgetConfiguration *config = f->createConfiguration(0, 0);
            // it is not mandatory for uavgadgets to have any configurations (settings)
            // and therefore we have to check for that
            if (config) {
                config->setName(tr("default"));
                config->setProvisionalName(tr("default"));
                m_configurations.append(config);
            }
        }
        qs->endGroup();
    }
}

void UAVGadgetInstanceManager::readConfigs_1_1_0(QSettings *qs)
{
    UAVConfigInfo configInfo;

    foreach (QString classId, m_classIdNameMap.keys())
    {
        IUAVGadgetFactory *f = factory(classId);
        qs->beginGroup(classId);

        QStringList configs = QStringList();

        configs = qs->childGroups();
        foreach (QString configName, configs) {
            qDebug() << "Loading config: " << classId << "," <<  configName;
            qs->beginGroup(configName);
            bool locked = qs->value("config.locked").toBool();
            configInfo.setNameOfConfigurable(classId+"-"+configName);
            IUAVGadgetConfiguration *config = f->createConfiguration(qs, &configInfo);
            if (config){
                config->setName(configName);
                config->setProvisionalName(configName);
                config->setLocked(locked);
                int idx = indexForConfig(m_configurations, classId, configName);
                if ( idx >= 0 ){
                    // We should replace the config, but it might be used, so just
                    // throw it out of the list. The GCS should be reinitialised soon.
                    m_configurations[idx] = config;
                }
                else{
                    m_configurations.append(config);
                }
            }
            qs->endGroup();
        }

        if (configs.count() == 0) {
            IUAVGadgetConfiguration *config = f->createConfiguration(0, 0);
            // it is not mandatory for uavgadgets to have any configurations (settings)
            // and therefore we have to check for that
            if (config) {
                config->setName(tr("default"));
                config->setProvisionalName(tr("default"));
                m_configurations.append(config);
            }
        }
        qs->endGroup();
    }
}

void UAVGadgetInstanceManager::saveSettings(QSettings *qs)
{
    UAVConfigInfo *configInfo;
    qs->beginGroup("UAVGadgetConfigurations");
    qs->remove(""); // Remove existing configurations
    configInfo = new UAVConfigInfo(m_versionUAVGadgetConfigurations, "UAVGadgetConfigurations");
    configInfo->save(qs);
    delete configInfo;
    foreach (IUAVGadgetConfiguration *config, m_configurations)
    {
        configInfo = new UAVConfigInfo(config);
        qs->beginGroup(config->classId());
        qs->beginGroup(config->name());
        qs->beginGroup("data");
        config->saveConfig(qs, configInfo);
        qs->endGroup();
        configInfo->save(qs);
        qs->endGroup();
        qs->endGroup();
        delete configInfo;
    }
    qs->endGroup();
}

void UAVGadgetInstanceManager::createOptionsPages()
{
    // In case there are pages (import a configuration), remove them.
    // Maybe they should be deleted as well (memory-leak),
    // but this might lead to NULL-pointers?
    while (!m_optionsPages.isEmpty()){
        m_pm->removeObject(m_optionsPages.takeLast());
    }

    QMutableListIterator<IUAVGadgetConfiguration*> ite(m_configurations);
    while (ite.hasNext()) {
        IUAVGadgetConfiguration *config = ite.next();
        IUAVGadgetFactory *f = factory(config->classId());
        IOptionsPage *p = f->createOptionsPage(config);
        if (p) {
            IOptionsPage *page = new UAVGadgetOptionsPageDecorator(p, config, f->isSingleConfigurationGadget());
            page->setIcon(f->icon());
            m_optionsPages.append(page);
            m_pm->addObject(page);
        }
        else {
            qWarning()
                    << "UAVGadgetInstanceManager::createOptionsPages - failed to create options page for configuration "
                            + config->classId() + ":" + config->name() + ", configuration will be removed.";
            // The m_optionsPages list and m_configurations list must be in synch otherwise nasty issues happen later
            // so if we fail to create an options page we must remove the associated configuration
            ite.remove();
        }
    }
}


IUAVGadget *UAVGadgetInstanceManager::createGadget(QString classId, QWidget *parent)
{
    IUAVGadgetFactory *f = factory(classId);
    if (f) {
        QList<IUAVGadgetConfiguration*> *configs = configurations(classId);
        IUAVGadget *g = f->createGadget(parent);
        IUAVGadget *gadget = new UAVGadgetDecorator(g, configs);
        m_gadgetInstances.append(gadget);
        connect(this, SIGNAL(configurationAdded(IUAVGadgetConfiguration*)), gadget, SLOT(configurationAdded(IUAVGadgetConfiguration*)));
        connect(this, SIGNAL(configurationChanged(IUAVGadgetConfiguration*)), gadget, SLOT(configurationChanged(IUAVGadgetConfiguration*)));
        connect(this, SIGNAL(configurationNameChanged(IUAVGadgetConfiguration*, QString,QString)), gadget, SLOT(configurationNameChanged(IUAVGadgetConfiguration*, QString,QString)));
        connect(this, SIGNAL(configurationToBeDeleted(IUAVGadgetConfiguration*)), gadget, SLOT(configurationToBeDeleted(IUAVGadgetConfiguration*)));
        return gadget;
    }
    return 0;
}

void UAVGadgetInstanceManager::removeGadget(IUAVGadget *gadget)
{
    if (m_gadgetInstances.contains(gadget)) {
        m_gadgetInstances.removeOne(gadget);
        delete gadget;
        gadget = 0;
    }
}

/**
  * Removes all the gadgets. This is called by the core plugin when
  * shutting down: this ensures that all registered gadget factory destructors are
  * indeed called when the GCS is shutting down. We can't destroy them at the end
  * (coreplugin is deleted last), because the gadgets sometimes depend on other
  * plugins, like uavobjects...
  */
void UAVGadgetInstanceManager::removeAllGadgets()
{
    foreach( IUAVGadget *gadget, m_gadgetInstances) {
        m_gadgetInstances.removeOne(gadget);
        delete gadget;
    }
}


bool UAVGadgetInstanceManager::canDeleteConfiguration(IUAVGadgetConfiguration *config)
{
    // to be able to delete a configuration, no instance must be using it
    foreach (IUAVGadget *gadget, m_gadgetInstances) {
        if (gadget->activeConfiguration() == config) {
          return false;
        }
    }
    // and it cannot be the only configuration
    foreach (IUAVGadgetConfiguration *c, m_configurations) {
        if (c != config && c->classId() == config->classId())
            return true;
    }
    return false;
}

void  UAVGadgetInstanceManager::deleteConfiguration(IUAVGadgetConfiguration *config)
{
    m_provisionalDeletes.append(config);
    if (m_provisionalConfigs.contains(config)) {
        int i = m_provisionalConfigs.indexOf(config);
        m_provisionalConfigs.removeAt(i);
        m_provisionalOptionsPages.removeAt(i);
        int j = m_takenNames[config->classId()].indexOf(config->name());
        m_takenNames[config->classId()].removeAt(j);
        m_settingsDialog->deletePage();
    } else if (m_configurations.contains(config)) {
        m_settingsDialog->deletePage();
    }
    configurationNameEdited("", false);
}

void  UAVGadgetInstanceManager::cloneConfiguration(IUAVGadgetConfiguration *configToClone)
{
    QString name = suggestName(configToClone->classId(), configToClone->name());

    IUAVGadgetConfiguration *config = configToClone->clone();
    config->setName(name);
    config->setProvisionalName(name);
    IUAVGadgetFactory *f = factory(config->classId());
    IOptionsPage *p = f->createOptionsPage(config);
    IOptionsPage *page = new UAVGadgetOptionsPageDecorator(p, config);
    page->setIcon(f->icon());
    m_provisionalConfigs.append(config);
    m_provisionalOptionsPages.append(page);
    m_settingsDialog->insertPage(page);
}

// "name" => "name 1", "Name 3" => "Name 4", "Name1" => "Name1 1"
QString UAVGadgetInstanceManager::suggestName(QString classId, QString name)
{
    QString suggestedName;

    QString last = name.split(" ").last();
    bool ok;
    int num = last.toInt(&ok);
    int i = 1;
    if (ok) {
        QStringList n = name.split(" ");
        n.removeLast();
        name = n.join(" ");
        i = num+1;
    }
    do {
        suggestedName = name + " " + QString::number(i);
        ++i;
    } while (m_takenNames[classId].contains(suggestedName));

    m_takenNames[classId].append(suggestedName);
    return suggestedName;
}

void UAVGadgetInstanceManager::applyChanges(IUAVGadgetConfiguration *config)
{
    if (m_provisionalDeletes.contains(config)) {
        m_provisionalDeletes.removeAt(m_provisionalDeletes.indexOf(config));
        int i = m_configurations.indexOf(config);
        if (i >= 0) {
            emit configurationToBeDeleted(config);
            int j = m_takenNames[config->classId()].indexOf(config->name());
            m_takenNames[config->classId()].removeAt(j);
            m_pm->removeObject(m_optionsPages.at(i));
            m_configurations.removeAt(i);
            m_optionsPages.removeAt(i);//TODO delete
        }
        return;
    }
    if (config->provisionalName() != config->name()) {
        emit configurationNameChanged(config, config->name(), config->provisionalName());
        config->setName(config->provisionalName());
    }
    if (m_configurations.contains(config)) {
        emit configurationChanged(config);
    } else if (m_provisionalConfigs.contains(config)) {
        emit configurationAdded(config);
        int i = m_provisionalConfigs.indexOf(config);
        IOptionsPage *page = m_provisionalOptionsPages.at(i);
        m_configurations.append(config);
        m_optionsPages.append(page);
        m_provisionalConfigs.removeAt(i);
        m_provisionalOptionsPages.removeAt(i);
        m_pm->addObject(page);
    }
}

void UAVGadgetInstanceManager::configurationNameEdited(QString text, bool hasText)
{
    bool disable = false;
    foreach (IUAVGadgetConfiguration *c, m_configurations) {
        foreach (IUAVGadgetConfiguration *d, m_configurations) {
            if (c != d && c->classId() == d->classId() && c->provisionalName() == d->provisionalName())
                disable = true;
        }
        foreach (IUAVGadgetConfiguration *d, m_provisionalConfigs) {
            if (c != d && c->classId() == d->classId() && c->provisionalName() == d->provisionalName())
                disable = true;
        }
    }
    foreach (IUAVGadgetConfiguration *c, m_provisionalConfigs) {
        foreach (IUAVGadgetConfiguration *d, m_provisionalConfigs) {
            if (c != d && c->classId() == d->classId() && c->provisionalName() == d->provisionalName())
                disable = true;
        }
    }
    if (hasText && text == "")
        disable = true;
    m_settingsDialog->disableApplyOk(disable);
    if (hasText)
        m_settingsDialog->updateText(text);
}

void UAVGadgetInstanceManager::settingsDialogShown(Core::Internal::SettingsDialog* settingsDialog)
{
    foreach (QString classId, classIds())
        m_takenNames.insert(classId, configurationNames(classId));
    m_settingsDialog = settingsDialog;
}

void UAVGadgetInstanceManager::settingsDialogRemoved()
{
    m_takenNames.clear();
    m_provisionalConfigs.clear();
    m_provisionalDeletes.clear();
    m_provisionalOptionsPages.clear(); //TODO delete
    foreach (IUAVGadgetConfiguration *config, m_configurations)
        config->setProvisionalName(config->name());
    m_settingsDialog = 0;
}

QStringList UAVGadgetInstanceManager::configurationNames(QString classId) const
{
    QStringList names;
    foreach (IUAVGadgetConfiguration *config, m_configurations) {
        if (config->classId() == classId)
            names.append(config->name());
    }
    return names;
}

QString UAVGadgetInstanceManager::gadgetName(QString classId) const
{
    return m_classIdNameMap.value(classId);
}

QIcon UAVGadgetInstanceManager::gadgetIcon(QString classId) const
{
    return m_classIdIconMap.value(classId);
}

IUAVGadgetFactory *UAVGadgetInstanceManager::factory(QString classId) const
{
    foreach (IUAVGadgetFactory *f, m_factories) {
        if (f->classId() == classId)
            return f;
    }
    return 0;
}

QList<IUAVGadgetConfiguration*> *UAVGadgetInstanceManager::configurations(QString classId) const
{
    QList<IUAVGadgetConfiguration*> *configs = new QList<IUAVGadgetConfiguration*>;
    foreach (IUAVGadgetConfiguration *config, m_configurations) {
        if (config->classId() == classId)
            configs->append(config);
    }
    return configs;
}

int UAVGadgetInstanceManager::indexForConfig(QList<IUAVGadgetConfiguration*> configurations,
                   QString classId, QString configName)
{
    for ( int i=0; i<configurations.length(); ++i){
        if ( configurations.at(i)->classId() == classId
             && configurations.at(i)->name() == configName )
            return i;
    }
    return -1;
}
