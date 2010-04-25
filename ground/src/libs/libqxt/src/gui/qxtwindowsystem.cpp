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
#include <QStringList>

/*!
    \class QxtWindowSystem
    \inmodule QxtGui
    \brief The QxtWindowSystem class provides means for accessing native windows.

    \bold {Note:} The underlying window system might or might not allow one to alter
    states of windows belonging to other processes.

    \warning QxtWindowSystem is portable in principle, but be careful while
    using it since you are probably about to do something non-portable.

    \section1 Advanced example usage:
    \code
    class NativeWindow : public QWidget {
        public:
            NativeWindow(WId wid) {
                QWidget::create(wid, false, false); // window, initializeWindow, destroyOldWindow
            }
            ~NativeWindow() {
                QWidget::destroy(false, false); // destroyWindow, destroySubWindows
            }
    };
    \endcode

    \code
    WindowList windows = QxtWindowSystem::windows();
    QStringList titles = QxtWindowSystem::windowTitles();
    bool ok = false;
    QString title = QInputDialog::getItem(0, "Choose Window", "Choose a window to be hid:", titles, 0, false, &ok);
    if (ok)
    {
        int index = titles.indexOf(title);
        if (index != -1)
        {
            NativeWindow window(windows.at(index));
            window.hide();
        }
    }
    \endcode

    \bold {Note:} Currently supported platforms are \bold X11 and \bold Windows.
 */

/*!
    \fn QxtWindowSystem::windows()

    Returns the list of native window system identifiers.

    \sa QApplication::topLevelWidgets(), windowTitles()
 */

/*!
    \fn QxtWindowSystem::activeWindow()

    Returns the native window system identifier of the active window if any.

    \sa QApplication::activeWindow()
 */

/*!
    \fn QxtWindowSystem::findWindow(const QString& title)

    Returns the native window system identifier of the window if any with given \a title.

    Example usage:
    \code
    WId wid = QxtWindowSystem::findWindow("Mail - Kontact");
    QPixmap screenshot = QPixmap::grabWindow(wid);
    \endcode

    \sa QWidget::find()
 */

/*!
    \fn QxtWindowSystem::windowAt(const QPoint& pos)

    Returns the native window system identifier of the window if any at \a pos.

    \sa QApplication::widgetAt()
 */

/*!
    \fn QxtWindowSystem::windowTitle(WId window)

    Returns the title of the native \a window.

    \sa QWidget::windowTitle(), windowTitles()
 */

/*!
    \fn QxtWindowSystem::windowTitles()

    Returns a list of native window titles.

    \sa QWidget::windowTitle(), windowTitle(), windows()
 */

/*!
    \fn QxtWindowSystem::windowGeometry(WId window)

    Returns the geometry of the native \a window.

    \sa QWidget::frameGeometry()
 */

/*!
    \fn QxtWindowSystem::idleTime()

    Returns the system "idle time" ie. the time since last user input
    in milliseconds.
 */

QStringList QxtWindowSystem::windowTitles()
{
    WindowList windows = QxtWindowSystem::windows();
    QStringList titles;
    foreach(WId window, windows)
    titles += QxtWindowSystem::windowTitle(window);
    return titles;
}
