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
#include "hwsettings.h"

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
    addUAVObjectToWidgetRelation("TelemetrySettings","Speed",m_telemetry->telemetrySpeed);
    addUAVObjectToWidgetRelation("HwSettings","CC_FlexiPort",m_telemetry->cbFlexi);
    addUAVObjectToWidgetRelation("HwSettings","CC_MainPort",m_telemetry->cbTele);
    addUAVObjectToWidgetRelation("HwSettings","CC_RcvrPort",m_telemetry->cbRcvr);
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

    if (((m_telemetry->cbTele->currentIndex() == HwSettings::CC_MAINPORT_TELEMETRY) && (m_telemetry->cbFlexi->currentIndex() == HwSettings::CC_FLEXIPORT_TELEMETRY)) ||
        ((m_telemetry->cbTele->currentIndex() == HwSettings::CC_MAINPORT_GPS) && (m_telemetry->cbFlexi->currentIndex() == HwSettings::CC_FLEXIPORT_GPS)) ||
        ((m_telemetry->cbTele->currentIndex() == HwSettings::CC_MAINPORT_COMAUX) && (m_telemetry->cbFlexi->currentIndex() == HwSettings::CC_FLEXIPORT_COMAUX)))
    {
        enableControls(false);
        m_telemetry->problems->setText(tr("Warning: you have configured both MainPort and FlexiPort for the same function, this currently is not supported"));
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
