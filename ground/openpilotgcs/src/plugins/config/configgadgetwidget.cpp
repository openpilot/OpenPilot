/**
 ******************************************************************************
 *
 * @file       configgadgetwidget.cpp
 * @author     E. Lafargue & The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#include "configgadgetwidget.h"

#include "configvehicletypewidget.h"
#include "configccattitudewidget.h"
#include "configinputwidget.h"
#include "configoutputwidget.h"
#include "configstabilizationwidget.h"
#include "configcamerastabilizationwidget.h"
#include "configtxpidwidget.h"
#include "config_pro_hw_widget.h"
#include "config_cc_hw_widget.h"
#include "configpipxtremewidget.h"
#include "configrevowidget.h"
#include "defaultattitudewidget.h"
#include "defaulthwsettingswidget.h"
#include "uavobjectutilmanager.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>



ConfigGadgetWidget::ConfigGadgetWidget(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    ftw = new MyTabbedStackWidget(this, true, true);
    ftw->setIconSize(64);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(ftw);
    setLayout(layout);

    // *********************
    QWidget *qwd;

    qwd = new DefaultHwSettingsWidget(this);
    ftw->insertTab(ConfigGadgetWidget::hardware, qwd, QIcon(":/configgadget/images/hw_config.png"), QString("HW Settings"));

    qwd = new ConfigVehicleTypeWidget(this);
    ftw->insertTab(ConfigGadgetWidget::aircraft, qwd, QIcon(":/configgadget/images/Airframe.png"), QString("Aircraft"));

    qwd = new ConfigInputWidget(this);
    ftw->insertTab(ConfigGadgetWidget::input, qwd, QIcon(":/configgadget/images/Transmitter.png"), QString("Input"));

    qwd = new ConfigOutputWidget(this);
    ftw->insertTab(ConfigGadgetWidget::output, qwd, QIcon(":/configgadget/images/Servo.png"), QString("Output"));

    qwd = new DefaultAttitudeWidget(this);
    ftw->insertTab(ConfigGadgetWidget::sensors, qwd, QIcon(":/configgadget/images/AHRS-v1.3.png"), QString("INS"));

    qwd = new ConfigStabilizationWidget(this);
    ftw->insertTab(ConfigGadgetWidget::stabilization, qwd, QIcon(":/configgadget/images/gyroscope.png"), QString("Stabilization"));

    qwd = new ConfigCameraStabilizationWidget(this);
    ftw->insertTab(ConfigGadgetWidget::camerastabilization, qwd, QIcon(":/configgadget/images/camera.png"), QString("Camera Stab"));

    qwd = new ConfigTxPIDWidget(this);
    ftw->insertTab(ConfigGadgetWidget::txpid, qwd, QIcon(":/configgadget/images/txpid.png"), QString("TxPID"));

    // *********************
    // Listen to autopilot connection events

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    TelemetryManager* telMngr = pm->getObject<TelemetryManager>();
    connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()));
    connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()));

    // And check whether by any chance we are not already connected
    if (telMngr->isConnected())
        onAutopilotConnect();    

    help = 0;
    connect(ftw,SIGNAL(currentAboutToShow(int,bool*)),this,SLOT(tabAboutToChange(int,bool*)));//,Qt::BlockingQueuedConnection);

    // Connect to the PipXStatus object updates
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    pipxStatusObj = dynamic_cast<UAVDataObject*>(objManager->getObject("PipXStatus"));
    if (pipxStatusObj != NULL ) {
        connect(pipxStatusObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updatePipXStatus(UAVObject*)));
    } else {
	qDebug() << "Error: Object is unknown (PipXStatus).";
    }

    // Create the timer that is used to timeout the connection to the PipX.
    pipxTimeout = new QTimer(this);
    connect(pipxTimeout, SIGNAL(timeout()),this,SLOT(onPipxtremeDisconnect()));
    pipxConnected = false;
}

ConfigGadgetWidget::~ConfigGadgetWidget()
{
   // Do nothing

    // TODO: properly delete all the tabs in ftw before exiting
}

void ConfigGadgetWidget::startInputWizard()
{
    ftw->setCurrentIndex(ConfigGadgetWidget::input);
    ConfigInputWidget* inputWidget = dynamic_cast<ConfigInputWidget*>(ftw->getWidget(ConfigGadgetWidget::input));
    Q_ASSERT(inputWidget);
    inputWidget->startInputWizard();
}

void ConfigGadgetWidget::resizeEvent(QResizeEvent *event)
{

    QWidget::resizeEvent(event);
}

void ConfigGadgetWidget::onAutopilotDisconnect() {
    ftw->setCurrentIndex(ConfigGadgetWidget::hardware);
    ftw->removeTab(ConfigGadgetWidget::sensors);
    QWidget *qwd = new DefaultAttitudeWidget(this);
    ftw->insertTab(ConfigGadgetWidget::sensors, qwd, QIcon(":/configgadget/images/AHRS-v1.3.png"), QString("INS"));
    ftw->removeTab(ConfigGadgetWidget::hardware);
    qwd = new DefaultHwSettingsWidget(this);
    ftw->insertTab(ConfigGadgetWidget::hardware, qwd, QIcon(":/configgadget/images/hw_config.png"), QString("HW Settings"));
    ftw->setCurrentIndex(ConfigGadgetWidget::hardware);

    emit autopilotDisconnected();
}

void ConfigGadgetWidget::onAutopilotConnect() {

    qDebug()<<"ConfigGadgetWidget onAutopilotConnect";
    // First of all, check what Board type we are talking to, and
    // if necessary, remove/add tabs in the config gadget:
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager* utilMngr = pm->getObject<UAVObjectUtilManager>();
    if (utilMngr) {
        int board = utilMngr->getBoardModel();
        if ((board & 0xff00) == 1024) {
            // CopterControl family
            // Delete the INS panel, replace with CC Panel:
            ftw->setCurrentIndex(ConfigGadgetWidget::hardware);
            ftw->removeTab(ConfigGadgetWidget::sensors);
            QWidget *qwd = new ConfigCCAttitudeWidget(this);
            ftw->insertTab(ConfigGadgetWidget::sensors, qwd, QIcon(":/configgadget/images/AHRS-v1.3.png"), QString("Attitude"));
            ftw->removeTab(ConfigGadgetWidget::hardware);
            qwd = new ConfigCCHWWidget(this);
            ftw->insertTab(ConfigGadgetWidget::hardware, qwd, QIcon(":/configgadget/images/hw_config.png"), QString("HW Settings"));
            ftw->setCurrentIndex(ConfigGadgetWidget::hardware);
        } else if ((board & 0xff00) == 256 ) {
            // Mainboard family
            Q_ASSERT(0);
            /*
            ftw->setCurrentIndex(ConfigGadgetWidget::hardware);
            ftw->removeTab(ConfigGadgetWidget::sensors);
            QWidget *qwd = new ConfigAHRSWidget(this);
            ftw->insertTab(ConfigGadgetWidget::sensors, qwd, QIcon(":/configgadget/images/AHRS-v1.3.png"), QString("INS"));
            ftw->removeTab(ConfigGadgetWidget::hardware);
            qwd = new ConfigProHWWidget(this);
            ftw->insertTab(ConfigGadgetWidget::hardware, qwd, QIcon(":/configgadget/images/hw_config.png"), QString("HW Settings"));
            ftw->setCurrentIndex(ConfigGadgetWidget::hardware);
            */
        } else if ((board & 0xff00) == 0x0900) {
            // Revolution sensor calibration
            ftw->setCurrentIndex(ConfigGadgetWidget::hardware);
            ftw->removeTab(ConfigGadgetWidget::sensors);
            QWidget *qwd = new ConfigRevoWidget(this);
            ftw->insertTab(ConfigGadgetWidget::sensors, qwd, QIcon(":/configgadget/images/AHRS-v1.3.png"), QString("Revo"));
            ftw->removeTab(ConfigGadgetWidget::hardware);
            qwd = new ConfigProHWWidget(this);
            ftw->insertTab(ConfigGadgetWidget::hardware, qwd, QIcon(":/configgadget/images/hw_config.png"), QString("HW Settings"));
            ftw->setCurrentIndex(ConfigGadgetWidget::hardware);
        }
    }
    emit autopilotConnected();
}

void ConfigGadgetWidget::tabAboutToChange(int i,bool * proceed)
{
    Q_UNUSED(i);
    *proceed=true;
    ConfigTaskWidget * wid=qobject_cast<ConfigTaskWidget *>(ftw->currentWidget());
    if(!wid)
        return;
    if(wid->isDirty())
    {
        int ans=QMessageBox::warning(this,tr("Unsaved changes"),tr("The tab you are leaving has unsaved changes,"
                                                           "if you proceed they will be lost."
                                                           "Do you still want to proceed?"),QMessageBox::Yes,QMessageBox::No);
        if(ans==QMessageBox::No)
            *proceed=false;
        else
            wid->setDirty(false);
    }
}

/*!
  \brief Called by updates to @PipXStatus
  */
void ConfigGadgetWidget::updatePipXStatus(UAVObject *object)
{

	// Restart the disconnection timer.
	pipxTimeout->start(5000);
	if (!pipxConnected)
	{
		qDebug()<<"ConfigGadgetWidget onPipxtremeConnect";
		QWidget *qwd = new ConfigPipXtremeWidget(this);
		ftw->insertTab(ConfigGadgetWidget::pipxtreme, qwd, QIcon(":/configgadget/images/PipXtreme.png"), QString("PipXtreme"));
		ftw->setCurrentIndex(ConfigGadgetWidget::pipxtreme);
		pipxConnected = true;
	}
}

void ConfigGadgetWidget::onPipxtremeDisconnect() {
	qDebug()<<"ConfigGadgetWidget onPipxtremeDisconnect";
	pipxTimeout->stop();
	ftw->removeTab(ConfigGadgetWidget::pipxtreme);
	pipxConnected = false;
}
