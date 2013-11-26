/**
 ******************************************************************************
 *
 * @file       uavobjecthelper.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup [Group]
 * @{
 * @addtogroup UAVObjectHelper
 * @{
 * @brief [Brief]
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

#include "uavobjecthelper.h"
#include <QTimer>

AbstractUAVObjectHelper::AbstractUAVObjectHelper(QObject *parent) :
    QObject(parent), m_transactionResult(false), m_transactionCompleted(false)
{}

AbstractUAVObjectHelper::Result AbstractUAVObjectHelper::doObjectAndWait(UAVObject *object, int timeout)
{
    // Lock, we can't call this twice from different threads
    QMutexLocker locker(&m_mutex);

    m_object = object;

    // Reset variables
    m_transactionResult    = false;
    m_transactionCompleted = false;

    // Create timer and connect it, connect object tx completed to local slot
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(&timeoutTimer, SIGNAL(timeout()), &m_eventLoop, SLOT(quit()));
    connect(object, SIGNAL(transactionCompleted(UAVObject *, bool)), this, SLOT(transactionCompleted(UAVObject *, bool)));

    // Start timeout timer
    timeoutTimer.start(timeout);

    // Call the actual implementation in concrete subclass
    doObjectAndWaitImpl();

    // Wait if not completed
    if (!m_transactionCompleted) {
        m_eventLoop.exec();
    }
    timeoutTimer.stop();

    // Disconnect
    disconnect(object, SIGNAL(transactionCompleted(UAVObject *, bool)), this, SLOT(transactionCompleted(UAVObject *, bool)));
    disconnect(&timeoutTimer, SIGNAL(timeout()), &m_eventLoop, SLOT(quit()));

    // Return result
    if (!m_transactionCompleted) {
        return TIMEOUT;
    } else {
        return m_transactionResult ? SUCCESS : FAIL;
    }
}

void AbstractUAVObjectHelper::transactionCompleted(UAVObject *object, bool success)
{
    Q_UNUSED(object)

    // Set variables and quit event loop
    m_transactionResult    = success;
    m_transactionCompleted = true;
    m_eventLoop.quit();
}

UAVObjectUpdaterHelper::UAVObjectUpdaterHelper(QObject *parent) : AbstractUAVObjectHelper(parent)
{}

void UAVObjectUpdaterHelper::doObjectAndWaitImpl()
{
    m_object->updated();
}

UAVObjectRequestHelper::UAVObjectRequestHelper(QObject *parent) : AbstractUAVObjectHelper(parent)
{}

void UAVObjectRequestHelper::doObjectAndWaitImpl()
{
    m_object->requestUpdate();
}
