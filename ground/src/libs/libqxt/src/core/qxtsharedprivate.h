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
/*!
\class QxtSharedPrivate QxtSharedPrivate
\inmodule QxtCore
\brief base class for shared data objects using the pimpl idom


combines both, the functionality of QxtPimpl and QSharedData \n
It is assumed you are familiar with QxtPimpl and QSharedData / QSharedDataPointer. If not, please read their documentation before this one. \n


<h4>Major differences to QxtPimpl are:</h4>
    - there is no qxt_p() since there are multiple public classes sharing the same private
    - you have to explicitly initialise the private data in the ctor of your public class using new
    - instead of using QXT_DECLARE_PRIVATE(MyClass) you use QxtSharedPrivate<myCLassPrivate> d;
    - The private data does not have to be a QxtPrivate subclass

<h4>Major differences to QSharedData / QSharedDataPointer are:</h4>
    - the actual Data can be a private implementation
    - access via d().member() instead of  d->member(); (QxtPimpl style)
    - The private data does not have to be a QSharedData subclass


<h4>example</h4>
expanding the example from the QSharedDataPointer we get this:
\code
 #ifndef EMPLOYEE_H
 #define EMPLOYEE_H

 #include <QxtSharedPrivate>
 #include <QString>

 class EmployeePrivate;

 class Employee
 {
 public:
     Employee();
     Employee(int id, const QString &name);

     void setId(int id) { d->id = id; }
     void setName(const QString &name);

     int id() const { return d->id; }
     QString name() const;

 private:
     QxtSharedPrivate<EmployeePrivate> qxt_d;
 };


 class EmployeePrivate
 {
 public:
     int id;
     QString name;
 };

 #endif

\endcode


note that contrary to QxtPimpl you need to initialise your private data once

\code
 Employee::Employee()
 {
     qxt_d = new EmployeePrivate;
 }
\endcode

if you ever really want to define a copy constructor for your public class, remember you must not reinitialise qxt_d as this would defeat the whole point of shared data.  instead just copy it. The underlying QSharedDataPointer will take care of the rest.

\code
 Employee::Employee(const Employee & other)
 {
     qxt_d = other.qxt_d;
 }
\endcode

Also remember you must not delete the private data yourself at any time.
*/

#ifndef QXTSHAREDPRIVATE_H
#define QXTSHAREDPRIVATE_H

#include <QSharedData>
#include <QSharedDataPointer>
#include <qxtglobal.h>

#ifndef QXT_DOXYGEN_RUN

template <typename PVT>
class QxtSharedPrivateData : public QSharedData
{
public:
    QxtSharedPrivateData()
    {
    }
    ~QxtSharedPrivateData()
    {
        delete data;
    }
    QxtSharedPrivateData(const QxtSharedPrivateData & other): QSharedData(other)
    {
        data = new PVT(*other.data);
    }
    PVT * data;
};

#endif // QXT_DOXYGEN_RUN

class QxtSharedPrivateDestructor
{
public:
    virtual ~QxtSharedPrivateDestructor() {}
};

template <typename PVT = QxtSharedPrivateDestructor>
class /*QXT_CORE_EXPORT*/ QxtSharedPrivate
{
public:

    QxtSharedPrivate()
    {
        pvt = 0;
    }
    QxtSharedPrivate(const QxtSharedPrivate &other)
    {
        pvt = other.pvt;
    }
    inline PVT& operator=(PVT * n)
    {
        Q_ASSERT(pvt == 0);

        QxtSharedPrivateData<PVT> *t = new QxtSharedPrivateData<PVT>;
        t->data = n;
        pvt = t;

        return   *static_cast<PVT *>(pvt->data);

    }
    inline PVT& operator()()
    {
        return   *static_cast<PVT *>(pvt->data);
    }
    inline const PVT& operator()() const
    {
        return   *static_cast<const PVT *>(pvt->data);
    }

private:
    QSharedDataPointer<QxtSharedPrivateData<PVT> > pvt;
};

#endif // QXTSHAREDPRIVATE_H
