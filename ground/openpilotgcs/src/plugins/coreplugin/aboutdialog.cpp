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
#include <QDesktopServices>

#include <QtQuick>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>

using namespace Core::Constants;

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowIcon(QIcon(":/core/images/openpilot_logo_32.png"));
    setWindowTitle(tr("About OpenPilot"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumSize(600, 400);
    setMaximumSize(800, 600);

    const QString description = tr(
        "Revision: <b>%1</b><br/>"
        "UAVO Hash: <b>%2</b><br/>"
        "<br/>"
        "Built from %3<br/>"
        "Built on %4 at %5<br/>"
        "Based on Qt %6 (%7 bit)<br/>"
        "<br/>"
        "&copy; %8, 2010-%9. All rights reserved.<br/>"       
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

    QQuickView *view = new QQuickView();
    view->rootContext()->setContextProperty("dialog", this);
    view->rootContext()->setContextProperty("version", description);
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setSource(QUrl("qrc:/core/qml/AboutDialog.qml"));

    QWidget * container = QWidget::createWindowContainer(view);
    container->setMinimumSize(600, 400);
    container->setMaximumSize(800, 600);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *lay = new QVBoxLayout();
    lay->setContentsMargins(0,0,0,0);
    setLayout(lay);
    layout()->addWidget(container);
}

void AboutDialog::openUrl(const QString &url)
{
    QDesktopServices::openUrl(QUrl(url));
}

AboutDialog::~AboutDialog()
{
}
