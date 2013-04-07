/**
 ******************************************************************************
 *
 * @file       lineardialgadgetoptionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup LinearDialPlugin Linear Dial Plugin
 * @{
 * @brief Impliments a gadget that displays linear gauges 
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

#ifndef LINEARDIALGADGETOPTIONSPAGE_H
#define LINEARDIALGADGETOPTIONSPAGE_H

#include "coreplugin/dialogs/ioptionspage.h"
#include <QString>
#include <QFont>
#include <QStringList>
#include <QDebug>

namespace Core {
class IUAVGadgetConfiguration;
}

class LineardialGadgetConfiguration;

namespace Ui {
    class LineardialGadgetOptionsPage;
}

using namespace Core;

class LineardialGadgetOptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit LineardialGadgetOptionsPage(LineardialGadgetConfiguration *config, QObject *parent = 0);

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

private:
    Ui::LineardialGadgetOptionsPage *options_page;
    LineardialGadgetConfiguration *m_config;
    QFont font;

private slots:
    void on_fontPicker_clicked();
    void on_objectName_currentIndexChanged(QString val);
    void on_rangeMin_valueChanged(double val);
    void on_rangeMax_valueChanged(double val);

};

#endif // LINEARDIALGADGETOPTIONSPAGE_H
