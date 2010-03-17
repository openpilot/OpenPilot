/**
 ******************************************************************************
 *
 * @file       uavgadgetmode.cpp
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

#include "uavgadgetmode.h"
#include "uavgadgetmanager.h"
#include "coreconstants.h"
#include "modemanager.h"
#include "uniqueidmanager.h"
#include "minisplitter.h"
#include "outputpane.h"
#include "rightpane.h"
#include "iuavgadget.h"

#include <QtCore/QLatin1String>
#include <QtGui/QHBoxLayout>
#include <QtGui/QWidget>
#include <QtGui/QSplitter>

using namespace Core;
using namespace Core::Internal;

UAVGadgetMode::UAVGadgetMode(UAVGadgetManager *uavGadgetManager, QString name, QIcon icon, int priority, QString uniqueName) :
    m_uavGadgetManager(uavGadgetManager),
    m_name(name),
    m_icon(icon),
    m_priority(0),
    m_widget(new QWidget),
    m_layout(new QVBoxLayout)
{
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_widget->setLayout(m_layout);
    m_layout->insertWidget(0, new Core::UAVGadgetManagerPlaceHolder(this));
    if (0 <= priority && priority <= 100)
        m_priority = priority;

    ModeManager *modeManager = ModeManager::instance();
    // checking that the mode name is unique gives harmless
    // warnings on the console output
    if (!modeManager->mode(uniqueName)) {
        m_uniqueName = uniqueName;
    } else {
        // this shouldn't happen
        m_uniqueName = uniqueName + QString::number(quint64(this));
    }
    connect(modeManager, SIGNAL(currentModeChanged(Core::IMode*)),
            this, SLOT(grabUAVGadgetManager(Core::IMode*)));
    m_widget->setFocusProxy(m_uavGadgetManager);
}

UAVGadgetMode::~UAVGadgetMode()
{
    // TODO: see if this leftover from Qt Creator still applies
    // Make sure the uavGadget manager does not get deleted
    m_uavGadgetManager->setParent(0);
    delete m_widget;
}

QString UAVGadgetMode::name() const
{
    return m_name;
}

QIcon UAVGadgetMode::icon() const
{
    return m_icon;
}

int UAVGadgetMode::priority() const
{
    return m_priority;
}

QWidget* UAVGadgetMode::widget()
{
    return m_widget;
}

const char* UAVGadgetMode::uniqueModeName() const
{
    return m_uniqueName.toAscii().data();
}

QList<int> UAVGadgetMode::context() const
{
    static QList<int> contexts = QList<int>() <<
        UniqueIDManager::instance()->uniqueIdentifier(Constants::C_UAVGADGET_MODE) <<
        UniqueIDManager::instance()->uniqueIdentifier(Constants::C_UAVGADGETMANAGER);
    return contexts;
}

void UAVGadgetMode::grabUAVGadgetManager(Core::IMode *mode)
{
    if (mode != this)
        return;

    if (m_uavGadgetManager->currentUAVGadget())
        m_uavGadgetManager->currentUAVGadget()->widget()->setFocus();
}
