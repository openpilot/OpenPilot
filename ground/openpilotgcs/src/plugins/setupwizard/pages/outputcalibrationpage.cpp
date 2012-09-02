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

OutputCalibrationPage::OutputCalibrationPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent), ui(new Ui::OutputCalibrationPage), m_vehicleBoundsItem(0),
    m_currentWizardIndex(0), m_calibrationUtil(0)
{
    ui->setupUi(this);

    m_calibrationUtil = new OutputCalibrationUtil();

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
    }
    delete ui;
}

void OutputCalibrationPage::setupVehicle()
{
    m_wizardIndexes.clear();
    m_vehicleElementIds.clear();
    m_vehicleHighlightElementIndexes.clear();
    m_currentWizardIndex = 0;
    m_vehicleScene->clear();
    switch(getWizard()->getVehicleSubType())
    {
        case SetupWizard::MULTI_ROTOR_TRI_Y:
            m_wizardIndexes << 0 << 1 << 2 << 1 << 2 << 1 << 2 << 3 << 4;
            m_vehicleElementIds << "tri" << "tri-frame" << "tri-m1" << "tri-m2" << "tri-m3" << "tri-s1";
            m_vehicleHighlightElementIndexes << 0 << 1 << 1 << 2 << 2 << 3 << 3 << 4 << 4;
            break;
        case SetupWizard::MULTI_ROTOR_QUAD_X:
            m_wizardIndexes << 0 << 1 << 2 << 1 << 2 << 1 << 2 << 1 << 2;
            m_vehicleElementIds << "quad-x" << "quad-x-frame" << "quad-x-m1" << "quad-x-m2" << "quad-x-m3" << "quad-x-m4";
            m_vehicleHighlightElementIndexes << 0 << 1 << 1 << 2 << 2 << 3 << 3 << 4 << 4;
            break;
        case SetupWizard::MULTI_ROTOR_QUAD_PLUS:
            m_wizardIndexes << 0 << 1 << 2 << 1 << 2 << 1 << 2 << 1 << 2;
            m_vehicleElementIds << "quad-p" << "quad-p-frame" << "quad-p-m1" << "quad-p-m2" << "quad-p-m3" << "quad-p-m4";
            m_vehicleHighlightElementIndexes << 0 << 1 << 1 << 2 << 2 << 3 << 3 << 4 << 4;
            break;
        case SetupWizard::MULTI_ROTOR_HEXA:
            m_wizardIndexes << 0 << 1 << 2 << 1 << 2 << 1 << 2 << 1 << 2 << 1 << 2 << 1 << 2;
            m_vehicleElementIds << "hexa" << "hexa-frame" << "hexa-m1" << "hexa-m2" << "hexa-m3" << "hexa-m4" << "hexa-m5" << "hexa-m6";
            m_vehicleHighlightElementIndexes << 0 << 1 << 1 << 2 << 2 << 3 << 3 << 4 << 4 << 5 << 5 << 6 << 6;
            break;
        case SetupWizard::MULTI_ROTOR_HEXA_COAX_Y:
            m_wizardIndexes << 0 << 1 << 2 << 1 << 2 << 1 << 2 << 1 << 2 << 1 << 2 << 1 << 2;
            m_vehicleElementIds << "hexa-y6" << "hexa-y6-frame" << "hexa-y6-m1" << "hexa-y6-m2" << "hexa-y6-m3" << "hexa-y6-m4" << "hexa-y6-m5" << "hexa-y6-m6";
            m_vehicleHighlightElementIndexes << 0 << 1 << 1 << 2 << 2 << 3 << 3 << 4 << 4 << 5 << 5 << 6 << 6;
            break;
        case SetupWizard::MULTI_ROTOR_HEXA_H:
            m_wizardIndexes << 0 << 1 << 2 << 1 << 2 << 1 << 2 << 1 << 2 << 1 << 2 << 1 << 2;
            m_vehicleElementIds << "hexa-h" << "hexa-h-frame" << "hexa-h-m1" << "hexa-h-m2" << "hexa-h-m3" << "hexa-h-m4" << "hexa-h-m5" << "hexa-h-m6";
            m_vehicleHighlightElementIndexes << 0 << 1 << 1 << 2 << 2 << 3 << 3 << 4 << 4 << 5 << 5 << 6 << 6;
            break;
        default:
            break;
    }
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
    m_calibrationUtil->stopChannelOutput();
    ui->backPageButton->setEnabled(m_currentWizardIndex > 0);
    ui->nextPageButton->setEnabled(m_currentWizardIndex < (m_wizardIndexes.size() - 1));
    ui->calibrationStack->setCurrentIndex(m_wizardIndexes.at(m_currentWizardIndex));
    setupVehicleHighlightedPart();
}

void OutputCalibrationPage::initializePage()
{
    if(m_vehicleScene) {
        setupVehicle();
        startWizard();
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

void OutputCalibrationPage::on_nextPageButton_clicked()
{
    if(m_currentWizardIndex < m_wizardIndexes.size()) {
        m_currentWizardIndex++;
        setWizardPage();
    }
}

void OutputCalibrationPage::on_backPageButton_clicked()
{
    if(m_currentWizardIndex > 0) {
        m_currentWizardIndex--;
        setWizardPage();
    }
}

void OutputCalibrationPage::on_motorNeutralButton_toggled(bool checked)
{
    if(checked) {
        m_calibrationUtil->startChannelOutput(m_vehicleHighlightElementIndexes[m_currentWizardIndex]);
        m_calibrationUtil->setChannelOutputValue(ui->motorNeutralSlider->value());
    }
    else {
        m_calibrationUtil->setChannelOutputValue(ui->motorNeutralSlider->minimum());
        m_calibrationUtil->stopChannelOutput();
    }
}

void OutputCalibrationPage::on_motorNeutralSlider_valueChanged(int value)
{
    if(ui->motorNeutralButton->isChecked())
    {
        m_calibrationUtil->setChannelOutputValue(ui->motorNeutralSlider->value());
    }
}

void OutputCalibrationPage::on_motorMaxButton_toggled(bool checked)
{

}

void OutputCalibrationPage::on_motorMaxSlider_valueChanged(int position)
{

}

void OutputCalibrationPage::on_servoCenterButton_toggled(bool checked)
{

}

void OutputCalibrationPage::on_servoAngleButton_toggled(bool checked)
{

}

void OutputCalibrationPage::on_servoMaxAngleSlider_valueChanged(int position)
{

}

void OutputCalibrationPage::on_servoMinAngleSlider_valueChanged(int position)
{

}

void OutputCalibrationPage::on_servoCenterSlider_valueChanged(int position)
{

}
