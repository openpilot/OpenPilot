/**
 ******************************************************************************
 *
 * @file       outputcalibrationpage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup OutputCalibrationPage
 * @{
 * @brief
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

#include "outputcalibrationpage.h"
#include "ui_outputcalibrationpage.h"
#include "systemalarms.h"
#include "uavobjectmanager.h"

OutputCalibrationPage::OutputCalibrationPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent), ui(new Ui::OutputCalibrationPage), m_vehicleBoundsItem(0),
    m_currentWizardIndex(-1), m_calibrationUtil(0)
{
    ui->setupUi(this);

    m_vehicleRenderer = new QSvgRenderer();
    if (QFile::exists(QString(":/setupwizard/resources/multirotor-shapes.svg")) &&
            m_vehicleRenderer->load(QString(":/setupwizard/resources/multirotor-shapes.svg")) &&
            m_vehicleRenderer->isValid())
    {
        m_vehicleScene = new QGraphicsScene(this);
        ui->vehicleView->setScene(m_vehicleScene);
    }
}

OutputCalibrationPage::~OutputCalibrationPage()
{
    if(m_calibrationUtil) {
        delete m_calibrationUtil;
        m_calibrationUtil = 0;
    }
    delete ui;
}

void OutputCalibrationPage::setupVehicle()
{
    m_actuatorSettings = getWizard()->getActuatorSettings();
    m_wizardIndexes.clear();
    m_vehicleElementIds.clear();
    m_vehicleHighlightElementIndexes.clear();
    m_channelIndex.clear();
    m_currentWizardIndex = 0;
    m_vehicleScene->clear();
    switch(getWizard()->getVehicleSubType())
    {
        case SetupWizard::MULTI_ROTOR_TRI_Y:
            m_wizardIndexes << 0 << 1 << 1 << 1 << 2 << 3 << 4;
            m_vehicleElementIds << "tri" << "tri-frame" << "tri-m1" << "tri-m2" << "tri-m3" << "tri-s1";
            m_vehicleHighlightElementIndexes << 0 << 1 << 2 << 3 << 4 << 4 << 4;
            m_channelIndex << 0 << 0 << 1 << 2 << 3 << 3 << 3;
            m_actuatorSettings[3].channelMin = 1500;
            m_actuatorSettings[3].channelNeutral = 1500;
            m_actuatorSettings[3].channelMax = 1500;
            getWizard()->setActuatorSettings(m_actuatorSettings);
            break;
        case SetupWizard::MULTI_ROTOR_QUAD_X:
            m_wizardIndexes << 0 << 1 << 1 << 1 << 1;
            m_vehicleElementIds << "quad-x" << "quad-x-frame" << "quad-x-m1" << "quad-x-m2" << "quad-x-m3" << "quad-x-m4";
            m_vehicleHighlightElementIndexes << 0 << 1 << 2 << 3 << 4;
            m_channelIndex << 0 << 0 << 1 << 2 << 3;
            break;
        case SetupWizard::MULTI_ROTOR_QUAD_PLUS:
            m_wizardIndexes << 0 << 1 << 1 << 1 << 1;
            m_vehicleElementIds << "quad-p" << "quad-p-frame" << "quad-p-m1" << "quad-p-m2" << "quad-p-m3" << "quad-p-m4";
            m_vehicleHighlightElementIndexes << 0 << 1 << 2 << 3 << 4;
            m_channelIndex << 0 << 0 << 1 << 2 << 3;
            break;
        case SetupWizard::MULTI_ROTOR_HEXA:
            m_wizardIndexes << 0 << 1 << 1 << 1 << 1 << 1 << 1;
            m_vehicleElementIds << "hexa" << "hexa-frame" << "hexa-m1" << "hexa-m2" << "hexa-m3" << "hexa-m4" << "hexa-m5" << "hexa-m6";
            m_vehicleHighlightElementIndexes << 0 << 1 << 2 << 3 << 4 << 5 << 6;
            m_channelIndex << 0 << 0 << 1 << 2 << 3 << 4 << 5;
            break;
        case SetupWizard::MULTI_ROTOR_HEXA_COAX_Y:
            m_wizardIndexes << 0 << 1 << 1 << 1 << 1 << 1 << 1;
            m_vehicleElementIds << "hexa-y6" << "hexa-y6-frame" << "hexa-y6-m2" << "hexa-y6-m1" << "hexa-y6-m4" << "hexa-y6-m3" << "hexa-y6-m6" << "hexa-y6-m5";
            m_vehicleHighlightElementIndexes << 0 << 2 << 1 << 4 << 3 << 6 << 5;
            m_channelIndex << 0 << 0 << 1 << 2 << 3 << 4 << 5;
            break;
        case SetupWizard::MULTI_ROTOR_HEXA_H:
            m_wizardIndexes << 0 << 1 << 1 << 1 << 1 << 1 << 1;
            m_vehicleElementIds << "hexa-h" << "hexa-h-frame" << "hexa-h-m1" << "hexa-h-m2" << "hexa-h-m3" << "hexa-h-m4" << "hexa-h-m5" << "hexa-h-m6";
            m_vehicleHighlightElementIndexes << 0 << 1 << 2 << 3 << 4 << 5 << 6;
            m_channelIndex << 0 << 0 << 1 << 2 << 3 << 4 << 5;
           break;
        default:
            break;
    }

    VehicleConfigurationHelper helper(getWizard());
    helper.setupVehicle(false);

    if(m_calibrationUtil) {
        delete m_calibrationUtil;
        m_calibrationUtil = 0;
    }
    m_calibrationUtil = new OutputCalibrationUtil();

    setupVehicleItems();
}

void OutputCalibrationPage::setupVehicleItems()
{
    m_vehicleItems.clear();
    m_vehicleBoundsItem = new QGraphicsSvgItem();
    m_vehicleBoundsItem->setSharedRenderer(m_vehicleRenderer);
    m_vehicleBoundsItem->setElementId(m_vehicleElementIds[0]);
    m_vehicleBoundsItem->setZValue(-1);
    m_vehicleBoundsItem->setOpacity(0);
    m_vehicleScene->addItem(m_vehicleBoundsItem);

    QRectF parentBounds = m_vehicleRenderer->boundsOnElement(m_vehicleElementIds[0]);

    for(int i = 1; i < m_vehicleElementIds.size(); i++)
    {
        QGraphicsSvgItem *item = new QGraphicsSvgItem();
        item->setSharedRenderer(m_vehicleRenderer);
        item->setElementId(m_vehicleElementIds[i]);
        item->setZValue(i);
        item->setOpacity(1.0);

        QRectF itemBounds = m_vehicleRenderer->boundsOnElement(m_vehicleElementIds[i]);
        item->setPos(itemBounds.x() - parentBounds.x(), itemBounds.y() - parentBounds.y());

        m_vehicleScene->addItem(item);
        m_vehicleItems << item;
    }
}

void OutputCalibrationPage::startWizard()
{
    ui->calibrationStack->setCurrentIndex(m_wizardIndexes[0]);
    setupVehicleHighlightedPart();
}

void OutputCalibrationPage::setupVehicleHighlightedPart()
{
    qreal dimOpaque = m_currentWizardIndex == 0 ? 1.0 : 0.3;
    qreal highlightOpaque = 1.0;
    int highlightedIndex = m_vehicleHighlightElementIndexes[m_currentWizardIndex];
    for(int i = 0; i < m_vehicleItems.size(); i++) {
        QGraphicsSvgItem* item = m_vehicleItems[i];
        item->setOpacity((highlightedIndex == i) ? highlightOpaque : dimOpaque);
    }
}

void OutputCalibrationPage::setWizardPage()
{
    qDebug() << "Wizard index: " << m_currentWizardIndex;
    m_calibrationUtil->stopChannelOutput();

    QApplication::processEvents();

    int currentPageIndex = m_wizardIndexes[m_currentWizardIndex];
    qDebug() << "Current page: " << currentPageIndex;
    ui->calibrationStack->setCurrentIndex(currentPageIndex);

    int currentChannel = getCurrentChannel();
    qDebug() << "Current channel: " << currentChannel;
    if(currentChannel >= 0) {
        if(currentPageIndex == 1) {
            ui->motorNeutralSlider->setValue(m_actuatorSettings[currentChannel].channelNeutral);
        }
        else if(currentPageIndex == 2) {
            ui->servoCenterSlider->setValue(m_actuatorSettings[currentChannel].channelNeutral);
        }
        else if(currentPageIndex == 3) {
            ui->servoMinAngleSlider->setMaximum(m_actuatorSettings[currentChannel].channelNeutral);
            ui->servoMinAngleSlider->setValue(m_actuatorSettings[currentChannel].channelMin);
        }
        else if(currentPageIndex == 4) {
            ui->servoMaxAngleSlider->setMinimum(m_actuatorSettings[currentChannel].channelNeutral);
            ui->servoMaxAngleSlider->setValue(m_actuatorSettings[currentChannel].channelMax);
        }
    }
    setupVehicleHighlightedPart();
}

void OutputCalibrationPage::initializePage()
{
    if(m_vehicleScene) {
        setupVehicle();
        startWizard();
    }
}

bool OutputCalibrationPage::validatePage()
{
    if(isFinished()) {
        getWizard()->setActuatorSettings(m_actuatorSettings);
        return true;
    } else {
        m_currentWizardIndex++;
        setWizardPage();
        return false;
    }
}

void OutputCalibrationPage::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    if(m_vehicleBoundsItem) {
        ui->vehicleView->setSceneRect(m_vehicleBoundsItem->boundingRect());
        ui->vehicleView->fitInView(m_vehicleBoundsItem, Qt::KeepAspectRatio);
    }
}

void OutputCalibrationPage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if(m_vehicleBoundsItem) {
        ui->vehicleView->setSceneRect(m_vehicleBoundsItem->boundingRect());
        ui->vehicleView->fitInView(m_vehicleBoundsItem, Qt::KeepAspectRatio);
    }
}

void OutputCalibrationPage::customBackClicked()
{
    if(m_currentWizardIndex > 0)
    {
        m_currentWizardIndex--;
        setWizardPage();
    }
    else
    {
        getWizard()->back();
    }
}

quint16 OutputCalibrationPage::getCurrentChannel()
{
    return  m_channelIndex[m_currentWizardIndex];
}

void OutputCalibrationPage::enableButtons(bool enable)
{
    getWizard()->button(QWizard::NextButton)->setEnabled(enable);
    getWizard()->button(QWizard::CustomButton1)->setEnabled(enable);
    getWizard()->button(QWizard::CancelButton)->setEnabled(enable);
    getWizard()->button(QWizard::BackButton)->setEnabled(enable);
    QApplication::processEvents();
}

void OutputCalibrationPage::on_motorNeutralButton_toggled(bool checked)
{
    ui->motorNeutralButton->setText(checked ? tr("Stop") : tr("Start"));
    quint16 channel = getCurrentChannel();
    onStartButtonToggle(ui->motorNeutralButton, channel, m_actuatorSettings[channel].channelNeutral, 1000, ui->motorNeutralSlider);
}

void OutputCalibrationPage::onStartButtonToggle(QAbstractButton *button, quint16 channel, quint16 value, quint16 safeValue, QSlider *slider) {
    if(button->isChecked()) {
        if(checkAlarms()) {
            enableButtons(false);
            m_calibrationUtil->startChannelOutput(channel, safeValue);
            slider->setValue(value);
            m_calibrationUtil->setChannelOutputValue(value);
        }
        else {
            button->setChecked(false);
        }
    }
    else {
        m_calibrationUtil->stopChannelOutput();
        enableButtons(true);
    }
    debugLogChannelValues();
}

bool OutputCalibrationPage::checkAlarms()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *uavObjectManager = pm->getObject<UAVObjectManager>();
    Q_ASSERT(uavObjectManager);
    SystemAlarms * systemAlarms = SystemAlarms::GetInstance(uavObjectManager);
    Q_ASSERT(systemAlarms);
    SystemAlarms::DataFields data = systemAlarms->getData();

    if(data.Alarm[SystemAlarms::ALARM_ACTUATOR] != SystemAlarms::ALARM_OK) {
        QMessageBox mbox(this);
        mbox.setText(QString(tr("The actuator module is in an error state.\n\n"
                                "Please make sure the correct firmware version is used then "
                                "restart the wizard and try again. If the problem persists please "
                                "consult the openpilot.org support forum.")));
        mbox.setStandardButtons(QMessageBox::Ok);
        mbox.setIcon(QMessageBox::Critical);

        getWizard()->setWindowFlags(getWizard()->windowFlags() & ~Qt::WindowStaysOnTopHint);

        mbox.exec();

        getWizard()->setWindowFlags(getWizard()->windowFlags() | Qt::WindowStaysOnTopHint);
        getWizard()->setWindowIcon(qApp->windowIcon());
        getWizard()->show();
        return false;
    }
    return true;
}

void OutputCalibrationPage::debugLogChannelValues()
{
    quint16 channel = getCurrentChannel();
    qDebug() << "ChannelMin    : " << m_actuatorSettings[channel].channelMin;
    qDebug() << "ChannelNeutral: " << m_actuatorSettings[channel].channelNeutral;
    qDebug() << "ChannelMax    : " << m_actuatorSettings[channel].channelMax;
}

void OutputCalibrationPage::on_motorNeutralSlider_valueChanged(int value)
{
    if(ui->motorNeutralButton->isChecked()) {
        quint16 value = ui->motorNeutralSlider->value();
        m_calibrationUtil->setChannelOutputValue(value);
        m_actuatorSettings[getCurrentChannel()].channelNeutral = value;
        debugLogChannelValues();
    }
}

void OutputCalibrationPage::on_servoCenterButton_toggled(bool checked)
{
    ui->servoCenterButton->setText(checked ? tr("Stop") : tr("Start"));
    quint16 channel = getCurrentChannel();
    quint16 safeValue = m_actuatorSettings[channel].channelNeutral;
    onStartButtonToggle(ui->servoCenterButton, channel, safeValue, safeValue, ui->servoCenterSlider);
}

void OutputCalibrationPage::on_servoCenterSlider_valueChanged(int position)
{
    if(ui->servoCenterButton->isChecked()) {
        quint16 value = ui->servoCenterSlider->value();
        m_calibrationUtil->setChannelOutputValue(value);
        quint16 channel = getCurrentChannel();
        m_actuatorSettings[channel].channelNeutral = value;

        //Adjust min and max
        if(value < m_actuatorSettings[channel].channelMin) {
            m_actuatorSettings[channel].channelMin = value;
        }
        if(value > m_actuatorSettings[channel].channelMax) {
            m_actuatorSettings[channel].channelMax = value;
        }
        debugLogChannelValues();
    }
}

void OutputCalibrationPage::on_servoMinAngleButton_toggled(bool checked)
{
    ui->servoMinAngleButton->setText(checked ? tr("Stop") : tr("Start"));
    quint16 channel = getCurrentChannel();
    quint16 safeValue = m_actuatorSettings[channel].channelNeutral;
    onStartButtonToggle(ui->servoMinAngleButton, channel, m_actuatorSettings[channel].channelMin, safeValue, ui->servoMinAngleSlider);
}

void OutputCalibrationPage::on_servoMinAngleSlider_valueChanged(int position)
{
    if(ui->servoMinAngleButton->isChecked()) {
        quint16 value = ui->servoMinAngleSlider->value();
        m_calibrationUtil->setChannelOutputValue(value);
        m_actuatorSettings[getCurrentChannel()].channelMin = value;
        debugLogChannelValues();
    }
}

void OutputCalibrationPage::on_servoMaxAngleButton_toggled(bool checked)
{
    ui->servoMaxAngleButton->setText(checked ? tr("Stop") : tr("Start"));
    quint16 channel = getCurrentChannel();
    quint16 safeValue = m_actuatorSettings[channel].channelNeutral;
    onStartButtonToggle(ui->servoMaxAngleButton, channel, m_actuatorSettings[channel].channelMax, safeValue, ui->servoMaxAngleSlider);
}

void OutputCalibrationPage::on_servoMaxAngleSlider_valueChanged(int position)
{
    if(ui->servoMaxAngleButton->isChecked()) {
        quint16 value = ui->servoMaxAngleSlider->value();
        m_calibrationUtil->setChannelOutputValue(value);
        m_actuatorSettings[getCurrentChannel()].channelMax = value;
        debugLogChannelValues();
    }
}
