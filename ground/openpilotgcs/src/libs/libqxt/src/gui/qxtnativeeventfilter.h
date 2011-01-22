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
#ifndef QXTNATIVEEVENTFILTER_H
#define QXTNATIVEEVENTFILTER_H

#include "qxtapplication.h"

class QxtNativeEventFilter
{
public:
    virtual ~QxtNativeEventFilter()
    {
        qxtApp->removeNativeEventFilter(this);
    }

#if defined(Q_WS_X11) || defined(QXT_DOXYGEN_RUN)
    virtual bool x11EventFilter(XEvent*)
    {
        return false;
    }
#endif // Q_WS_X11
#if defined(Q_WS_WIN) || defined(QXT_DOXYGEN_RUN)
    virtual bool winEventFilter(MSG*, long*)
    {
        return false;
    }
#endif // Q_WS_WIN
#if defined(Q_WS_MAC) || defined(QXT_DOXYGEN_RUN)
    virtual bool macEventFilter(EventHandlerCallRef, EventRef)
    {
        return false;
    }
#endif // Q_WS_MAC
};

#endif // QXTNATIVEEVENTFILTER_H
