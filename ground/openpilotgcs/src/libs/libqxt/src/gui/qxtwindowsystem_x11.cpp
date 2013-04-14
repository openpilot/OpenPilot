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
#include "qxtwindowsystem.h"
#include <QLibrary>
#include <QX11Info>
#include <X11/Xutil.h>

static void qxt_getWindowProperty(Window wid, Atom prop, int maxlen, Window** data, int* count)
{
    Atom type = 0;
    int format = 0;
    unsigned long after = 0;
    XGetWindowProperty(QX11Info::display(), wid, prop, 0, maxlen / 4, False, AnyPropertyType,
                       &type, &format, (unsigned long*) count, &after, (unsigned char**) data);
}

static QRect qxt_getWindowRect(Window wid)
{
    QRect rect;
    XWindowAttributes attr;
    if (XGetWindowAttributes(QX11Info::display(), wid, &attr))
    {
        rect = QRect(attr.x, attr.y, attr.width, attr.height);
    }
    return rect;
}

WindowList QxtWindowSystem::windows()
{
    static Atom net_clients = 0;
    if (!net_clients)
        net_clients = XInternAtom(QX11Info::display(), "_NET_CLIENT_LIST_STACKING", True);

    int count = 0;
    Window* list = 0;
    qxt_getWindowProperty(QX11Info::appRootWindow(), net_clients, 1024 * sizeof(Window), &list, &count);

    WindowList res;
    for (int i = 0; i < count; ++i)
        res += list[i];
    XFree(list);
    return res;
}

WId QxtWindowSystem::activeWindow()
{
    Window focus;
    int revert = 0;
    Display* display = QX11Info::display();
    XGetInputFocus(display, &focus, &revert);
    return focus;
}

WId QxtWindowSystem::findWindow(const QString& title)
{
    Window result = 0;
    WindowList list = windows();
    foreach(const Window &wid, list)
    {
        if (windowTitle(wid) == title)
        {
            result = wid;
            break;
        }
    }
    return result;
}

WId QxtWindowSystem::windowAt(const QPoint& pos)
{
    Window result = 0;
    WindowList list = windows();
    for (int i = list.size() - 1; i >= 0; --i)
    {
        WId wid = list.at(i);
        if (windowGeometry(wid).contains(pos))
        {
            result = wid;
            break;
        }
    }
    return result;
}

QString QxtWindowSystem::windowTitle(WId window)
{
    QString name;
    char* str = 0;
    if (XFetchName(QX11Info::display(), window, &str))
    {
        name = QString::fromLatin1(str);
    }
    XFree(str);
    return name;
}

QRect QxtWindowSystem::windowGeometry(WId window)
{
    QRect rect = qxt_getWindowRect(window);

    Window root = 0;
    Window parent = 0;
    Window* children = 0;
    unsigned int count = 0;
    Display* display = QX11Info::display();
    if (XQueryTree(display, window, &root, &parent, &children, &count))
    {
        window = parent; // exclude decoration
        XFree(children);
        while (window != 0 && XQueryTree(display, window, &root, &parent, &children, &count))
        {
            XWindowAttributes attr;
            if (parent != 0 && XGetWindowAttributes(display, parent, &attr))
                rect.translate(QRect(attr.x, attr.y, attr.width, attr.height).topLeft());
            window = parent;
            XFree(children);
        }
    }
    return rect;
}

typedef struct {
    Window  window;     /* screen saver window - may not exist */
    int     state;      /* ScreenSaverOff, ScreenSaverOn, ScreenSaverDisabled*/
    int     kind;       /* ScreenSaverBlanked, ...Internal, ...External */
    unsigned long    til_or_since;   /* time til or since screen saver */
    unsigned long    idle;      /* total time since last user input */
    unsigned long   eventMask; /* currently selected events for this client */
} XScreenSaverInfo;

typedef XScreenSaverInfo* (*XScreenSaverAllocInfo)();
typedef Status (*XScreenSaverQueryInfo)(Display* display, Drawable* drawable, XScreenSaverInfo* info);

static XScreenSaverAllocInfo _xScreenSaverAllocInfo = 0;
static XScreenSaverQueryInfo _xScreenSaverQueryInfo = 0;

uint QxtWindowSystem::idleTime()
{
    static bool xssResolved = false;
    if (!xssResolved) {
        QLibrary xssLib(QLatin1String("Xss"), 1);
        if (xssLib.load()) {
            _xScreenSaverAllocInfo = (XScreenSaverAllocInfo) xssLib.resolve("XScreenSaverAllocInfo");
            _xScreenSaverQueryInfo = (XScreenSaverQueryInfo) xssLib.resolve("XScreenSaverQueryInfo");
            xssResolved = true;
        }
    }

    uint idle = 0;
    if (xssResolved)
    {
        XScreenSaverInfo* info = _xScreenSaverAllocInfo();
        const int screen = QX11Info::appScreen();
        Qt::HANDLE rootWindow = QX11Info::appRootWindow(screen);
        _xScreenSaverQueryInfo(QX11Info::display(), (Drawable*) rootWindow, info);
        idle = info->idle;
        XFree(info);
    }
    return idle;
}
