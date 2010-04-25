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
#include "qxtlabel.h"
#include <QTime>
#include <QEvent>
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>

static const int Vertical_Mask = 0x02;

class QxtLabelPrivate : public QxtPrivate<QxtLabel>
{
public:
    QXT_DECLARE_PUBLIC(QxtLabel)

    void init(const QString& txt = QString());
    void updateLabel();

    QTime time;
    QString text;
    Qt::Alignment align;
    Qt::TextElideMode mode;
    Qxt::Rotation rot;
};

void QxtLabelPrivate::init(const QString& txt)
{
    text = txt;
    align = Qt::AlignCenter;
    mode = Qt::ElideMiddle;
    rot = Qxt::NoRotation;
}

void QxtLabelPrivate::updateLabel()
{
    qxt_p().updateGeometry();
    qxt_p().update();
}

/*!
    \class QxtLabel
    \inmodule QxtGui
    \brief The QxtLabel widget is a label which is able to show elided and rotated plain text.

    QxtLabel is a label which is able to show elided and rotated plain text.
    In addition, QxtLabel provides a signal for clicking.

    \image qxtlabel.png "QxtLabel in action."

    Usage:
    \code
    QxtLabel* label = new QxtLabel(tr("Hello, World!"), this);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setElideMode(Qt::ElideRight);
    label->setRotation(Qxt::CounterClockwise);
    connect(label, SIGNAL(clicked()), this, SLOT(labelClicked()));
    \endcode

    \sa QLabel
 */

/*!
    \fn QxtLabel::clicked()

    This signal is emitted whenever the label has been clicked.

    \bold {Note:} A combination of mouse button press and release in shorter
    time than QApplication::doubleClickInterval is considered
    as a click.

    \sa QApplication::doubleClickInterval
 */

/*!
    \fn QxtLabel::textChanged(const QString& text)

    This signal is emitted whenever the \a text has changed.
 */

/*!
    Constructs a new QxtLabel with \a parent and \a flags.
 */
QxtLabel::QxtLabel(QWidget* parent, Qt::WindowFlags flags) : QFrame(parent, flags)
{
    QXT_INIT_PRIVATE(QxtLabel);
    qxt_d().init();
}

/*!
    Constructs a new QxtLabel with \a text, \a parent and \a flags.
 */
QxtLabel::QxtLabel(const QString& text, QWidget* parent, Qt::WindowFlags flags) : QFrame(parent, flags)
{
    QXT_INIT_PRIVATE(QxtLabel);
    qxt_d().init(text);
}

/*!
    Destructs the label.
 */
QxtLabel::~QxtLabel()
{}

/*!
    \property QxtLabel::text
    \brief the text of the label
 */
QString QxtLabel::text() const
{
    return qxt_d().text;
}

void QxtLabel::setText(const QString& text)
{
    if (qxt_d().text != text)
    {
        qxt_d().text = text;
        qxt_d().updateLabel();
        emit textChanged(text);
    }
}

/*!
    \property QxtLabel::alignment
    \brief the alignment of the text

    The text is aligned according to this property.
    The default value is Qt::AlignCenter.

    \sa text, Qt::Alignment
 */
Qt::Alignment QxtLabel::alignment() const
{
    return qxt_d().align;
}

void QxtLabel::setAlignment(Qt::Alignment alignment)
{
    if (qxt_d().align != alignment)
    {
        qxt_d().align = alignment;
        update(); // no geometry change, repaint is sufficient
    }
}

/*!
    \property QxtLabel::elideMode
    \brief the elide mode of the text

    The text is elided according to this property.
    The default value is Qt::ElideMiddle.

    \sa text, Qt::TextElideMode
 */
Qt::TextElideMode QxtLabel::elideMode() const
{
    return qxt_d().mode;
}

void QxtLabel::setElideMode(Qt::TextElideMode mode)
{
    if (qxt_d().mode != mode)
    {
        qxt_d().mode = mode;
        qxt_d().updateLabel();
    }
}

/*!
    \property QxtLabel::rotation
    \brief the rotation of the label

    The label is rotated according to this property.
    The default value is Qxt::NoRotation.

    \sa Qxt::Rotation
 */
Qxt::Rotation QxtLabel::rotation() const
{
    return qxt_d().rot;
}

void QxtLabel::setRotation(Qxt::Rotation rotation)
{
    if (qxt_d().rot != rotation)
    {
        Qxt::Rotation prev = qxt_d().rot;
        qxt_d().rot = rotation;
        switch (rotation)
        {
        case Qxt::NoRotation:
        case Qxt::UpsideDown:
            if (prev & Vertical_Mask)
            {
                updateGeometry();
            }
            break;

        case Qxt::Clockwise:
        case Qxt::CounterClockwise:
            if ((prev & Vertical_Mask) == 0)
            {
                updateGeometry();
            }
            break;
        default:
            // nothing to do
            break;
        }
    }
    update();
}

/*!
    \reimp
 */
QSize QxtLabel::sizeHint() const
{
    const QFontMetrics& fm = fontMetrics();
    QSize size(fm.width(qxt_d().text), fm.height());
    if (qxt_d().rot & Vertical_Mask)
        size.transpose();
    return size;
}

/*!
    \reimp
 */
QSize QxtLabel::minimumSizeHint() const
{
    switch (qxt_d().mode)
    {
    case Qt::ElideNone:
        return sizeHint();
    default:
    {
        const QFontMetrics& fm = fontMetrics();
        QSize size(fm.width("..."), fm.height());
        if (qxt_d().rot & Vertical_Mask)
            size.transpose();
        return size;
    }
    }
}

/*!
    \reimp
 */
void QxtLabel::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter p(this);
    p.rotate(qxt_d().rot);
    QRect r = contentsRect();
    switch (qxt_d().rot)
    {
    case Qxt::UpsideDown:
        p.translate(-r.width(), -r.height());
        break;

    case Qxt::Clockwise:
        p.translate(0, -r.width());
        break;

    case Qxt::CounterClockwise:
        p.translate(-r.height(), 0);
        break;

    default:
        // nothing to do
        break;
    }

    if (qxt_d().rot & Vertical_Mask)
    {
        QSize s = r.size();
        s.transpose();
        r = QRect(r.topLeft(), s);
    }

    const QString elidedText = fontMetrics().elidedText(qxt_d().text, qxt_d().mode, r.width());
    p.drawText(r, qxt_d().align, elidedText);
}

/*!
    \reimp
 */
void QxtLabel::changeEvent(QEvent* event)
{
    QFrame::changeEvent(event);
    switch (event->type())
    {
    case QEvent::FontChange:
    case QEvent::ApplicationFontChange:
        qxt_d().updateLabel();
        break;
    default:
        // nothing to do
        break;
    }
}

/*!
    \reimp
 */
void QxtLabel::mousePressEvent(QMouseEvent* event)
{
    QFrame::mousePressEvent(event);
    qxt_d().time.start();
}

/*!
    \reimp
 */
void QxtLabel::mouseReleaseEvent(QMouseEvent* event)
{
    QFrame::mouseReleaseEvent(event);
    if (qxt_d().time.elapsed() < qApp->doubleClickInterval())
        emit clicked();
}
