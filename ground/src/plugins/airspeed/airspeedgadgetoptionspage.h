/**
 ******************************************************************************
 *
 * @file       airspeedgadgetoptionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Airspeed Plugin Gadget options page
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   Airspeed
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

#ifndef AIRSPEEDGADGETOPTIONSPAGE_H
#define AIRSPEEDGADGETOPTIONSPAGE_H

#include "coreplugin/dialogs/ioptionspage.h"
#include "QString"
#include <QStringList>
#include <QDebug>

namespace Core {
class IUAVGadgetConfiguration;
}

class AirspeedGadgetConfiguration;

namespace Ui {
    class AirspeedGadgetOptionsPage;
}

using namespace Core;

class AirspeedGadgetOptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit AirspeedGadgetOptionsPage(AirspeedGadgetConfiguration *config, QObject *parent = 0);

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

private:
    Ui::AirspeedGadgetOptionsPage *options_page;
    AirspeedGadgetConfiguration *m_config;

private slots:
    void on_loadFile_clicked();
};

#endif // AIRSPEEDGADGETOPTIONSPAGE_H
