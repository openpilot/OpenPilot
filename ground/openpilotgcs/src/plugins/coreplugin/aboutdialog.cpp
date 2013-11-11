/**
 ******************************************************************************
 *
 * @file       aboutdialog.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "version_info/version_info.h"
#include "coreconstants.h"
#include "icore.h"

#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QSysInfo>

#include <QtQuick/QQuickView>
#include <QQmlContext>

using namespace Core::Constants;

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/core/images/openpilot_logo_32.png"));
    setWindowTitle(tr("About OpenPilot"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // This loads a QML doc
    QQuickView *view = new QQuickView();
    QWidget *container = QWidget::createWindowContainer(view, this);
    view->setSource(QUrl("qrc:/core/qml/AboutDialog.qml"));

    ui->verticalLayout->addWidget(container);

    QString version = QLatin1String(GCS_VERSION_LONG);
    version += QDate(2007, 25, 10).toString(Qt::SystemLocaleDate);

    QString ideRev;

    // : This gets conditionally inserted as argument %8 into the description string.
    ideRev = tr("From revision %1<br/>").arg(VersionInfo::revision().left(10));

    const QString description = tr(
        "<h3>OpenPilot Ground Control Station</h3>"
        "GCS Revision: <b>%1</b><br/>"
        "UAVO Hash: <b>%2</b><br/>"
        "<br/>"
        "Built from %3<br/>"
        "Built on %4 at %5<br/>"
        "Based on Qt %6 (%7 bit)<br/>"
        "<br/>"
        "&copy; %8, 2010-%9. All rights reserved.<br/>"
        "<br/>"
        "<small>This program is free software; you can redistribute it and/or modify<br/>"
        "it under the terms of the GNU General Public License as published by<br/>"
        "the Free Software Foundation; either version 3 of the License, or<br/>"
        "(at your option) any later version.<br/>"
        "<br/>"
        "The program is provided AS IS with NO WARRANTY OF ANY KIND, "
        "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A "
        "PARTICULAR PURPOSE.</small>"
        ).arg(
        VersionInfo::revision().left(60), // %1
        VersionInfo::uavoHash().left(8), // %2
        VersionInfo::origin(), // $3
        QLatin1String(__DATE__), // %4
        QLatin1String(__TIME__), // %5
        QLatin1String(QT_VERSION_STR), // %6
        QString::number(QSysInfo::WordSize), // %7
        QLatin1String(GCS_AUTHOR), // %8
        VersionInfo::year() // %9
        );
    // Expose the version description to the QML doc
    view->rootContext()->setContextProperty("version", description);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
