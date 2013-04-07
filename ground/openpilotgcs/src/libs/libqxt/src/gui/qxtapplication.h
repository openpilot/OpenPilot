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
#ifndef QXTAPPLICATION_H
#define QXTAPPLICATION_H

#include <QApplication>
#include "qxtglobal.h"

class QxtApplicationPrivate;
class QxtNativeEventFilter;

#define qxtApp (QxtApplication::instance())

class QXT_GUI_EXPORT QxtApplication : public QApplication
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtApplication)

public:
    QxtApplication(int& argc, char** argv);
    QxtApplication(int& argc, char** argv, bool GUIenabled);
    QxtApplication(int& argc, char** argv, Type type);
#if defined(Q_WS_X11)
    explicit QxtApplication(Display* display, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0);
    QxtApplication(Display* display, int& argc, char** argv, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0);
#endif // Q_WS_X11
    virtual ~QxtApplication();

    void installNativeEventFilter(QxtNativeEventFilter* filter);
    void removeNativeEventFilter(QxtNativeEventFilter* filter);

#if defined(Q_WS_X11)
    virtual bool x11EventFilter(XEvent* event);
#elif defined(Q_WS_WIN)
    virtual bool winEventFilter(MSG* msg, long* result);
#elif defined(Q_WS_MAC)
    virtual bool macEventFilter(EventHandlerCallRef caller, EventRef event);
#endif // Q_WS_*

    inline static QxtApplication* instance()
    {
        return qobject_cast<QxtApplication*>(QApplication::instance());
    }
};

#endif // QXTAPPLICATION_H
