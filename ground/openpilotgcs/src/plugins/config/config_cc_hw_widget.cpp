/**
 ******************************************************************************
 *
 * @file       configtelemetrywidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware
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
#include "config_cc_hw_widget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QDesktopServices>
#include <QUrl>

ConfigCCHWWidget::ConfigCCHWWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_telemetry = new Ui_CC_HW_Widget();
    m_telemetry->setupUi(this);
    setupButtons(m_telemetry->saveTelemetryToRAM,m_telemetry->saveTelemetryToSD);
    addUAVObjectToWidgetRelation("HwSettings","CC_FlexiPort",m_telemetry->cbFlexi);
    addUAVObjectToWidgetRelation("HwSettings","CC_MainPort",m_telemetry->cbTele);
    addUAVObjectToWidgetRelation("HwSettings","CC_RcvrPort",m_telemetry->cbRcvr);
    addUAVObjectToWidgetRelation("HwSettings","TelemetrySpeed",m_telemetry->telemetrySpeed);
    addUAVObjectToWidgetRelation("HwSettings","GPSSpeed",m_telemetry->gpsSpeed);
    connect(m_telemetry->cchwHelp,SIGNAL(clicked()),this,SLOT(openHelp()));
    enableControls(false);
    populateWidgets();
    refreshWidgetsValues();
}

ConfigCCHWWidget::~ConfigCCHWWidget()
{
    // Do nothing
}

void ConfigCCHWWidget::refreshValues()
{
}

void ConfigCCHWWidget::widgetsContentsChanged()
{
    ConfigTaskWidget::widgetsContentsChanged();
    enableControls(false);
    if((m_telemetry->cbFlexi->currentText()==m_telemetry->cbTele->currentText()) && m_telemetry->cbTele->currentText()!="Disabled")
    {
        m_telemetry->problems->setText("Warning: you have configured the MainPort and the FlexiPort for the same function, this is currently not suported");
    }
    else
    {
        m_telemetry->problems->setText("");
        enableControls(true);
    }
}

void ConfigCCHWWidget::openHelp()
{
    QDesktopServices::openUrl( QUrl("http://wiki.openpilot.org/display/Doc/CopterControl+HW+Settings", QUrl::StrictMode) );
}

/**
  * @}
  * @}
  */

