/**
 ******************************************************************************
 *
 * @file       uavgadgetdecorator.cpp
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

#include "uavgadgetdecorator.h"
#include "iuavgadgetconfiguration.h"
#include <QtGui/QComboBox>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>

using namespace Core;

UAVGadgetDecorator::UAVGadgetDecorator(IUAVGadget *gadget, QList<IUAVGadgetConfiguration*> *configurations) :
        IUAVGadget(gadget->classId(), gadget->parent()),
        m_gadget(gadget),
        m_toolbar(new QComboBox),
        m_activeConfiguration(0),
        m_configurations(configurations)
{
    m_gadget->setParent(this);
    m_toolbar->setMinimumContentsLength(15);
    foreach (IUAVGadgetConfiguration *config, *m_configurations)
        m_toolbar->addItem(config->name());
    connect(m_toolbar, SIGNAL(activated(int)), this, SLOT(loadConfiguration(int)));
    if (m_configurations->count() > 0)
        loadConfiguration(0);
    updateToolbar();
}

UAVGadgetDecorator::~UAVGadgetDecorator()
{
    delete m_configurations;
    delete m_toolbar;
}

void UAVGadgetDecorator::loadConfiguration(int index) {
    IUAVGadgetConfiguration* config = m_configurations->at(index);
    loadConfiguration(config);
}

void UAVGadgetDecorator::loadConfiguration(IUAVGadgetConfiguration *config)
{
    m_activeConfiguration = config;
    int index = m_toolbar->findText(config->name());
    m_toolbar->setCurrentIndex(index);
    m_gadget->loadConfiguration(config);

}

void UAVGadgetDecorator::configurationChanged(IUAVGadgetConfiguration* config)
{
    if (config == m_activeConfiguration)
        loadConfiguration(config);
}

void UAVGadgetDecorator::configurationAdded(IUAVGadgetConfiguration* config)
{
    if (config->classId() != classId())
        return;
    m_configurations->append(config);
    m_toolbar->addItem(config->name());
    updateToolbar();
}

void UAVGadgetDecorator::configurationToBeDeleted(IUAVGadgetConfiguration* config)
{
    if (config->classId() != classId())
        return;
    int index = m_configurations->indexOf(config);
    if (index >= 0) {
        m_toolbar->removeItem(index);
        m_configurations->removeAt(index);
    }
    updateToolbar();
}

void UAVGadgetDecorator::configurationNameChanged(IUAVGadgetConfiguration* config, QString oldName, QString newName)
{
    if (config->classId() != classId())
        return;
    for (int i = 0; i < m_toolbar->count(); ++i) {
        if (m_toolbar->itemText(i) == oldName)
            m_toolbar->setItemText(i, newName);
    }
}

void UAVGadgetDecorator::updateToolbar()
{
    m_toolbar->setEnabled(m_toolbar->count() > 1);
}

QByteArray UAVGadgetDecorator::saveState()
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    if (m_activeConfiguration)
        stream << m_activeConfiguration->name().toLatin1();
    return bytes;
}

void UAVGadgetDecorator::restoreState(QByteArray state)
{
    QDataStream stream(state);
    QByteArray configName;
    stream >> configName;
    foreach (IUAVGadgetConfiguration *config, *m_configurations) {
        if (config->name() == configName) {
            m_activeConfiguration = config;
            loadConfiguration(config);
        }
    }
}
