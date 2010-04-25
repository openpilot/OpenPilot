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

#include "qxtcountrycombobox.h"
#include "qxtcountrycombobox_p.h"
#include "qxtcountrymodel.h"
#include <QDebug>


QxtCountryComboBoxPrivate::QxtCountryComboBoxPrivate()
{
}

void QxtCountryComboBoxPrivate::comboBoxCurrentIndexChanged(int)
{
    emit qxt_p().currentCountryChanged(currentCountry());
    emit qxt_p().currentCountryNameChanged(currentCountryName());
}

QLocale::Country QxtCountryComboBoxPrivate::currentCountry() const
{
    QModelIndex idx = qxt_p().model()->index(qxt_p().currentIndex(), 3);
    return static_cast<QLocale::Country>(idx.data().toInt());
}

QString QxtCountryComboBoxPrivate::currentCountryName() const
{
    return qxt_p().currentText();
}

void QxtCountryComboBoxPrivate::setCurrentCountry(QLocale::Country country)
{
    // column 3 is QLocale::Country
    QModelIndex start = qxt_p().model()->index(0, 3);
    QModelIndexList result = qxt_p().model()->match(start, Qt::DisplayRole, country, 1, Qt::MatchExactly);
    if (!result.isEmpty())
        qxt_p().setCurrentIndex(result.first().row());
}

/*!
    \class QxtCountryComboBox
    \inmodule QxtGui
    \brief The QxtCountryComboBox widget is an extended QComboBox to display countries.

    QxtCountryComboBox is a specialized combo box to display countries.
    The languages are taken from QLocale::Country.

    \image qxtcountrycombobox.png "QxtCountryComboBox in Plastique style."
 */

/*!
    \fn QxtCountryComboBox::currentCountryChanged(QLocale::Country country)

    This signal is emitted whenever the current \a country has been changed.
 */

/*!
    \fn QxtCountryComboBox::currentCountryNameChanged(const QString& name)

    This signal is emitted whenever the current country \a name has been changed.
 */

/*!
    Constructs a new QxtCountryComboBox with \a parent.
 */
QxtCountryComboBox::QxtCountryComboBox(QWidget* parent)
        : QComboBox(parent)
{
    QXT_INIT_PRIVATE(QxtCountryComboBox);
#ifdef QXT_DESIGNER_MODE
    setEditable(false);
#else
    setModel(new QxtCountryModel(this));
    setModelColumn(0);

    setCurrentCountry(QLocale::system().country());
#endif

    connect(this, SIGNAL(currentIndexChanged(int)), &qxt_d(), SLOT(comboBoxCurrentIndexChanged(int)));
}

#ifdef QXT_DESIGNER_MODE
void QxtCountryComboBox::paintEvent(QPaintEvent *e)
{
    QComboBox::paintEvent(e);
    QStylePainter painter(this);
    painter.drawItemText(e->rect(), Qt::AlignCenter, palette(), true, tr("DESIGNER MODE  -  DESIGNER MODE"));
}
#endif
/*!
    Destructs the country combo box.
 */
QxtCountryComboBox::~QxtCountryComboBox()
{
}

/*!
    \property QxtCountryComboBox::currentCountry
    \brief the current country.
 */
QLocale::Country QxtCountryComboBox::currentCountry() const
{
    return qxt_d().currentCountry();
}

/*!
    \property QxtCountryComboBox::currentCountryName
    \brief the name of the current country.
 */
QString QxtCountryComboBox::currentCountryName() const
{
    return qxt_d().currentCountryName();
}

void QxtCountryComboBox::setCurrentCountry(QLocale::Country country)
{
    return qxt_d().setCurrentCountry(country);
}

