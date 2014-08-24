/**
 ******************************************************************************
 *
 * @file       uavobjecthelper.h
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

#ifndef UAVOBJECTHELPER_H
#define UAVOBJECTHELPER_H

#include <QObject>
#include <QEventLoop>
#include <QMutex>
#include <QMutexLocker>

#include "uavobjectutil_global.h"
#include "uavobject.h"

class UAVOBJECTUTIL_EXPORT AbstractUAVObjectHelper : public QObject {
    Q_OBJECT
public:
    explicit AbstractUAVObjectHelper(QObject *parent = 0);
    virtual ~AbstractUAVObjectHelper();

    enum Result { SUCCESS, FAIL, TIMEOUT };
    Result doObjectAndWait(UAVObject *object, int timeout);

protected:
    virtual void doObjectAndWaitImpl() = 0;
    UAVObject *m_object;

private slots:
    void transactionCompleted(UAVObject *object, bool success);

private:
    QMutex m_mutex;
    QEventLoop m_eventLoop;
    bool m_transactionResult;
    bool m_transactionCompleted;
};

class UAVOBJECTUTIL_EXPORT UAVObjectUpdaterHelper : public AbstractUAVObjectHelper {
    Q_OBJECT
public:
    explicit UAVObjectUpdaterHelper(QObject *parent = 0);
    virtual ~UAVObjectUpdaterHelper();

protected:
    virtual void doObjectAndWaitImpl();
};

class UAVOBJECTUTIL_EXPORT UAVObjectRequestHelper : public AbstractUAVObjectHelper {
    Q_OBJECT
public:
    explicit UAVObjectRequestHelper(QObject *parent = 0);
    virtual ~UAVObjectRequestHelper();

protected:
    virtual void doObjectAndWaitImpl();
};

#endif // UAVOBJECTHELPER_H
