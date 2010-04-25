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
#include "qxtscreen.h"
#include "qxtscreen_p.h"
#include <QDesktopWidget>
#include <QApplication>

QxtScreenPrivate::QxtScreenPrivate() :
    currReso(),
    setReso(),
    availResos(),
    currRate(-1),
    setRate(-1),
    availRates(),
    currDepth(-1),
    setDepth(-1),
    availDepths(),
    screen(-1)
{
}

void QxtScreenPrivate::invalidate()
{
    currReso = QSize();
    availResos.clear();

    currRate = -1;
    availRates.clear();
    
    currDepth = -1;
    availDepths.clear();

    screen = -1;
}

void QxtScreenPrivate::init()
{
    if (screen < 0)
        screen = qApp->desktop()->primaryScreen();

    if (availResos.isEmpty() || !currReso.isValid() || 
        availRates.isEmpty() || currRate < 0 || 
        availDepths.isEmpty() || currDepth < 0)
        init_sys();
}

/*!
    \class QxtScreen
    \inmodule QxtGui
    \brief The QxtScreen class provides access to screen settings.

    QxtScreen provides access to screen settings.

    \bold {Note:} Currently supported platforms are X11 and Windows.
 */

/*!
    Constructs a new QxtScreen for \a screen.

    \bold {Note:} The primary screen is used if \a screen is \c -1.
 */
QxtScreen::QxtScreen(int screen)
{
    qxt_d().screen = screen;
}

/*!
    Constructs a new QxtScreen for \a screen.
 */
QxtScreen::QxtScreen(QWidget* screen)
{
    qxt_d().screen = qApp->desktop()->screenNumber(screen);
}

/*!
    Destructs the screen object.
 */
QxtScreen::~QxtScreen()
{
}

/*!
    Returns the screen number.
 */
int QxtScreen::screenNumber() const
{
    const_cast<QxtScreen*>(this)->qxt_d().init();
    return qxt_d().screen;
}

/*!
    Sets the \a screen number.
 */
void QxtScreen::setScreenNumber(int screen)
{
    qxt_d().invalidate();
    qxt_d().screen = screen;
}

/*!
    Returns the screen widget.

    \sa QDesktopWidget::screen()
 */
QWidget* QxtScreen::screen() const
{
    const_cast<QxtScreen*>(this)->qxt_d().init();
    return qApp->desktop()->screen(qxt_d().screen);
}

/*!
    Sets the \a screen.

    \sa QDesktopWidget::screenNumber()
 */
void QxtScreen::setScreen(QWidget* screen)
{
    qxt_d().invalidate();
    qxt_d().screen = qApp->desktop()->screenNumber(screen);
}

/*!
    Returns the list of available resolutions.

    \sa availableRefreshRates()
 */
QList<QSize> QxtScreen::availableResolutions() const
{
    const_cast<QxtScreen*>(this)->qxt_d().init();
    return qxt_d().availResos;
}

/*!
    Returns the list of available refresh rates for \a resolution.

    \sa availableResolutions()
 */
QList<int> QxtScreen::availableRefreshRates(const QSize& resolution) const
{
    const_cast<QxtScreen*>(this)->qxt_d().init();
    return qxt_d().availRates.values(resolution);
}

/*!
    Returns the list of available color depths for \a resolution.

    \sa availableResolutions()
 */
QList<int> QxtScreen::availableColorDepths(const QSize& resolution) const
{
    const_cast<QxtScreen*>(this)->qxt_d().init();
    return qxt_d().availDepths.values(resolution);
}

/*!
    Returns the current resolution.
 */
QSize QxtScreen::resolution() const
{
    const_cast<QxtScreen*>(this)->qxt_d().init();
    return qxt_d().currReso;
}

/*!
    Sets the \a resolution.

    \bold {Note:} No changes are applied until apply() is called.

    \sa apply()
 */
void QxtScreen::setResolution(const QSize& resolution)
{
    qxt_d().setReso = resolution;
}

/*!
    Returns the current refresh rate.
 */
int QxtScreen::refreshRate() const
{
    const_cast<QxtScreen*>(this)->qxt_d().init();
    return qxt_d().currRate;
}

/*!
    Sets the refresh \a rate.

    \bold {Note:} No changes are applied until apply() is called.

    \sa apply()
 */
void QxtScreen::setRefreshRate(int rate)
{
    qxt_d().setRate = rate;
}

/*!
    Returns the current color depth.

    \bold {Note:} This is currently supported only on Windows.
 */
int QxtScreen::colorDepth() const
{
    const_cast<QxtScreen*>(this)->qxt_d().init();
    return qxt_d().currDepth;
}

/*!
    Sets the color \a depth.

    \bold {Note:} No changes are applied until apply() is called.
    \bold {Note:} This is currently supported only on Windows.

    \sa apply()
 */
void QxtScreen::setColorDepth(int depth)
{
    qxt_d().setDepth = depth;
}

/*!
    Applies the resolution and refresh rate.

    Returns \c true if succeeds, \c false otherwise.

    \sa setResolution(), setRefreshRate()
 */
bool QxtScreen::apply()
{
    qxt_d().init();
    return qxt_d().set(qxt_d().setReso, qxt_d().setRate, qxt_d().setDepth);
}

/*!
    Reverts the resolution and refresh rate.

    Returns \c true if succeeds, \c false otherwise.

    \sa setResolution(), setRefreshRate()
 */
bool QxtScreen::cancel()
{
    qxt_d().init();
    return qxt_d().set(qxt_d().currReso, qxt_d().currRate, qxt_d().currDepth);
}

/*!
    Refreshes the screen information.
 */
void QxtScreen::refresh()
{
    qxt_d().invalidate();
    qxt_d().init();
}
