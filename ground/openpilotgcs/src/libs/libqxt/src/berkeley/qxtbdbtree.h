/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtBerkeley module of the Qxt library.
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
#ifndef QxtBdbTree_H_Guard_pyxvby
#define QxtBdbTree_H_Guard_pyxvby

#include <qxtsharedprivate.h>
#include <QVariant>
#include <QPair>
#include <QDebug>
#include "qxtbdb.h"


template<class T>
class QxtBdbTreeIterator;


template<class T>
class QxtBdbTree
{
public:
    QxtBdbTree();
    QxtBdbTree(QString file);
    bool open(QString file);
    void clear();
    bool flush();
    QxtBdbTreeIterator<T> root() const;

    void dumpTree() const;


private:
    int meta_id;
    QxtSharedPrivate<QxtBdb> qxt_d;
    void debugprintchildren(QxtBdbTreeIterator<T> it) const;
};






template<class T>
QxtBdbTree<T>::QxtBdbTree()
{
    meta_id = qMetaTypeId<T>();
    qxt_d = new QxtBdb;
}

template<class T>
QxtBdbTree<T>::QxtBdbTree(QString file)
{
    meta_id = qMetaTypeId<T>();
    qxt_d = new QxtBdb;
    open(file);
}


template<class T>
bool QxtBdbTree<T>::open(QString file)
{
    BerkeleyDB::u_int32_t f;
    qxt_d().db->get_flags(qxt_d().db, &f);
    f |= DB_DUP;
    qxt_d().db->set_flags(qxt_d().db, f);
    bool r = qxt_d().open(file, QxtBdb::CreateDatabase | QxtBdb::LockFree);
    return r;

}

template<class T>
void QxtBdbTree<T>::clear()
{
    if (!qxt_d().isOpen)
        return;

    BerkeleyDB::u_int32_t x;
    qxt_d().db->truncate(qxt_d().db, NULL, &x, 0);
}

template<class T>
bool QxtBdbTree<T>::flush()
{
    return qxt_d().flush();
}


template<class T>
QxtBdbTreeIterator<T> QxtBdbTree<T>::root() const
{
    QxtBdbTreeIterator<T> r(0, const_cast<QxtBdb*>(&qxt_d()));
    r.root = true;
    return r;
}





template<class T>
void QxtBdbTree<T>::debugprintchildren(QxtBdbTreeIterator<T> it) const
{
    it = it.child();
    while (it.isValid())
    {
        QByteArray p;
        for (quint64 x = 0; x < it.level();x++)
        {
            p += "   |";
        }
        qDebug() << p.data() << "-" << it.value();
        debugprintchildren(it);
        ++it;
    }
}

/*

|-n1
|-n2
|  |-n3
|-n1
|  |-n1
|  |  |-n3
|  |  |  -n2

*/


template<class T>
void QxtBdbTree<T>::dumpTree() const
{
    BerkeleyDB::DBC *cursor;
    qxt_d().db->cursor(qxt_d().db, NULL, &cursor, 0);
    if (!qxt_d().get((void*)0, 0, 0, 0, DB_FIRST, cursor))
    {
        return;
    }

    QxtBdbTreeIterator<T> k(cursor, const_cast<QxtBdb*>(&qxt_d()));
    qDebug() << "\r\nQxtBdbTree<" << QMetaType::typeName(qMetaTypeId<T>()) << ">::dumpTree()\r\n";
    qDebug() << "Level\t  \t Value";

    Q_FOREVER
    {
        qDebug() << k.level() << "  \t->\t" << k.value();
        if (!qxt_d().get((void*)0, 0, 0, 0, DB_NEXT, cursor))
            break;
    }
    qDebug() << "\r\n";

    qDebug() << "Iterate";
    debugprintchildren(root());
    qDebug() << "\r\n";
}



























template<class T>
class QxtBdbTreeIterator
{
public:
    QxtBdbTreeIterator();
    ~QxtBdbTreeIterator();
    QxtBdbTreeIterator(const QxtBdbTreeIterator<T> & other);
    QxtBdbTreeIterator<T> & operator= (const QxtBdbTreeIterator<T> & other);

    bool isValid() const;

    operator T() const;
    T value() const;
    bool setValue(T);


    QxtBdbTreeIterator<T>    parent() const;
    QxtBdbTreeIterator<T>    next() const;
    QxtBdbTreeIterator<T>    previous() const;
    QxtBdbTreeIterator<T>    child() const;


    QxtBdbTreeIterator<T>    operator + (int j) const;
    QxtBdbTreeIterator<T> &  operator ++ ();
    QxtBdbTreeIterator<T>    operator ++ (int);
    QxtBdbTreeIterator<T> &  operator += (int j);

    QxtBdbTreeIterator<T>    operator - (int j) const;
    QxtBdbTreeIterator<T> &  operator -- ();
    QxtBdbTreeIterator<T>    operator -- (int);
    QxtBdbTreeIterator<T> &  operator -= (int j);


    QxtBdbTreeIterator<T>   append(const T & t);
    QxtBdbTreeIterator<T>   prepend(const T & t);
    QxtBdbTreeIterator<T>   erase();

protected:
    quint64 level() const;
    void invalidate()
    {
        if (dbc)
            dbc->c_close(dbc);
        dbc = 0;
    }
private:
    friend class QxtBdbTree<T>;
    QxtBdbTreeIterator(BerkeleyDB::DBC*, QxtBdb * p);
    QxtBdbTreeIterator<T> croot() const;


    int meta_id;
    QxtBdb * db;
    BerkeleyDB::DBC *dbc;

    bool root;


    /*won't work. no support in bdb*/
    bool operator== (const QxtBdbTreeIterator<T> & other) const
    {
        return false;
    }
    bool operator!= (const QxtBdbTreeIterator<T> & other) const
    {
        return false;
    }



};










template<class T>
QxtBdbTreeIterator<T>::QxtBdbTreeIterator()
{
    dbc = 0;
    db = 0;
    root = false;
    meta_id = qMetaTypeId<T>();
}
template<class T>
QxtBdbTreeIterator<T>::~QxtBdbTreeIterator()
{
    invalidate();
}

template<class T>
QxtBdbTreeIterator<T>::QxtBdbTreeIterator(const QxtBdbTreeIterator<T> & other)
{
    dbc = 0;
    db = 0;
    root = false;
    meta_id = qMetaTypeId<T>();

    db = other.db;
    root = other.root;


    if (other.dbc)
    {
        other.dbc->c_dup(other.dbc, &dbc, DB_POSITION);
    }
}

template<class T>
QxtBdbTreeIterator<T>::QxtBdbTreeIterator(BerkeleyDB::DBC* dbc, QxtBdb * p)
{
    this->dbc = dbc;
    db = p;
    root = false;
    meta_id = qMetaTypeId<T>();
}
template<class T>
QxtBdbTreeIterator<T> QxtBdbTreeIterator<T>::croot() const
{
    QxtBdbTreeIterator<T> k = *this;
    k.invalidate();
    k.root = true;
    return k;
}

template<class T>
QxtBdbTreeIterator<T> & QxtBdbTreeIterator<T>::operator= (const QxtBdbTreeIterator<T> & other)
{
    invalidate();

    db = other.db;
    root = other.root;


    if (other.dbc)
    {
        other.dbc->c_dup(other.dbc, &dbc, DB_POSITION);
    }
    return *this;
}


template<class T>
bool QxtBdbTreeIterator<T>::isValid() const
{
    if (root)
        return true;

    return (dbc != 0 && db != 0);
}

template<class T>
QxtBdbTreeIterator<T>::operator T() const
{
    return value();
}

template<class T>
T QxtBdbTreeIterator<T>::value() const
{

    if (!dbc)
    {
        qWarning("QxtBdbTreeIterator<T>::value() on invalid iterator ");
        return T();
    }

    BerkeleyDB::DBT dbkey, dbvalue;
    memset(&dbkey, 0, sizeof(BerkeleyDB::DBT));
    memset(&dbvalue, 0, sizeof(BerkeleyDB::DBT));

    dbvalue.ulen = 0;
    dbvalue.flags = DB_DBT_USERMEM;

    dbkey.ulen = 0;
    dbkey.flags = DB_DBT_USERMEM;

    int ret = dbc->c_get(dbc, &dbkey, &dbvalue, DB_CURRENT);
    if (ret != DB_BUFFER_SMALL)
        return T();


    dbvalue.ulen = dbvalue.size;
    dbvalue.data =::malloc(dbvalue.size);

    dbkey.ulen = dbkey.size;
    dbkey.data =::malloc(dbkey.size);

    ret = dbc->c_get(dbc, &dbkey, &dbvalue, DB_CURRENT);
    T t = QxtBdb::qxtMetaLoad<T>((quint64*)dbvalue.data + 1, dbvalue.size - sizeof(quint64));
    ::free(dbkey.data);
    ::free(dbvalue.data);

    if (ret != 0)
        return T();

    return t;

}




template<class T>
bool QxtBdbTreeIterator<T>::setValue(T t)
{
    if (!dbc)
    {
        qWarning("QxtBdbTreeIterator<T>::setValue() on invalid iterator ");
        return false;
    }
    BerkeleyDB::DBT dbkey, dbvalue;
    ::memset(&dbkey, 0, sizeof(BerkeleyDB::DBT));
    ::memset(&dbvalue, 0, sizeof(BerkeleyDB::DBT));

    char uselesszero = 0;
    dbkey.data = &uselesszero;
    dbkey.size = sizeof(char);
    dbkey.ulen = 0;
    dbkey.flags = DB_DBT_USERMEM;

    quint64 mylevel = level();
    QByteArray d = QxtBdb::qxtMetaSave<T>(t);


    dbvalue.size = sizeof(quint64) + d.size();
    dbvalue.ulen = dbvalue.size;
    dbvalue.data =::malloc(dbvalue.size);
    ::memcpy(dbvalue.data, &mylevel, sizeof(quint64));
    ::memcpy((quint64*)dbvalue.data + 1, d.data(), d.size());
    dbvalue.flags = DB_DBT_USERMEM;

    int ret = 234525;
    ret = dbc->c_put(dbc, &dbkey, &dbvalue, DB_CURRENT);
    ::free(dbvalue.data);

    if (ret != 0)
        return false;
    return true;
}



template<class T>
QxtBdbTreeIterator<T>    QxtBdbTreeIterator<T>::parent() const
{
    if (root)
        return croot();
    if (!dbc)
        return QxtBdbTreeIterator<T>();

    QxtBdbTreeIterator<T> d(*this);
    quint64 lvl = level();

    Q_FOREVER
    {
#if DB_VERSION_MINOR > 5

        if (!d.db->get((void*)0, 0, 0, 0, DB_PREV_DUP, d.dbc))
#else
        if (!d.db->get((void*)0, 0, 0, 0, DB_PREV, d.dbc))
#endif
            return croot();
        if (d.level() == lvl - 1)
            break;
    }

    return d;
}

template<class T>
QxtBdbTreeIterator<T>    QxtBdbTreeIterator<T>::next() const
{
    if (root)
        return QxtBdbTreeIterator<T>();
    return QxtBdbTreeIterator<T>(*this) + 1;
}

template<class T>
QxtBdbTreeIterator<T>    QxtBdbTreeIterator<T>::previous() const
{
    if (root)
        return QxtBdbTreeIterator<T>();
    return QxtBdbTreeIterator<T>(*this) - 1;
}

template<class T>
QxtBdbTreeIterator<T>    QxtBdbTreeIterator<T>::child() const
{
    QxtBdbTreeIterator<T> d(*this);


    if (root)
    {
        BerkeleyDB::DBC *cursor;
        db->db->cursor(db->db, NULL, &cursor, 0);
        d = QxtBdbTreeIterator<T>(cursor, db);

        if (!d.db->get((void*)0, 0, 0, 0, DB_FIRST, d.dbc))
        {
            return QxtBdbTreeIterator<T>();
        }
    }
    else
    {
        if (!dbc)
            return QxtBdbTreeIterator<T>();


        quint64 lvl = level();
        if (!d.db->get((void*)0, 0, 0, 0, DB_NEXT_DUP, d.dbc))
        {
            return QxtBdbTreeIterator<T>();
        }
        if (d.level() != lvl + 1)
            return QxtBdbTreeIterator<T>();
    }

    return d;
}

template<class T>
QxtBdbTreeIterator<T>    QxtBdbTreeIterator<T>::operator + (int j) const
{
    if(j < 0)
        return this->operator-(j*(-1));

    QxtBdbTreeIterator<T> d(*this);
    for (int i = 0;i < j;i++)
        ++d;
    return d;
}


template<class T>
QxtBdbTreeIterator<T> &  QxtBdbTreeIterator<T>::operator ++ ()
{
    if (root)
    {
        invalidate();
        return *this;
    }

    if (!dbc)
        return *this;

    quint64 before = level();
    Q_FOREVER
    {

        if (!db->get((void*)0, 0, 0, 0, DB_NEXT_DUP, dbc))
        {
            invalidate();
            break;
        }
        if (before == level())
            break;
        if (before > level())
        {
            invalidate();
            break;
        }
    }

    return *this;
}


template<class T>
QxtBdbTreeIterator<T>    QxtBdbTreeIterator<T>::operator ++ (int)
{
    QxtBdbTreeIterator<T> d(*this);
    operator++();
    return d;
}


template<class T>
QxtBdbTreeIterator<T> &  QxtBdbTreeIterator<T>::operator += (int j)
{
    if(j < 0)
        return this->operator-=(j*(-1));

    for (int i = 0;i < j;i++)
        operator++();
    return *this;
}


template<class T>
QxtBdbTreeIterator<T>    QxtBdbTreeIterator<T>::operator - (int j) const
{
    if(j < 0)
        return this->operator+(j*(-1));

    QxtBdbTreeIterator<T> d(*this);
    for (int i = 0;i < j;i++)
        --d;
    return d;
}


template<class T>
QxtBdbTreeIterator<T> &  QxtBdbTreeIterator<T>::operator -- ()
{
    if (root)
        return *this;

    if (!dbc)
        return *this;


    int lvl = level();


#if DB_VERSION_MINOR > 5
    do
    {
        if (!db->get((void*)0, 0, 0, 0, DB_PREV_DUP, dbc))
        {
            invalidate();
            break;
        }
    }
    while (lvl != level());
#else
    do
    {
        if (!db->get((void*)0, 0, 0, 0, DB_PREV, dbc))
        {
            invalidate();
            break;
        }
    }
    while (lvl != level());
#endif

    return *this;
}


template<class T>
QxtBdbTreeIterator<T>    QxtBdbTreeIterator<T>::operator -- (int)
{
    QxtBdbTreeIterator<T> d(*this);
    operator--();
    return d;
}


template<class T>
QxtBdbTreeIterator<T> &  QxtBdbTreeIterator<T>::operator -= (int j)
{
    if(j < 0)
        return this->operator+=(j*(-1));

    for (int i = 0;i < j;i++)
        operator--();
    return *this;
}


template<class T>
QxtBdbTreeIterator<T>   QxtBdbTreeIterator<T>::append(const T & t)
{
    Q_ASSERT(isValid());
    BerkeleyDB::DBT dbkey, dbvalue;
    ::memset(&dbkey, 0, sizeof(BerkeleyDB::DBT));
    ::memset(&dbvalue, 0, sizeof(BerkeleyDB::DBT));

    char uselesszero = 0;
    dbkey.data = &uselesszero;
    dbkey.size = sizeof(char);
    dbkey.ulen = 0;
    dbkey.flags = DB_DBT_USERMEM;


    quint64 newlevel = level() + 1;


    QByteArray d = QxtBdb::qxtMetaSave<T>(t);


    dbvalue.size = sizeof(quint64) + d.size();
    dbvalue.ulen = dbvalue.size;
    dbvalue.data =::malloc(dbvalue.size);
    ::memcpy(dbvalue.data, &newlevel, sizeof(quint64));
    ::memcpy((quint64*)dbvalue.data + 1, d.data(), d.size());
    dbvalue.flags = DB_DBT_USERMEM;

    int ret = 234525;


    QxtBdbTreeIterator<T> e = *this;
    if (dbc)
    {
        while (e.db->get((void*)0, 0, 0, 0, DB_NEXT_DUP, e.dbc))
        {
            if (e.level() <= level())
            {
#if DB_VERSION_MINOR > 5
                e.db->get((void*)0, 0, 0, 0, DB_PREV_DUP, e.dbc);
#else
                e.db->get((void*)0, 0, 0, 0, DB_PREV, e.dbc);
#endif
                break;
            }
        }
        ret = dbc->c_put(e.dbc, &dbkey, &dbvalue, DB_AFTER);
    }
    else if (root)
    {
        ret = db->db->put(db->db, NULL, &dbkey, &dbvalue, NULL);
        BerkeleyDB::DBC *cursor;
        db->db->cursor(db->db, NULL, &cursor, 0);
        if (db->get((void*)0, 0, 0, 0, DB_LAST, cursor))
            e = QxtBdbTreeIterator<T>(cursor, db);

    }
    ::free(dbvalue.data);

    if (ret != 0)
    {
        qWarning("QxtBdbTreeIterator::append failed %i", ret);
        return QxtBdbTreeIterator<T>();
    }
    return e;
}


template<class T>
QxtBdbTreeIterator<T>   QxtBdbTreeIterator<T>::prepend(const T & t)
{
    if (!dbc)
        return QxtBdbTreeIterator<T>();
    QxtBdbTreeIterator<T> e(*this);

    BerkeleyDB::DBT dbkey, dbvalue;
    ::memset(&dbkey, 0, sizeof(BerkeleyDB::DBT));
    ::memset(&dbvalue, 0, sizeof(BerkeleyDB::DBT));

    char uselesszero = 0;
    dbkey.data = &uselesszero;
    dbkey.size = sizeof(char);
    dbkey.ulen = 0;
    dbkey.flags = DB_DBT_USERMEM;


    quint64 newlevel = level();


    QByteArray d = QxtBdb::qxtMetaSave<T>(t);


    dbvalue.size = sizeof(quint64) + d.size();
    dbvalue.ulen = dbvalue.size;
    dbvalue.data =::malloc(dbvalue.size);
    ::memcpy(dbvalue.data, &newlevel, sizeof(quint64));
    ::memcpy((quint64*)dbvalue.data + 1, d.data(), d.size());
    dbvalue.flags = DB_DBT_USERMEM;

    int ret = 234525;
    ret = e.dbc->c_put(e.dbc, &dbkey, &dbvalue, DB_BEFORE);
    ::free(dbvalue.data);

    if (ret != 0)
        return QxtBdbTreeIterator<T>();
    return e;
}


template<class T>
quint64 QxtBdbTreeIterator<T>::level() const
{
    if (!dbc)
        return 0;

    BerkeleyDB::DBT dbkey, dbvalue;
    memset(&dbkey, 0, sizeof(BerkeleyDB::DBT));
    memset(&dbvalue, 0, sizeof(BerkeleyDB::DBT));

    dbvalue.size = 0;
    dbvalue.size = 0;
    dbvalue.ulen = 0;
    dbvalue.flags = DB_DBT_USERMEM;


    dbkey.ulen = 0;
    dbkey.flags = DB_DBT_USERMEM;

    int ret = dbc->c_get(dbc, &dbkey, &dbvalue, DB_CURRENT);
    if (ret != DB_BUFFER_SMALL)
        qFatal("QxtBdbTreeIterator::level() %s", qPrintable(QxtBdb::dbErrorCodeToString(ret)));
    dbvalue.ulen = dbvalue.size;
    dbvalue.data =::malloc(dbvalue.size);

    dbkey.ulen = dbkey.size;
    dbkey.data =::malloc(dbkey.size);

    ret = dbc->c_get(dbc, &dbkey, &dbvalue, DB_CURRENT);

    quint64 lvl;
    ::memcpy(&lvl, dbvalue.data, sizeof(quint64));
    ::free(dbkey.data);
    ::free(dbvalue.data);

    if (ret != 0)
        return 0;

    return lvl;
}

template<class T>
QxtBdbTreeIterator<T> QxtBdbTreeIterator<T>::erase()
{
    Q_ASSERT(isValid());
    quint64 before = level();
    Q_FOREVER
    {

        int ret = dbc->c_del(dbc, 0);
        if (ret != 0)
        {
            qWarning("QxtBdbTreeIterator<T>::erase() failed %s", qPrintable(QxtBdb::dbErrorCodeToString(ret)));
            return QxtBdbTreeIterator<T>();
        }


        if (!db->get((void*)0, 0, 0, 0, DB_NEXT_DUP, dbc))
            return *this;
        if (level() <= before)
            return *this;

    }
    Q_ASSERT(false);
    return *this;
}


#endif
