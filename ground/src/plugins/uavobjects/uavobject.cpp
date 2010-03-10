/**
 ******************************************************************************
 *
 * @file       uavobject.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   
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
#include "uavobject.h"


UAVObject::UAVObject(quint32 objID, quint32 instID, bool isSingleInst, QString& name, quint32 numBytes): QObject()
{
    this->objID = objID;
    this->instID = instID;
    this->isSingleInst = isSingleInst;
    this->name = name;
    this->numBytes = numBytes;
    this->mutex = new QMutex(QMutex::Recursive);
}

quint32 UAVObject::getObjID()
{
    return objID;
}

quint32 UAVObject::getInstID()
{
    return instID;
}

bool UAVObject::isSingleInstance()
{
    return isSingleInst;
}

QString UAVObject::getName()
{
    return name;
}

quint32 UAVObject::getNumBytes()
{
    return numBytes;
}

void UAVObject::requestUpdate()
{
    emit updateRequested(this);
}

void UAVObject::updated()
{
    emit objectUpdated(this, false);
}

void UAVObject::lock()
{
    mutex->lock();
}

void UAVObject::lock(int timeoutMs)
{
    mutex->tryLock(timeoutMs);
}

void UAVObject::unlock()
{
    mutex->unlock();
}

QMutex* UAVObject::getMutex()
{
    return mutex;
}

