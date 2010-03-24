/**
 ******************************************************************************
 *
 * @file       uavgadgetinstancemanager.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   coreplugin
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

#include "uavgadgetinstancemanager.h"
#include "iuavgadgetfactory.h"
#include "iuavgadgetconfiguration.h"
#include "uavgadgetoptionspage.h"
#include "icore.h"

#include <extensionsystem/pluginmanager.h>
#include <QtCore/QStringList>
#include <QtCore/QSettings>
#include <QtCore/QDebug>


using namespace Core;

UAVGadgetInstanceManager::UAVGadgetInstanceManager(QObject *parent) :
    QObject(parent)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    QList<IUAVGadgetFactory*> factories = pm->getObjects<IUAVGadgetFactory>();
    foreach (IUAVGadgetFactory *f, factories) {
        if (!m_uavGadgetFactories.contains(f)) {
            m_uavGadgetFactories.append(f);
            QString classId = f->classId();
            QString name = f->name();
            m_uavGadgetClassIds.insert(classId, name);
        }
    }
}

void UAVGadgetInstanceManager::readUAVGadgetConfigurations()
{
    QSettings *qs = Core::ICore::instance()->settings();
    qs->beginGroup("UAVGadgetConfigurations");
    foreach (QString classId, m_uavGadgetClassIds.keys())
    {
        IUAVGadgetFactory *f = uavGadgetFactory(classId);
        qs->beginGroup(classId);
        QStringList configs = qs->childKeys();

        foreach (QString configName, configs) {
            QByteArray ba = QByteArray::fromBase64(qs->value(configName).toByteArray());
            QDataStream stream(ba);
            bool locked;
            stream >> locked;
            QByteArray state;
            stream >> state;
            IUAVGadgetConfiguration *config = f->createUAVGadgetConfiguration(locked, configName, state);
            m_uavGadgetConfigurations.append(config);
        }

        if (configs.count() == 0) {
            IUAVGadgetConfiguration *config = f->createUAVGadgetConfiguration(false, tr("default"), 0);
            // it is not mandatory for uavgadgets to have any configurations (settings)
            // and therefore we have to check for that
            if (config)
                m_uavGadgetConfigurations.append(config);
        }
        qs->endGroup();
    }
    qs->endGroup();
    createUAVGadgetOptionPages();
}

void UAVGadgetInstanceManager::writeUAVGadgetConfigurations()
{
    QSettings *qs = Core::ICore::instance()->settings();
    qs->beginGroup("UAVGadgetConfigurations");
    qs->allKeys().clear(); // Remove existing configurations
    foreach (IUAVGadgetConfiguration *config, m_uavGadgetConfigurations)
    {
        qs->beginGroup(config->classId());
        QByteArray ba;
        QDataStream stream(&ba, QIODevice::WriteOnly);
        stream << config->locked();
        stream << config->saveState();
        qs->setValue(config->name(), ba.toBase64());
        qs->endGroup();
    }
    qs->endGroup();
}

void UAVGadgetInstanceManager::createUAVGadgetOptionPages()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    foreach (IUAVGadgetConfiguration *config, m_uavGadgetConfigurations)
    {
        IUAVGadgetFactory *f = uavGadgetFactory(config->classId());
        UAVGadgetOptionsPage *page = f->createUAVGadgetOptionsPage(config);
        pm->addObject(page);
    }
}


IUAVGadget *UAVGadgetInstanceManager::createUAVGadget(QString classId, QWidget *parent)
{
    IUAVGadgetFactory *f = uavGadgetFactory(classId);
    if (f) {
        QList<IUAVGadgetConfiguration*> *configs = uavGadgetConfigurations(classId);
        IUAVGadget *gadget = f->createUAVGadget(configs, parent);
        m_uavGadgetInstances.append(gadget);
        return gadget;
    }
    return 0;
}

//IUAVGadgetConfiguration *UAVGadgetInstanceManager::createUAVGadgetConfiguration(QString classId,
//                                                      QString configName)
//{
//    return 0;
//}

//UAVGadgetOptionsPage *UAVGadgetInstanceManager::createUAVGadgetOptionsPage(QString classId)
//{
//    return 0;
//}

QStringList UAVGadgetInstanceManager::uavGadgetConfigurationNames(QString classId) const
{
    QStringList names;
    return names;
}

QString UAVGadgetInstanceManager::uavGadgetName(QString classId) const
{
    return m_uavGadgetClassIds.value(classId);
}

IUAVGadgetFactory *UAVGadgetInstanceManager::uavGadgetFactory(QString classId) const
{
    foreach (IUAVGadgetFactory *f, m_uavGadgetFactories) {
        if (f->classId() == classId)
            return f;
    }
    return 0;
}

QList<IUAVGadgetConfiguration*> *UAVGadgetInstanceManager::uavGadgetConfigurations(QString classId) const
{
    QList<IUAVGadgetConfiguration*> *configs = new QList<IUAVGadgetConfiguration*>;
    foreach (IUAVGadgetConfiguration *config, m_uavGadgetConfigurations) {
        if (config->classId() == classId)
            configs->append(config);
    }
    return configs;
}



