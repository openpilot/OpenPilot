/**
 ******************************************************************************
 *
 * @file       connectiondiagram.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup ConnectionDiagram
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

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include "connectiondiagram.h"
#include "ui_connectiondiagram.h"

const char *ConnectionDiagram::FILE_NAME   = ":/setupwizard/resources/connection-diagrams.svg";
const int ConnectionDiagram::IMAGE_PADDING = 10;

ConnectionDiagram::ConnectionDiagram(QWidget *parent, VehicleConfigurationSource *configSource) :
    QDialog(parent), ui(new Ui::ConnectionDiagram), m_configSource(configSource)
{
    ui->setupUi(this);
    setWindowTitle(tr("Connection Diagram"));
    setupGraphicsScene();
}

ConnectionDiagram::~ConnectionDiagram()
{
    delete ui;
}

void ConnectionDiagram::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    fitInView();
}

void ConnectionDiagram::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    fitInView();
}

void ConnectionDiagram::fitInView()
{
    ui->connectionDiagram->setSceneRect(m_scene->itemsBoundingRect());
    ui->connectionDiagram->fitInView(
        m_scene->itemsBoundingRect().adjusted(-IMAGE_PADDING, -IMAGE_PADDING, IMAGE_PADDING, IMAGE_PADDING),
        Qt::KeepAspectRatio);
}

void ConnectionDiagram::setupGraphicsScene()
{
    m_renderer = new QSvgRenderer();
    if (QFile::exists(QString(FILE_NAME)) &&
        m_renderer->load(QString(FILE_NAME)) &&
        m_renderer->isValid()) {
        m_scene = new QGraphicsScene(this);
        ui->connectionDiagram->setScene(m_scene);

        QList<QString> elementsToShow;

        switch (m_configSource->getControllerType()) {
        case VehicleConfigurationSource::CONTROLLER_REVO:
            elementsToShow << "controller-revo";
            break;
        case VehicleConfigurationSource::CONTROLLER_NANO:
            elementsToShow << "controller-nano";
            break;
        case VehicleConfigurationSource::CONTROLLER_OPLINK:
        default:
            elementsToShow << "controller-cc";
            break;
        }

        switch (m_configSource->getVehicleType()) {
        case VehicleConfigurationSource::VEHICLE_MULTI:
            switch (m_configSource->getVehicleSubType()) {
            case VehicleConfigurationSource::MULTI_ROTOR_TRI_Y:
                elementsToShow << "tri";
                break;
            case VehicleConfigurationSource::MULTI_ROTOR_QUAD_X:
                elementsToShow << "quad-x";
                break;
            case VehicleConfigurationSource::MULTI_ROTOR_QUAD_PLUS:
                elementsToShow << "quad-p";
                break;
            case VehicleConfigurationSource::MULTI_ROTOR_HEXA:
                elementsToShow << "hexa";
                break;
            case VehicleConfigurationSource::MULTI_ROTOR_HEXA_X:
                elementsToShow << "hexa-x";
                break;
            case VehicleConfigurationSource::MULTI_ROTOR_HEXA_COAX_Y:
                elementsToShow << "hexa-y";
                break;
            case VehicleConfigurationSource::MULTI_ROTOR_HEXA_H:
                elementsToShow << "hexa-h";
                break;
            default:
                break;
            }
            break;
        case VehicleConfigurationSource::VEHICLE_FIXEDWING:
            switch (m_configSource->getVehicleSubType()) {
            case VehicleConfigurationSource::FIXED_WING_DUAL_AILERON:
                elementsToShow << "aileron";
                break;
            case VehicleConfigurationSource::FIXED_WING_AILERON:
                elementsToShow << "aileron-single";
                break;
            case VehicleConfigurationSource::FIXED_WING_ELEVON:
                elementsToShow << "elevon";
                break;
            case VehicleConfigurationSource::FIXED_WING_VTAIL:
                elementsToShow << "vtail";
                break;
            default:
                break;
            }
        case VehicleConfigurationSource::VEHICLE_SURFACE:
            switch (m_configSource->getVehicleSubType()) {
            case VehicleConfigurationSource::GROUNDVEHICLE_CAR:
                elementsToShow << "car";
                break;
            case VehicleConfigurationSource::GROUNDVEHICLE_DIFFERENTIAL:
                elementsToShow << "tank";
                break;
            case VehicleConfigurationSource::GROUNDVEHICLE_MOTORCYCLE:
                elementsToShow << "motorbike";
                break;
            default:
                break;
            }
        case VehicleConfigurationSource::VEHICLE_HELI:
        default:
            break;
        }

        QString prefix = "";
        QString suffix = "";
        switch (m_configSource->getControllerType()) {
        case VehicleConfigurationSource::CONTROLLER_REVO:
            prefix = "revo-";
            break;
        case VehicleConfigurationSource::CONTROLLER_NANO:
            prefix = "nano-";
            break;
        default:
            break;
        }

        switch (m_configSource->getInputType()) {
        case VehicleConfigurationSource::INPUT_PWM:
            elementsToShow << QString("%1pwm").arg(prefix);
            break;
        case VehicleConfigurationSource::INPUT_PPM:
            elementsToShow << QString("%1ppm%2").arg(prefix).arg(suffix);
            break;
        case VehicleConfigurationSource::INPUT_SBUS:
            elementsToShow << QString("%1sbus").arg(prefix);
            break;
        case VehicleConfigurationSource::INPUT_DSM:
            elementsToShow << QString("%1satellite").arg(prefix);
            break;
        default:
            break;
        }

        if (m_configSource->getVehicleType() == VehicleConfigurationSource::VEHICLE_FIXEDWING &&
            m_configSource->getAirspeedType() != VehicleConfigurationSource::AIRSPEED_ESTIMATE) {
            switch (m_configSource->getAirspeedType()) {
            case VehicleConfigurationSource::AIRSPEED_EAGLETREE:
                elementsToShow << QString("%1eagletree-speed-sensor").arg(prefix);
                break;
            case VehicleConfigurationSource::AIRSPEED_MS4525:
                elementsToShow << QString("%1ms4525-speed-sensor").arg(prefix);
                break;
            default:
                break;
            }
        }

        if (m_configSource->getInputType() == VehicleConfigurationSource::INPUT_SBUS) {
            prefix = QString("flexi-%1").arg(prefix);
        }
        switch (m_configSource->getGpsType()) {
        case VehicleConfigurationSource::GPS_DISABLED:
            break;
        case VehicleConfigurationSource::GPS_NMEA:
            elementsToShow << QString("%1generic-nmea").arg(prefix);
            break;
        case VehicleConfigurationSource::GPS_PLATINUM:
            elementsToShow << QString("%1OPGPS-v9").arg(prefix);
            break;
        case VehicleConfigurationSource::GPS_UBX:
            elementsToShow << QString("%1OPGPS-v8-ublox").arg(prefix);
            break;
        default:
            break;
        }

        setupGraphicsSceneItems(elementsToShow);
        fitInView();
        qDebug() << "Scene complete";
    }
}

void ConnectionDiagram::setupGraphicsSceneItems(QList<QString> elementsToShow)
{
    qreal z = 0;

    foreach(QString elementId, elementsToShow) {
        if (m_renderer->elementExists(elementId)) {
            QGraphicsSvgItem *element = new QGraphicsSvgItem();
            element->setSharedRenderer(m_renderer);
            element->setElementId(elementId);
            element->setZValue(z++);
            element->setOpacity(1.0);

            QMatrix matrix = m_renderer->matrixForElement(elementId);
            QRectF orig    = matrix.mapRect(m_renderer->boundsOnElement(elementId));
            element->setPos(orig.x(), orig.y());
            m_scene->addItem(element);
            qDebug() << "Adding " << elementId << " to scene at " << element->pos();
        } else {
            qDebug() << "Element with id: " << elementId << " not found.";
        }
    }
}

void ConnectionDiagram::on_saveButton_clicked()
{
    QImage image(2200, 1100, QImage::Format_ARGB32);

    image.fill(Qt::white);
    QPainter painter(&image);
    m_scene->render(&painter);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Images (*.png *.xpm *.jpg)"));
    if (!fileName.isEmpty()) {
        image.save(fileName);
    }
}
