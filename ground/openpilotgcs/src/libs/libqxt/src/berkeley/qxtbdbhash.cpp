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
#include "qxtbdbhash.h"



/*!
    \class QxtBdbHash
    \inmodule QxtBerkeley
    \brief The QxtBdbHash class is a template class that provides key/value access to a berkeley db file.

    Both value and key must be registered with the qt meta system.
    You may not touch the file while a QxtBdbHash instance is running on it.

    examples usage:
    \code
    QxtBdbHash<qreal,QStringList> db("test.db");
    db.insert(344.4,QStringList<<"HI"<<":)");
    qDebug()<<db[344.4];
    \endcode


    There is an extensive example in /examples/berkeley/adressbook.


    All functions of this class are thread safe.
    Calling open() multiple times is undefined.
    An iterator may only be used from one thread at once, but you can have multiple iterators.

    TODO: {implicitshared}
    \sa QxtBdbHashIterator
*/

/*!
    \fn void QxtBdbHash::QxtBdbHash()

    Constructs an invalid QxtBdbHash
*/

/*!
    \fn void QxtBdbHash::QxtBdbHash(QString file)

    Constructs a QxtBdbHash, and opens the \a file specified as its database.
*/


/*!
    \fn bool QxtBdbHash::open(QString file)

    Opens the specified \a file.

    Returns \c true on success and \c false on failure.
    \bold {Note:} A sanity check is performed before opening the file.
*/

/*!
    \fn void QxtBdbHash::clear()

    Erase all records. This does not delete the underlying file.
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL> QxtBdbHash::begin()

    Returns an iterator to the first key,value pair.
    \sa QxtBdbHashIterator
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL> QxtBdbHash::end()

    Return an iterator to the last key,value pair
    \sa QxtBdbHashIterator
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL> QxtBdbHash::find ( const KEY & key )

    Returns an iterator to the position of the pair identified by \a key
    \sa QxtBdbHashIterator
*/

/*!
    \fn bool QxtBdbHash::contains ( const KEY & key ) const

    Returns \c true if there is a record for the specified \a key.
*/

/*!
    \fn bool QxtBdbHash::remove ( const KEY & key )

    Removes all records with the specified \a key. Returns \c true on success and \c false on failure.
*/

/*!
    \fn bool QxtBdbHash::insert( KEY key, VAL value )

    Inserts a record with the specified \a key / \a value combination. Replaces any record with the same key.
    \bold {Note:} When working with iterators, keep in mind that inserting pairs, works reverse to the iteration.
*/

/*!
    \fn const VAL QxtBdbHash::value( const KEY & key ) const

    Returns the value associated with the specified \a key, or a default contructed value, if no such key exists.
*/

/*!
    \fn const VAL QxtBdbHash::operator[] ( const KEY & key ) const

    Same as value()
*/

/*!
    \fn bool QxtBdbHash::flush()

    Flushes the underlying DB file. All changes are synced to disk.
*/








/*!
    \class QxtBdbHashIterator
    \inmodule QxtBerkeley
    \brief The QxtBdbHashIterator class provides a fast iterator over a QxtBdbHash


    \sa QxtBdbHash


    TODO: {implicitshared}
*/

/*!
    \fn void QxtBdbHashIterator<KEY,VAL>::QxtBdbHashIterator()

    Constructs an invalid QxtBdbHashIterator
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL>::QxtBdbHashIterator(const QxtBdbHashIterator<KEY,VAL> & other)

    Copy constructor
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL> & QxtBdbHashIterator<KEY,VAL>::operator= ( const QxtBdbHashIterator<KEY,VAL> & other )

    Assignment operator
*/

/*!
    \fn bool QxtBdbHashIterator<KEY,VAL>::isValid() const

    Returns \c true if the iterator is valid.
    Invalid iterators are unusable and accessing any function will fail.
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL>::operator KEY() const

    Convertion operator to the current value.

    \sa key()
*/

/*!
    \fn KEY QxtBdbHashIterator<KEY,VAL>::key() const

    Returns the current key.

    \sa value()
*/

/*!
    \fn VAL QxtBdbHashIterator<KEY,VAL>::value() const

    Returns the current value.

    \sa key()
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL> QxtBdbHashIterator<KEY,VAL>::operator + ( int j ) const

    Returns an iterator to the item at \a j positions forward from this iterator.
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL> &  QxtBdbHashIterator<KEY,VAL>::operator ++ ()

    The prefix ++ operator (++i) advances the iterator to the next item in the hash and returns an iterator to the new current item.
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL>    QxtBdbHashIterator<KEY,VAL>::operator ++ (int)

    The postfix ++ operator (i++) advances the iterator to the next item in the hash and returns an iterator to the previously current item.
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL> &  QxtBdbHashIterator<KEY,VAL>::operator += ( int j )

    Advances the iterator by \a j items.
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL>    QxtBdbHashIterator<KEY,VAL>::operator - ( int j ) const

    Returns an iterator to the item at \a j positions backward from this iterator.
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL> &  QxtBdbHashIterator<KEY,VAL>::operator -- ()

    The prefix -- operator (--i) makes the preceding item current and returns an iterator pointing to the new current item.
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL>    QxtBdbHashIterator<KEY,VAL>::operator -- (int)

    The postfix -- operator (i--) makes the preceding item current and returns an iterator pointing to the previously current item.
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL> &  QxtBdbHashIterator<KEY,VAL>::operator -= ( int j )

    Makes the iterator go back by \a j items.
*/

/*!
    \fn QxtBdbHashIterator<KEY,VAL> QxtBdbHashIterator<KEY,VAL>::erase ()

    Removes the (key, value) pair associated with the iterator from the hash, and returns an iterator to the next item in the hash.

    This instance is invalid then, and cannot be used further.
*/
