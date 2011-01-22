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
#ifndef QXTSCREEN_P_H
#define QXTSCREEN_P_H

#include <QSize>
#include <QList>
#include <QMultiHash>
#include "qxtscreen.h"

class QxtScreenPrivate : public QxtPrivate<QxtScreen>
{
public:
    QxtScreenPrivate();
    void invalidate();
    void init();
    void init_sys();
    bool set(const QSize& reso, int rate, int depth);

    QSize currReso;
    QSize setReso;
    QList<QSize> availResos;
    int currRate;
    int setRate;
    QMultiHash<QSize, int> availRates;
    int currDepth;
    int setDepth;
    QMultiHash<QSize, int> availDepths;
    int screen;
};

inline uint qHash(const QSize& key)
{
    uint h1 = qHash(key.width());
    uint h2 = qHash(key.height());
    return ((h1 << 16) | (h1 >> 16)) ^ h2;
}

#endif // QXTSCREEN_P_H
