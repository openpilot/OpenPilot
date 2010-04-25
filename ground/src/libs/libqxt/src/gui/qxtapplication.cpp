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
#include "qxtapplication.h"
#include "qxtapplication_p.h"

/*!
    \class QxtApplication
    \inmodule QxtGui
    \brief The QxtApplication class is an extended QApplication with support for native event filters.

    QxtApplication lets you install native event filters. This gives an
    easy and straightforward access to platform specific native events.

    \sa QxtNativeEventFilter
 */

/*!
    \fn QxtApplication::instance()

    Returns a pointer to the instance of the application object.

    A convenience macro \l qxtApp is also available.
 */

/*!
    \macro qxtApp
    \relates QxtApplication
    \inmodule QxtGui

    A global pointer referring to the unique application object. It is
    equivalent to the pointer returned by QxtApplication::instance().

    \sa QxtApplication::instance()
*/

/*!
    Constructs a new QxtApplication with \a argc and \a argv.

    \sa QApplication::QApplication()
 */
QxtApplication::QxtApplication(int& argc, char** argv)
        : QApplication(argc, argv)
{
}

/*!
    Constructs a new QxtApplication with \a argc, \a argv and \a GUIenabled.

    \sa QApplication::QApplication()
 */
QxtApplication::QxtApplication(int& argc, char** argv, bool GUIenabled)
        : QApplication(argc, argv, GUIenabled)
{
}

/*!
    Constructs a new QxtApplication with \a argc, \a argv and \a type.

    \sa QApplication::QApplication()
 */
QxtApplication::QxtApplication(int& argc, char** argv, Type type)
        : QApplication(argc, argv, type)
{
}

/*!
    Destructs the QxtApplication.

    \sa QApplication::~QApplication()
 */
QxtApplication::~QxtApplication()
{
}

/*!
    Installs a native event \a filter.

    A native event filter is an object that receives all native events before they reach
    the application object. The filter can either stop the native event or forward it to
    the application object. The filter receives native events via its platform specific
    native event filter function. The native event filter function must return \c true
    if the event should be filtered, (i.e. stopped); otherwise it must return \c false.

    If multiple native event filters are installed, the filter that was installed last
    is activated first.

    \sa removeNativeEventFilter()
*/
void QxtApplication::installNativeEventFilter(QxtNativeEventFilter* filter)
{
    if (!filter)
        return;

    qxt_d().nativeFilters.removeAll(filter);
    qxt_d().nativeFilters.prepend(filter);
}

/*!
    Removes a native event \a filter. The request is ignored if such a native
    event filter has not been installed.

    \sa installNativeEventFilter()
*/
void QxtApplication::removeNativeEventFilter(QxtNativeEventFilter* filter)
{
    qxt_d().nativeFilters.removeAll(filter);
}
