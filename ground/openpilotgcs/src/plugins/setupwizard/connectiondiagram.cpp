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
    QDialog(parent), m_configSource(configSource),
    ui(new Ui::ConnectionDiagram)
{
    setWindowTitle(tr("Connection Diagram"));
    ui->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->connectionDiagram->setScene(scene);
    ui->connectionDiagram->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_renderer = new QSvgRenderer();
    if (m_renderer->load(QString(":/setupwizard/resources/connection-diagrams.svg")) && m_renderer->isValid())
    {
        scene->clear();
        QGraphicsSvgItem* ccPic = new QGraphicsSvgItem();
        ccPic->setSharedRenderer(m_renderer);
        ccPic->setElementId("cc");
        scene->addItem(ccPic);
        qDebug() << "Scene complete";

        //ui->connectionDiagram->setSceneRect(ccPic->boundingRect());
        //ui->connectionDiagram->fitInView(ccPic, Qt::KeepAspectRatio);
    }
}

ConnectionDiagram::~ConnectionDiagram()
{
    delete ui;
}
