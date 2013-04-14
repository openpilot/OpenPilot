/**
 ******************************************************************************
 *
 * @file       uploadergadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup YModemUploader YModem Serial Uploader Plugin
 * @{
 * @brief The YModem protocol serial uploader plugin
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

#include "uploadergadgetoptionspage.h"
#include "uploadergadgetconfiguration.h"
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QTextEdit>
#include <QtGui/QComboBox>
#include <QtAlgorithms>
#include <QStringList>


UploaderGadgetOptionsPage::UploaderGadgetOptionsPage(UploaderGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
}

//creates options page widget
QWidget *UploaderGadgetOptionsPage::createPage(QWidget *parent)
{
    Q_UNUSED(parent);

    //main widget
    QWidget *widget = new QWidget;

    return widget;
}
/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void UploaderGadgetOptionsPage::apply()
{

}

void UploaderGadgetOptionsPage::finish()
{

}


