/**
 ******************************************************************************
 *
 * @file       GCSControlgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A gadget to control the UAV, either from the keyboard or a joystick
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
#include "logginggadgetwidget.h"
#include "ui_logging.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <loggingplugin.h>

LoggingGadgetWidget::LoggingGadgetWidget(QWidget *parent) : QLabel(parent)
{
    m_logging = new Ui_Logging();
    m_logging->setupUi(this);

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    scpPlugin = pm->getObject<ScopeGadgetFactory>();

}

LoggingGadgetWidget::~LoggingGadgetWidget()
{
   // Do nothing
}

void LoggingGadgetWidget::setPlugin(LoggingPlugin * p)
{
    loggingPlugin = p;
    connect(p,SIGNAL(stateChanged(QString)),this,SLOT(stateChanged(QString)));
    connect(m_logging->playButton,SIGNAL(clicked()),p->getLogfile(),SLOT(resumeReplay()));
    connect(m_logging->playButton, SIGNAL(clicked()), scpPlugin, SLOT(startPlotting()));
    connect(m_logging->pauseButton,SIGNAL(clicked()),p->getLogfile(),SLOT(pauseReplay()));
    connect(m_logging->pauseButton, SIGNAL(clicked()), scpPlugin, SLOT(stopPlotting()));
    connect(m_logging->playbackSpeed,SIGNAL(valueChanged(double)),p->getLogfile(),SLOT(setReplaySpeed(double)));
    void pauseReplay();
    void resumeReplay();
}


void LoggingGadgetWidget::stateChanged(QString status)
{
    m_logging->statusLabel->setText(status);
}

/**
  * @}
  * @}
  */
