/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
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

#include "qxtsortfilterproxymodel.h"
#include <QRegExp>

class QxtModelFilterPrivate
{
    public:
        QxtModelFilterPrivate   (const QVariant &value = QVariant(), const int role = Qt::DisplayRole, const Qt::MatchFlags flags = Qt::MatchContains);
        bool            acceptsValue ( const QVariant & value );
        QVariant        m_value;
        int             m_role;
        Qt::MatchFlags  m_flags;
};

class QxtSortFilterProxyModelPrivate : public QxtPrivate<QxtSortFilterProxyModel>
{
    public:
        QMap<int,QxtModelFilterPrivate> filters;
        bool                            m_declaringFilter;
};

QxtModelFilterPrivate::QxtModelFilterPrivate(const QVariant &value, const int role, const Qt::MatchFlags flags)
    :m_value(value),m_role(role),m_flags(flags)
{

}

bool QxtModelFilterPrivate::acceptsValue ( const QVariant & value )
{
    uint matchType = m_flags & 0x0F;
    //if we have no value we accept everything
    if(!m_value.isValid() || !value.isValid())
        return true;

    // QVariant based matching
    if (matchType == Qt::MatchExactly ){
        if(m_value == value) 
            return true;
    }
    // QString based matching
    else { 
        Qt::CaseSensitivity cs = m_flags & Qt::MatchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
        QString filterText = m_value.toString();
        QString modelText  = value.toString();
        switch (matchType){
            case Qt::MatchRegExp:
                if (QRegExp(filterText, cs).exactMatch(modelText))
                    return true;
                break;
            case Qt::MatchWildcard:
                if (QRegExp(filterText, cs, QRegExp::Wildcard).exactMatch(modelText))
                    return true;
                break;
            case Qt::MatchStartsWith:
                if (modelText.startsWith(filterText, cs))
                    return true;
                break;
            case Qt::MatchEndsWith:
                if (modelText.endsWith(filterText, cs))
                    return true;
                break;
            case Qt::MatchFixedString:
                if (modelText.compare(filterText, cs) == 0)
                    return true;
                break;
            default:
            case Qt::MatchContains:
                if (modelText.contains(filterText, cs))
                    return true;
        }
    }
    return false;
}

/*!
    \class QxtSortFilterProxyModel
    \inmodule QxtGui
    \brief The QxtSortFilterProxyModel class is a multi column filter model.
    
    The QxtSortFilterProxyModel makes it possible to filter over multiple columns.

    \code
    QxtSortFilterProxyModel * filterModel = new QxtSortFilterProxyModel(parent);
    filterModel->setSourceModel(sourceModel);
    filterModel->beginDeclareFilter();
    filterModel->setFilter(1,QVariant("SomeStringValue"),Qt::DisplayRole,Qt::MatchExactly);
    //remove some old filter
    filterModel->removeFilter(2);
    filterModel->setFilter(5,QVariant(1234),Qt::DisplayRole,Qt::MatchExactly);
    filterModel->endDeclateFilter();
    \endcode

    Now the model will filter column 1 and 5, to be accepted by the filtermodel a row needs to pass all filters
*/

/*!
    Constructs a new QxtSortFilterProxyModel with \a parent.
 */
QxtSortFilterProxyModel::QxtSortFilterProxyModel ( QObject *parent) : QSortFilterProxyModel(parent)
{
    QXT_INIT_PRIVATE(QxtSortFilterProxyModel);
    qxt_d().m_declaringFilter = false;
}


/*!
    \brief tells the model you want to declare a new filter

    If you have a lot of data in your model it can be slow to declare more than one filter,
    because the model will always rebuild itself. If you call this member before setting the
    new filters the model will invalidate its contents not before you call
    
    \sa endDeclareFilter()
 */
void QxtSortFilterProxyModel::beginDeclareFilter ( )
{
    qxt_d().m_declaringFilter = true;
}

/*!
    \brief stops the filter declaration and invalidates the filter 

    \sa beginDeclareFilter()
 */
void QxtSortFilterProxyModel::endDeclareFilter ( )
{
    if(qxt_d().m_declaringFilter){
        qxt_d().m_declaringFilter = false;
        invalidateFilter();
    }
}

/*!
    \reimp
 */
bool QxtSortFilterProxyModel::filterAcceptsRow ( int source_row, const QModelIndex &source_parent ) const
{
    QList<int> filterColumns = qxt_d().filters.keys();
    foreach(int currCol,filterColumns){
        QxtModelFilterPrivate filter = qxt_d().filters[currCol];
        // if the column specified by the user is -1 
        // that means all columns need to pass the filter to get into the result
        if (currCol == -1) {
            int column_count = sourceModel()->columnCount(source_parent);
            for (int column = 0; column < column_count; ++column) {
                QModelIndex source_index = sourceModel()->index(source_row, column, source_parent);
                QVariant key = sourceModel()->data(source_index, filter.m_role);
                if (!filter.acceptsValue(key))
                   return false;
            }
            continue;
        }
        QModelIndex source_index = sourceModel()->index(source_row, currCol , source_parent);
        if (!source_index.isValid()) // the column may not exist
            continue;
        QVariant key = sourceModel()->data(source_index, filter.m_role);
        if(!filter.acceptsValue(key))
            return false;
    }
    return true;
}

/*!
    \brief Sets the filter with \a value, \a role and \a flags to the given \a column
 */
void QxtSortFilterProxyModel::setFilter ( const int column, const QVariant &value, const int role, Qt::MatchFlags flags )
{
    if(qxt_d().filters.contains(column))
        qxt_d().filters[column] = QxtModelFilterPrivate(value,role,flags);
    else
        qxt_d().filters.insert(column,QxtModelFilterPrivate(value,role,flags));

    if(!qxt_d().m_declaringFilter)
        invalidateFilter();
}

/*!
    \brief Removes the filter from the given \a column
 */
void QxtSortFilterProxyModel::removeFilter ( const int column )
{
    if(qxt_d().filters.contains(column)){
        qxt_d().filters.remove(column);

        if(!qxt_d().m_declaringFilter)
            invalidateFilter();
    }
}

/*!
    \brief Sets the filter \a value for the given \a column
 */
void QxtSortFilterProxyModel::setFilterValue ( const int column , const QVariant &value )
{
    if(qxt_d().filters.contains(column))
        qxt_d().filters[column].m_value = value;
    else
        qxt_d().filters.insert(column,QxtModelFilterPrivate(value));

    if(!qxt_d().m_declaringFilter)
        invalidateFilter();
}

/*!
    \brief Sets the filter \a role for the given \a column
 */
void QxtSortFilterProxyModel::setFilterRole ( const int column , const int role )
{
    if(qxt_d().filters.contains(column))
        qxt_d().filters[column].m_role=role;
    else
        qxt_d().filters.insert(column,QxtModelFilterPrivate(QVariant(),role));

    if(!qxt_d().m_declaringFilter)
        invalidateFilter();
}

/*!
    \brief Sets the filter \a flags for the given \a column
 */
void QxtSortFilterProxyModel::setFilterFlags ( const int column , const Qt::MatchFlags flags )
{
    if(qxt_d().filters.contains(column))
        qxt_d().filters[column].m_flags = flags;
    else
        qxt_d().filters.insert(column,QxtModelFilterPrivate(QVariant(),Qt::DisplayRole,flags));

    if(!qxt_d().m_declaringFilter)
        invalidateFilter();
}

/*!
    \brief Returns the filter value for the given \a column

    \bold {Note:} if the column is not filtered it will return a null variant
 */
QVariant QxtSortFilterProxyModel::filterValue ( const int column ) const
{
    if(qxt_d().filters.contains(column))
        return qxt_d().filters[column].m_value;
    return QVariant();
}

/*!
    \brief Returns the filter role for the given \a column

    \bold {Note:} if the column is not filtered it will return \c -1    
*/
int QxtSortFilterProxyModel::filterRole ( const int column ) const
{
    if(qxt_d().filters.contains(column))
        return qxt_d().filters[column].m_role;
    return -1;
}

/*!
    \brief returns the filter flags for the given \a column

    \bold {Note:} if the column is not filtered it will return the default value
 */
Qt::MatchFlags QxtSortFilterProxyModel::filterFlags ( const int column ) const
{
    if(qxt_d().filters.contains(column))
        return qxt_d().filters[column].m_flags;
    return Qt::MatchContains;
}

/*!
    Returns true if the \a column is filtered
 */
bool QxtSortFilterProxyModel::isFiltered ( const int column )
{
    return qxt_d().filters.contains(column);
}
