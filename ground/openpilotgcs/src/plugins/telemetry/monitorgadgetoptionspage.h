/**
 ******************************************************************************
 *
 * @file       monitorgadgetoptionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Telemetry Gadget options page header
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   telemetry
 * @{
 *
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

#ifndef MONITORGADGETOPTIONSPAGE_H
#define MONITORGADGETOPTIONSPAGE_H

#include "coreplugin/dialogs/ioptionspage.h"
#include "uavobjectmanager.h"
#include "uavobject.h"

#include "QString"
#include <QDebug>
#include <QtCore/QSettings>

namespace Ui {
class MonitorGadgetOptionsPage;
};

class MonitorGadgetConfiguration;

using namespace Core;

class MonitorGadgetOptionsPage : public IOptionsPage {
    Q_OBJECT

public:
    MonitorGadgetOptionsPage(MonitorGadgetConfiguration *config, QObject *parent = 0);
    ~MonitorGadgetOptionsPage();

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

private:
};

#endif // MONITORGADGETOPTIONSPAGE_H
