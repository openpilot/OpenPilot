/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtCore module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/

#ifndef QXTPOINTERLIST_H
#define QXTPOINTERLIST_H

#include <QList>
#include <QObject>
#include "qxtglobal.h"

class QXT_CORE_EXPORT QxtPointerListDeleter : public QObject
{
    Q_OBJECT
protected:
    virtual void removeThisObject(QObject * obj) = 0;
private Q_SLOTS:
    void removeSender();
};

template<class T>
class QxtPointerList : public QxtPointerListDeleter, public QList<T*>
{
public:
    ///constructs a new QxtPointerList
    QxtPointerList(): QList<T*>()
    {
    }
    ///destructs the QxtPointerList
    ~QxtPointerList()
    {
        QList<T*>::clear();
    }
    ///\reimp
    QxtPointerList(const QxtPointerList<T> & other): QxtPointerListDeleter(), QList<T*>(other)
    {
        for (int i = 0;i < other.size();i++)
        {
            addThisObject(other.at(i));
        }
    }
    ///\reimp
    void append(T*  value)
    {
        addThisObject(value);
        QList<T*>::append(value);
    }
    ///\reimp
    void insert(int i, T * value)
    {
        addThisObject(value);
        QList<T*>::insert(i, value);
    }
    ///\reimp
    typename QList<T*>::iterator insert(typename QList<T*>::iterator before,  T*  value)
    {
        addThisObject(value);
        return QList<T*>::insert(before, value);
    }
    ///\reimp
    QxtPointerList<T> operator+ (const QxtPointerList<T> & other) const
    {
        QxtPointerList<T> m = *this;
        m += other;
        return m;
    }
    ///\reimp
    QxtPointerList<T> & operator+= (const QxtPointerList<T> & other)
    {
        Q_FOREACH(T*t, other)
        {
            *this << t;
        }
        return *this;
    }
    ///\reimp
    QxtPointerList<T> & operator+= (T*  value)
    {
        addThisObject(value);
        QList<T*>::operator+=(value);
        return *this;
    }
    ///\reimp
    QxtPointerList<T> & operator<< (const QxtPointerList<T> & other)
    {
        *this += other;
        return *this;
    }
    ///\reimp
    QxtPointerList<T> & operator<< (T* value)
    {
        *this += value;
        return *this;
    }
    ///\reimp
    QxtPointerList<T> & operator= (const QxtPointerList<T> & other)
    {
        QList<T*>::clear();
        Q_FOREACH(T*t, other)
        {
            *this << t;
        }
        return *this;
    }

protected:
    ///reimplement to access objects before they are removed
    virtual void removeThisObject(QObject * obj)
    {
        removeAll(reinterpret_cast<T*>(obj));
    }
    ///reimplement to access objects before they are added
    virtual void addThisObject(QObject * obj)
    {
        QObject::connect(obj, SIGNAL(destroyed(QObject *)), this, SLOT(removeSender()));
    }
};

#endif // QXTPOINTERLIST_H
