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
#include "qxtlineedit.h"
#include <QStyleOptionFrameV2>
#include <QPainter>
#include <QStyle>

// copied from qlineedit.cpp:
#define vMargin 1
#define hMargin 2

class QxtLineEditPrivate : public QxtPrivate<QxtLineEdit>
{
public:
    QString sampleText;
};

/*!
    \class QxtLineEdit
    \inmodule QxtGui
    \brief The QxtLineEdit widget is a line edit that is able to show a sample text.

    QxtLineEdit is a line edit that is able to show a sample text.
    The sample text is shown when the line edit is empty and has
    no focus.

    \image qxtlineedit.png "Two empty QxtLineEdits: non-focused and focused."
 */

/*!
    Constructs a new QxtLineEdit with \a parent.
 */
QxtLineEdit::QxtLineEdit(QWidget* parent) : QLineEdit(parent)
{
}

/*!
    Constructs a new QxtLineEdit with \a text and \a parent.
 */
QxtLineEdit::QxtLineEdit(const QString& text, QWidget* parent) : QLineEdit(text, parent)
{
}

/*!
    Destructs the line edit.
 */
QxtLineEdit::~QxtLineEdit()
{
}

/*!
    \property QxtLineEdit::sampleText
    \brief the sample text of the line edit

    The sample text is shown when the line edit is empty and has
    no focus.
 */
QString QxtLineEdit::sampleText() const
{
    return qxt_d().sampleText;
}

void QxtLineEdit::setSampleText(const QString& text)
{
    if (qxt_d().sampleText != text)
    {
        qxt_d().sampleText = text;
        if (displayText().isEmpty() && !hasFocus())
            update();
    }
}

/*!
    \reimp
*/
void QxtLineEdit::paintEvent(QPaintEvent* event)
{
    QLineEdit::paintEvent(event);
    if (displayText().isEmpty() && !hasFocus())
    {
        QStyleOptionFrameV2 option;
        initStyleOption(&option);


        QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &option, this);
#if QT_VERSION >= 0x040500
        // TODO: sort out prior Qt 4.5
        int left, top, right, bottom;
        getTextMargins(&left, &top, &right, &bottom);
        r.adjust(left, top, -right, -bottom);
#endif // QT_VERSION >= 0x040500
        r.adjust(hMargin, vMargin, -hMargin, -vMargin);

        QPainter painter(this);
        QPalette pal = palette();
        pal.setCurrentColorGroup(QPalette::Disabled);
        style()->drawItemText(&painter, r, alignment(), pal, false, qxt_d().sampleText, QPalette::Text);
    }
}
