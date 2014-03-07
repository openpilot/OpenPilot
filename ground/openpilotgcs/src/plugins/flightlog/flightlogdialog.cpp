/**
 ******************************************************************************
 *
 * @file       flightlogdialog.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup [Group]
 * @{
 * @addtogroup FlightLogDialog
 * @{
 * @brief [Brief]
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

#include "flightlogdialog.h"

#include <QVBoxLayout>

#include <QtQuick>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>

#include "flightlogmanager.h"
#include "uavobject.h"

FlightLogDialog::FlightLogDialog(QWidget *parent, FlightLogManager *flightLogManager) :
    QDialog(parent)
{
    qmlRegisterType<ExtendedDebugLogEntry>("org.openpilot", 1, 0, "DebugLogEntry");    
    qmlRegisterType<UAVOLogSettingsWrapper>("org.openpilot", 1, 0, "UAVOLogSettingsWrapper");

    setWindowIcon(QIcon(":/core/images/openpilot_logo_32.png"));
    setWindowTitle(tr("Manage flight side logs"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumSize(600, 400);

    QQuickView *view = new QQuickView();
    view->rootContext()->setContextProperty("dialog", this);
    view->rootContext()->setContextProperty("logStatus", flightLogManager->flightLogStatus());
    view->rootContext()->setContextProperty("logManager", flightLogManager);
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setSource(QUrl("qrc:/flightlog/FlightLogDialog.qml"));

    QWidget *container = QWidget::createWindowContainer(view);
    container->setMinimumSize(600, 400);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *lay   = new QVBoxLayout();
    lay->setContentsMargins(0, 0, 0, 0);
    setLayout(lay);
    layout()->addWidget(container);
}

FlightLogDialog::~FlightLogDialog()
{}
