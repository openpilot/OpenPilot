/**
 ******************************************************************************
 *
 * @file       gcscontrolgadgetoptionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A place holder gadget plugin
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

#ifndef GCSCONTROLGADGETOPTIONSPAGE_H
#define GCSCONTROLGADGETOPTIONSPAGE_H

#include "coreplugin/dialogs/ioptionspage.h"
#include "QString"
#include <QStringList>
#include <QDebug>

namespace Core {
class IUAVGadgetConfiguration;
}

class GCSControlGadgetConfiguration;

namespace Ui {
    class GCSControlGadgetOptionsPage;
}

using namespace Core;

class GCSControlGadgetOptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit GCSControlGadgetOptionsPage(GCSControlGadgetConfiguration *config, QObject *parent = 0);

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

private:
    Ui::GCSControlGadgetOptionsPage *options_page;
    GCSControlGadgetConfiguration *m_config;

    /*
    QStringList BaudRateTypeString;
    QStringList BaudRateTypeStringALL;
    QStringList DataBitsTypeStringALL;
    QStringList ParityTypeStringALL;
    QStringList StopBitsTypeStringALL;
    QStringList DataBitsTypeString;
    QStringList ParityTypeString;
    QStringList StopBitsTypeString;
    QStringList FlowTypeString;
    */

private slots:
};

#endif // GCSCONTROLGADGETOPTIONSPAGE_H
