/**
 ******************************************************************************
 *
 * @file       antennatrackgadgetoptionspage.h
 * @author     Sami Korhonen & the OpenPilot team Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup AntennaTrackGadgetPlugin Antenna Track Gadget Plugin
 * @{
 * @brief A gadget that communicates with antenna tracker and enables basic configuration
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

#ifndef ANTENNATRACKGADGETOPTIONSPAGE_H
#define ANTENNATRACKGADGETOPTIONSPAGE_H

#include <qextserialport/src/qextserialenumerator.h>
#include "coreplugin/dialogs/ioptionspage.h"
#include "QString"
#include <QStringList>
#include <QDebug>

namespace Core {
class IUAVGadgetConfiguration;
}

class AntennaTrackGadgetConfiguration;

namespace Ui {
    class AntennaTrackGadgetOptionsPage;
}

using namespace Core;

class AntennaTrackGadgetOptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit AntennaTrackGadgetOptionsPage(AntennaTrackGadgetConfiguration *config, QObject *parent = 0);

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

private:
    Ui::AntennaTrackGadgetOptionsPage *options_page;
    AntennaTrackGadgetConfiguration *m_config;

    QStringList BaudRateTypeString;
    QStringList BaudRateTypeStringALL;
    QStringList DataBitsTypeStringALL;
    QStringList ParityTypeStringALL;
    QStringList StopBitsTypeStringALL;
    QStringList DataBitsTypeString;
    QStringList ParityTypeString;
    QStringList StopBitsTypeString;
    QStringList FlowTypeString;

private slots:
};

#endif // ANTENNATRACKGADGETOPTIONSPAGE_H
