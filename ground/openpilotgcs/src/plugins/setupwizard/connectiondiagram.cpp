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
#include "connectiondiagram.h"
#include "ui_connectiondiagram.h"

ConnectionDiagram::ConnectionDiagram(QWidget *parent, VehicleConfigurationSource* configSource) :
    QDialog(parent), ui(new Ui::ConnectionDiagram), m_configSource(configSource), m_background(0)
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
    ui->connectionDiagram->fitInView(m_background, Qt::KeepAspectRatio);
}

void ConnectionDiagram::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);
    ui->connectionDiagram->fitInView(m_background, Qt::KeepAspectRatio);
}

void ConnectionDiagram::setupGraphicsScene()
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->connectionDiagram->setScene(scene);
    ui->connectionDiagram->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_renderer = new QSvgRenderer();
    if (QFile::exists(QString(":/setupwizard/resources/connection-diagrams.svg")) &&
            m_renderer->load(QString(":/setupwizard/resources/connection-diagrams.svg")) &&
            m_renderer->isValid())
    {
        scene->clear();
        m_background = new QGraphicsSvgItem();
        m_background->setSharedRenderer(m_renderer);
        m_background->setElementId("background");
        m_background->setVisible(true);
        m_background->setFlags(QGraphicsItem::ItemClipsToShape);
        m_background->setZValue(-1);
        scene->addItem(m_background);

        QList<QString> elementsToShow;

        switch(m_configSource->getControllerType())
        {
            case VehicleConfigurationSource::CONTROLLER_CC:
            case VehicleConfigurationSource::CONTROLLER_CC3D:
            case VehicleConfigurationSource::CONTROLLER_REVO:
            case VehicleConfigurationSource::CONTROLLER_PIPX:
            default:
                elementsToShow << "controller";
                break;
        }

        switch (m_configSource->getVehicleType())
        {
            case VehicleConfigurationSource::VEHICLE_MULTI:
                switch (m_configSource->getVehicleSubType())
                {
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
            case VehicleConfigurationSource::VEHICLE_HELI:
            case VehicleConfigurationSource::VEHICLE_SURFACE:
            default:
                break;
        }

        switch (m_configSource->getInputType())
        {
            case VehicleConfigurationSource::INPUT_PWM:
                elementsToShow << "receiver" << "pwm" ;
                break;
            case VehicleConfigurationSource::INPUT_PPM:
                elementsToShow << "receiver" << "ppm";
                break;
            case VehicleConfigurationSource::INPUT_SBUS:
                elementsToShow << "sbus";
                break;
            case VehicleConfigurationSource::INPUT_DSM:
                elementsToShow << "satellite";
                break;
            default:
                break;
        }

        setupGraphicsSceneItems(scene, elementsToShow);

        ui->connectionDiagram->setSceneRect(m_background->boundingRect());
        ui->connectionDiagram->fitInView(m_background, Qt::KeepAspectRatio);

        qDebug() << "Scene complete";
    }
}

void ConnectionDiagram::setupGraphicsSceneItems(QGraphicsScene *scene, QList<QString> elementsToShow)
{
    qreal z = 0;
    foreach(QString elementId, elementsToShow) {
        if(m_renderer->elementExists(elementId)) {
            QGraphicsSvgItem* element = new QGraphicsSvgItem();
            element->setSharedRenderer(m_renderer);
            element->setElementId(elementId);
            element->setVisible(true);
            element->setZValue(z++);
            scene->addItem(element);

            QMatrix matrix = m_renderer->matrixForElement(elementId);
            QRectF orig = matrix.mapRect(m_renderer->boundsOnElement(elementId));
            element->setPos(orig.x(),orig.y());
            qDebug() << "Adding " << elementId << " to scene at " << element->pos();
        }
        else
        {
            qDebug() << "Element " << elementId << " not found in renderer!";
        }
    }
}


