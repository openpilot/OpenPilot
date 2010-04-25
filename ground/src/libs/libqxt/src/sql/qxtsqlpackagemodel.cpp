/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtSql module of the Qxt library.
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

#include "qxtsqlpackagemodel.h"




/*!
\class QxtSqlPackageModel

\inmodule QxtSql

\brief The QxtSqlPackageModel class provides a read-only data model for QxtSqlPackage result..

example usage:
\code
    QSqlQuery q;
    q.exec("SELECT *");

    QxtSqlPackage p;
    p.insert(q);

    QxtSqlPackageModel m;
    m.setQuery(p);

    QTableView v;
    v.setModel(&m);
    v.show();
\endcode

*/

/*!
\fn void QxtSqlPackageModel::setQuery(QxtSqlPackage package)
\brief set the \a package for the model.

\warning do this before any access.
*/

/*!
    Creates a QxtSqlPackageModel with \a parent.
 */
QxtSqlPackageModel::QxtSqlPackageModel(QObject * parent) : QAbstractTableModel(parent)
{
}

void QxtSqlPackageModel::setQuery(QxtSqlPackage package)
{
    pack = package;
}

/*!
    \reimp
 */
int QxtSqlPackageModel::rowCount(const QModelIndex &) const
{
    return pack.count();
}

/*!
    \reimp
 */
int QxtSqlPackageModel::columnCount(const QModelIndex &) const
{
    QxtSqlPackage p = pack;
    return p.hash(0).count();
}

/*!
    \reimp
 */
QVariant QxtSqlPackageModel::data(const QModelIndex  & index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();



    if ((index.row() < 0)  || (index.column() < 0)) return QVariant();
    QxtSqlPackage p = pack;

    return  p.hash(index.row()).values().at(index.column());


}

/*!
    \reimp
 */
QVariant QxtSqlPackageModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if (orientation == Qt::Vertical && role == Qt::DisplayRole)
        return section;


    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        QxtSqlPackage p = pack;
        return p.hash(0).keys().at(section)    ;
    }

    return QAbstractItemModel::headerData(section, orientation, role);

}
