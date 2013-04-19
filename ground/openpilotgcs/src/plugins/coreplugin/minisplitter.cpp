/**
 ******************************************************************************
 *
 * @file       minisplitter.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup CorePlugin Core Plugin
 * @{
 * @brief The Core GCS plugin
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "minisplitter.h"

#include <utils/stylehelper.h>

#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QSplitterHandle>

namespace Core {
namespace Internal {

class MiniSplitterHandle : public QSplitterHandle
{
public:
    MiniSplitterHandle(Qt::Orientation orientation, QSplitter *parent)
            : QSplitterHandle(orientation, parent)
    {
        setMask(QRegion(contentsRect()));
        setAttribute(Qt::WA_MouseNoMask, true);
    }
protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
};

} // namespace Internal
} // namespace Core

using namespace Core;
using namespace Core::Internal;

void MiniSplitterHandle::resizeEvent(QResizeEvent *event)
{
    // Warning: We specifically replace the QSplitterHandle::resizeEvent,
    // since there's no way of doing this while still calling it.
    // That's because it has pretty much identical code (in 4.7.0) which
    // undoes what we do here. And they didn't make that code configurable :)
    // This means that with Qt upgrades it's worthwhile to see if anything changed
    // in QSplitterHandle::resizeEvent, to see if there's anything important we miss.

    if (orientation() == Qt::Horizontal)
        setContentsMargins(6, 0, 6, 0);
    else
        setContentsMargins(0, 6, 0, 6);
    setMask(QRegion(contentsRect()));

    QWidget::resizeEvent(event);
}

void MiniSplitterHandle::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(event->rect(), Utils::StyleHelper::borderColor());
}

QSplitterHandle *MiniSplitter::createHandle()
{
    return new MiniSplitterHandle(orientation(), this);
}

MiniSplitter::MiniSplitter(QWidget *parent)
    : QSplitter(parent)
{
    setHandleWidth(1);
    setChildrenCollapsible(false);
    setProperty("minisplitter", true);
}

MiniSplitter::MiniSplitter(Qt::Orientation orientation)
    : QSplitter(orientation)
{
    setHandleWidth(1);
    setChildrenCollapsible(false);
    setProperty("minisplitter", true);
}
