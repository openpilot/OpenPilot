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
#include "qxtconfigdialog_p.h"
#include "qxtconfigdialog.h"
#include "qxtconfigwidget.h"
#include <QDialogButtonBox>
#include <QApplication>
#include <QVBoxLayout>

void QxtConfigDialogPrivate::init( QxtConfigWidget::IconPosition pos )
{
    QxtConfigDialog* p = &qxt_p();
    configWidget = new QxtConfigWidget(pos);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, p);
    QObject::connect(buttons, SIGNAL(accepted()), p, SLOT(accept()));
    QObject::connect(buttons, SIGNAL(rejected()), p, SLOT(reject()));
    layout = new QVBoxLayout(p);
    layout->addWidget(configWidget);
    layout->addWidget(buttons);
}

/*!
    \class QxtConfigDialog
    \inmodule QxtGui
    \brief The QxtConfigDialog class provides a configuration dialog.

    QxtConfigDialog provides a convenient interface for building
    common configuration dialogs. QxtConfigDialog consists of a
    list of icons and a stack of pages.

    Example usage:
    \code
    QxtConfigDialog dialog;
    dialog.addPage(new ConfigurationPage(&dialog), QIcon(":/images/config.png"), tr("Configuration"));
    dialog.addPage(new UpdatePage(&dialog), QIcon(":/images/update.png"), tr("Update"));
    dialog.addPage(new QueryPage(&dialog), QIcon(":/images/query.png"), tr("Query"));
    dialog.exec();
    \endcode

    \image qxtconfigdialog.png "QxtConfigDialog with page icons on the left (QxtConfigDialog::West)."

    \sa QxtConfigWidget
 */

/*!
    \fn QxtConfigDialog::currentIndexChanged(int index)

    This signal is emitted whenever the current page \a index changes.

    \sa currentIndex()
 */

/*!
    Constructs a new QxtConfigDialog with \a parent and \a flags.
 */
QxtConfigDialog::QxtConfigDialog(QWidget* parent, Qt::WindowFlags flags)
        : QDialog(parent, flags)
{
    QXT_INIT_PRIVATE(QxtConfigDialog);
    qxt_d().init(QxtConfigWidget::West);
}

/*!
    Constructs a new QxtConfigDialog with icon \a position, \a parent and \a flags.
 */
QxtConfigDialog::QxtConfigDialog(QxtConfigWidget::IconPosition position, QWidget* parent, Qt::WindowFlags flags)
        : QDialog(parent, flags)
{
    QXT_INIT_PRIVATE(QxtConfigDialog);
    qxt_d().init(position);
}

/*!
    Destructs the config dialog.
 */
QxtConfigDialog::~QxtConfigDialog()
{
}

/*!
    Returns the dialog button box.

    The default buttons are QDialogButtonBox::Ok and QDialogButtonBox::Cancel.

    \sa setDialogButtonBox()
*/
QDialogButtonBox* QxtConfigDialog::dialogButtonBox() const
{
    return qxt_d().buttons;
}

/*!
    Sets the dialog \a buttonBox. The previous button box
    is deleted if the parent equals \c this.

    \sa dialogButtonBox()
*/
void QxtConfigDialog::setDialogButtonBox(QDialogButtonBox* buttonBox)
{
    if (qxt_d().buttons != buttonBox)
    {
        if (qxt_d().buttons && qxt_d().buttons->parent() == this)
            delete qxt_d().buttons;
        qxt_d().buttons = buttonBox;
        if (qxt_d().buttons)
            qxt_d().layout->addWidget(qxt_d().buttons);
    }
}

/*!
    Returns the config widget.

    \sa setConfigWidget()
*/
QxtConfigWidget* QxtConfigDialog::configWidget() const
{
    return qxt_d().configWidget;
}

/*!
    Sets the \a configWidget. The previous config widget
    is deleted if the parent equals \c this.

    \sa configWidget()
*/
void QxtConfigDialog::setConfigWidget(QxtConfigWidget* configWidget)
{
    if (qxt_d().configWidget != configWidget)
    {
        if (qxt_d().configWidget && qxt_d().configWidget->parent() == this)
            qxt_d().configWidget->deleteLater();
        qxt_d().configWidget = configWidget;
        if (qxt_d().configWidget)
            qxt_d().layout->insertWidget(0, qxt_d().configWidget);
    }
}

/*!
    \bold {Note:} The default implementation calls SLOT(accept()) of
    each page page provided that such slot exists.

    \sa reject()
 */
void QxtConfigDialog::accept()
{
    qxt_d().configWidget->accept();
    QDialog::accept();
}

/*!
    \bold {Note:} The default implementation calls SLOT(reject()) of
    each page provided that such slot exists.

    \sa accept()
 */
void QxtConfigDialog::reject()
{
    qxt_d().configWidget->reject();
    QDialog::reject();
}
