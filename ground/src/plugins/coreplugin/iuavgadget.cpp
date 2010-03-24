/**
 ******************************************************************************
 *
 * @file       iuavgadget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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

#include "iuavgadget.h"
#include "iuavgadgetconfiguration.h"

using namespace Core;

IUAVGadget::IUAVGadget(QString classId, QList<IUAVGadgetConfiguration*> *configurations, QObject *parent) :
        IContext(parent),
        m_classId(classId),
        m_toolbar(new QComboBox),
        m_configurations(configurations)
{
    foreach (IUAVGadgetConfiguration *config, *configurations)
        m_toolbar->addItem(config->name());
    connect(m_toolbar, SIGNAL(activated(int)), this, SLOT(loadConfiguration(int)));
}

void IUAVGadget::loadConfiguration(int index) {
    IUAVGadgetConfiguration* config = m_configurations->at(index);
    loadConfiguration(config);
}


void IUAVGadget::configurationChanged(IUAVGadgetConfiguration* config)
{
    if (config == m_activeConfiguration)
        loadConfiguration(config);
}

void IUAVGadget::configurationNameChanged(QString oldName, QString newName)
{
    for (int i = 0; i < m_toolbar->count(); ++i) {
        if (m_toolbar->itemText(i) == oldName)
            m_toolbar->setItemText(i, newName);
    }
}

QByteArray IUAVGadget::saveState()
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
//    if (m_activeConfiguration)
//        stream << m_activeConfiguration->name().toLatin1();
    return bytes;
}

void IUAVGadget::restoreState(QByteArray state)
{
    QDataStream stream(state);
    QByteArray configName;
    stream >> configName;
    foreach (IUAVGadgetConfiguration *config, *m_configurations) {
        if (config->name() == configName)
            loadConfiguration(config);
    }
}
