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
#include "qxtbdb.h"
#include <QFileInfo>
#include <QBuffer>
#include <QDataStream>
#include <QVariant>





static void qxtBDBDatabaseErrorHandler(const  BerkeleyDB::DB_ENV*, const char* a, const char* b)
{
    qDebug("QxtBDBDatabase: %s, %s", a, b);
}


QxtBdb::QxtBdb()
{
    isOpen = false;
    if (db_create(&db, NULL, 0) != 0)
        qFatal("db_create failed");
    db->set_errcall(db, qxtBDBDatabaseErrorHandler);

}
QxtBdb::~QxtBdb()
{
    db->close(db, 0);
}




bool QxtBdb::open(QString path, OpenFlags f)
{
    Q_ASSERT(!isOpen);

    if (QFileInfo(path).exists())
    {

        BerkeleyDB::DB * tdb;
        if (db_create(&tdb, NULL, 0) != 0)
            qFatal("db_create failed");

        if (tdb->verify(tdb, qPrintable(path), NULL, NULL, 0) == DB_VERIFY_BAD)
            qCritical("QxtBdb::open Database '%s' is corrupted.", qPrintable(path));
    }



    int flags = 0;
    if (f&CreateDatabase)
        flags |= DB_CREATE;

    if (f&ReadOnly)
        flags |= DB_RDONLY;

    if (f&LockFree)
        flags |= DB_THREAD;




    isOpen = (db->open(db,      /* DB structure pointer */
                       NULL,       /* Transaction pointer */
                       qPrintable(path), /* On-disk file that holds the database. */
                       NULL,       /* Optional logical database name */
                       BerkeleyDB::DB_BTREE,   /* Database access method */
                       flags,  /* Open flags */
                       0)
              == 0);

    return isOpen;


}


QxtBdb::OpenFlags QxtBdb::openFlags()
{
    if (!isOpen)
        return 0;
    OpenFlags f;

    BerkeleyDB::u_int32_t open_flags;
    db->get_open_flags(db, &open_flags);



    if (open_flags&DB_CREATE)
        f |= CreateDatabase;
    if (open_flags&DB_RDONLY)
        f |= ReadOnly;
    if (open_flags&DB_THREAD)
        f |= LockFree;


    return f;
}




bool QxtBdb::flush()
{
    if (!isOpen)
        return false;
    return (db->sync(db, 0) == 0);
}

/*!
low level get function.
serialised key and value with the given meta ids.
always reads <b>and</b> writes both key and value, if given.
use this when doing operations that require the key to be read out of the db.
*/
bool QxtBdb::get(void* key, int keytype, void* value, int valuetype, BerkeleyDB::u_int32_t flags, BerkeleyDB::DBC * cursor) const
{

    BerkeleyDB::DBT dbkey, dbvalue;
    ::memset(&dbkey, 0, sizeof(BerkeleyDB::DBT));
    ::memset(&dbvalue, 0, sizeof(BerkeleyDB::DBT));



    if (key)
    {
        QByteArray d_key;
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        if (!QMetaType::save(s, keytype, key))
            qCritical("QMetaType::save failed. is your key registered with the QMetaType?");
        buffer.close();
        dbkey.size = d_key.size();
        dbkey.data = ::malloc(d_key.size());
        ::memcpy(dbkey.data, d_key.data(), d_key.size());
    }

    if (value)
    {
        QByteArray d_value;
        QBuffer buffer(&d_value);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        if (!QMetaType::save(s, valuetype, value))
            qCritical("QMetaType::save failed. is your value registered with the QMetaType?");
        buffer.close();
        dbvalue.size = d_value.size();
        dbvalue.data = ::malloc(d_value.size());
        ::memcpy(dbvalue.data, d_value.data(), d_value.size());
    }




    dbvalue.ulen = 0;
    dbvalue.flags = DB_DBT_USERMEM;

    dbkey.ulen = 0;
    dbkey.flags = DB_DBT_USERMEM;

    int ret;

    if (cursor)
        ret = cursor->c_get(cursor, &dbkey, &dbvalue, flags);
    else
        ret = db->get(db, NULL, &dbkey, &dbvalue, flags);

    if (ret != DB_BUFFER_SMALL)
    {
        ::free(dbvalue.data);
        ::free(dbkey.data);
        return (ret == 0);
    }
    dbvalue.ulen = dbvalue.size;
    dbvalue.data =::malloc(dbvalue.size);

    dbkey.ulen = dbkey.size;
    dbkey.data =::malloc(dbkey.size);

    if (cursor)
        ret = cursor->c_get(cursor, &dbkey, &dbvalue, flags);
    else
        ret = db->get(db, NULL, &dbkey, &dbvalue, flags);

    QByteArray  d_value = QByteArray::fromRawData((const char*) dbvalue.data, dbvalue.size);
    QByteArray  d_key = QByteArray::fromRawData((const char*) dbkey.data,  dbkey.size);


    if (ret != 0)
    {
        ::free(dbvalue.data);
        ::free(dbkey.data);
        return false;
    }



    if (key)
    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::ReadOnly);
        QDataStream s(&buffer);
        if (!QMetaType::load(s, keytype, key))
            qCritical("QMetaType::load failed. is your key registered with the QMetaType?");
        buffer.close();
    }
    if (value)
    {
        QBuffer buffer(&d_value);
        buffer.open(QIODevice::ReadOnly);
        QDataStream s(&buffer);
        if (!QMetaType::load(s, valuetype, value))
            qCritical("QMetaType::load failed. is your value registered with the QMetaType?");
        buffer.close();
    }

    ::free(dbvalue.data);
    ::free(dbkey.data);

    return true;
}






/*!
low level get function.
serialised key and value with the given meta ids.
doesn't write to the key. use this when doing operations that require the key to be passed.
*/
bool QxtBdb::get(const void* key, int keytype, void* value, int valuetype, BerkeleyDB::u_int32_t flags, BerkeleyDB::DBC * cursor) const
{

    BerkeleyDB::DBT dbkey, dbvalue;
    memset(&dbkey, 0, sizeof(BerkeleyDB::DBT));
    memset(&dbvalue, 0, sizeof(BerkeleyDB::DBT));

    if (key)
    {
        QByteArray d_key;
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        if (!QMetaType::save(s, keytype, key))
            qCritical("QMetaType::save failed. is your key registered with the QMetaType?");
        buffer.close();
        dbkey.size = d_key.size();
        dbkey.data = ::malloc(d_key.size());
        ::memcpy(dbkey.data, d_key.data(), d_key.size());
    }




    if (value)
    {
        QByteArray d_value;
        QBuffer buffer(&d_value);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        if (!QMetaType::save(s, valuetype, value))
            qCritical("QMetaType::save failed. is your value registered with the QMetaType?");
        buffer.close();
        dbvalue.size = d_value.size();
        dbvalue.data = ::malloc(d_value.size());
        ::memcpy(dbvalue.data, d_value.data(), d_value.size());
    }




    dbvalue.ulen = 0;
    dbvalue.flags = DB_DBT_USERMEM;

    dbkey.ulen = 0;
    dbkey.flags = 0;
    dbkey.flags = DB_DBT_USERMEM;  //it's my memory, ffs. stop deleting it! >_<

    int ret;

    if (cursor)
        ret = cursor->c_get(cursor, &dbkey, &dbvalue, flags);
    else
        ret = db->get(db, NULL, &dbkey, &dbvalue, flags);

    if (ret != DB_BUFFER_SMALL)
    {
        ::free(dbvalue.data);
        ::free(dbkey.data);
        return (ret == 0);
    }
    dbvalue.ulen = dbvalue.size;
    dbvalue.data =::malloc(dbvalue.size);


    if (cursor)
        ret = cursor->c_get(cursor, &dbkey, &dbvalue, flags);
    else
        ret = db->get(db, NULL, &dbkey, &dbvalue, flags);

    QByteArray  d_value((const char*) dbvalue.data, dbvalue.size);
    QByteArray  d_key((const char*) dbkey.data,  dbkey.size);

    ::free(dbvalue.data);
    ::free(dbkey.data);

    Q_ASSERT_X(ret != DB_BUFFER_SMALL, Q_FUNC_INFO, "QxtBdb::get bdb inists on retriving the key for this operation. You need to specify a non const key. (or just specify a non const void* with the value of 0, i'll delete the key for you after bdb fetched it, so you don't need to bother)");

    if (ret != 0)
    {
        return false;
    }




    if (value)
    {
        QBuffer buffer(&d_value);
        buffer.open(QIODevice::ReadOnly);
        QDataStream s(&buffer);
        if (!QMetaType::load(s, valuetype, value))
            qCritical("QMetaType::load failed. is your value registered with the QMetaType?");

        buffer.close();

    }


    return true;
}



QString QxtBdb::dbErrorCodeToString(int e)
{
    switch (e)
    {
    case DB_LOCK_DEADLOCK:
        return QString("Dead locked (%1)").arg(e);
    case DB_SECONDARY_BAD:
        return QString("Bad Secondary index (%1)").arg(e);
    case DB_RUNRECOVERY:
        return QString("Database corrupted. Run Recovery. (%1)").arg(e);

    default:
        return QString("Unknown error %1").arg(e);
    };
}

