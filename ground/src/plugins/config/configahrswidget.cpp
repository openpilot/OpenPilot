/**
 ******************************************************************************
 *
 * @file       configahrswidget.h
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
#include "configahrswidget.h"

#include <QDebug>
#include <QTimer>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>



ConfigAHRSWidget::ConfigAHRSWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_ahrs = new Ui_AHRSWidget();
    m_ahrs->setupUi(this);


    m_ahrs->ahrsBargraph->setScene(new QGraphicsScene(this));

    QSvgRenderer *renderer = new QSvgRenderer();
    ahrsbargraph = new QGraphicsSvgItem();
    renderer->load(QString(":/configgadget/images/ahrs-calib.svg"));
    ahrsbargraph->setSharedRenderer(renderer);
    ahrsbargraph->setElementId("background");
    ahrsbargraph->setObjectName("background");
    m_ahrs->ahrsBargraph->scene()->addItem(ahrsbargraph);
    m_ahrs->ahrsBargraph->setSceneRect(ahrsbargraph->boundingRect());

    // Initialize the 9 bargraph values:

    QMatrix lineMatrix = renderer->matrixForElement("accel_x");
    qreal startX = lineMatrix.mapRect(renderer->boundsOnElement("accel_x")).x();
    qreal startY = lineMatrix.mapRect(renderer->boundsOnElement("accel_x")).y();
    // Then once we have the initial location, we can put it
    // into a QGraphicsSvgItem which we will display at the same
    // place: we do this so that the heading scale can be clipped to
    // the compass dial region.
    accel_x = new QGraphicsSvgItem();
    accel_x->setSharedRenderer(renderer);
    accel_x->setElementId("accel_x");
    m_ahrs->ahrsBargraph->scene()->addItem(accel_x);
    QTransform matrix;
    matrix.translate(startX,startY);
    accel_x->setTransform(matrix,false);

    lineMatrix = renderer->matrixForElement("accel_y");
    startX = lineMatrix.mapRect(renderer->boundsOnElement("accel_y")).x();
    startY = lineMatrix.mapRect(renderer->boundsOnElement("accel_y")).y();
    accel_y = new QGraphicsSvgItem();
    accel_y->setSharedRenderer(renderer);
    accel_y->setElementId("accel_y");
    m_ahrs->ahrsBargraph->scene()->addItem(accel_y);
    matrix.reset();
    matrix.translate(startX,startY);
    accel_y->setTransform(matrix,false);

    lineMatrix = renderer->matrixForElement("accel_z");
    startX = lineMatrix.mapRect(renderer->boundsOnElement("accel_z")).x();
    startY = lineMatrix.mapRect(renderer->boundsOnElement("accel_z")).y();
    accel_z = new QGraphicsSvgItem();
    accel_z->setSharedRenderer(renderer);
    accel_z->setElementId("accel_z");
    m_ahrs->ahrsBargraph->scene()->addItem(accel_z);
    matrix.reset();
    matrix.translate(startX,startY);
    accel_z->setTransform(matrix,false);



    // Fill the dropdown menus:
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVObject *obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("AHRSSettings")));
    UAVObjectField *field = obj->getField(QString("Algorithm"));
    m_ahrs->algorithm->addItems(field->getOptions());


    // Connect the signals
    connect(m_ahrs->ahrsCalibStart, SIGNAL(clicked()), this, SLOT(launchAHRSCalibration()));

}

ConfigAHRSWidget::~ConfigAHRSWidget()
{
   // Do nothing
}


void ConfigAHRSWidget::showEvent(QShowEvent *event)
{
    m_ahrs->ahrsBargraph->fitInView(ahrsbargraph, Qt::KeepAspectRatio);
}

/**
  Launches the AHRS sensors calibration
  */
void ConfigAHRSWidget::launchAHRSCalibration()
{
    m_ahrs->calibInstructions->setText("Calibration launched...");
    m_ahrs->ahrsCalibStart->setEnabled(false);
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    UAVObject *obj = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("AHRSCalibration")));
    UAVObjectField *field = obj->getField(QString("measure_var"));
    field->setValue("TRUE");
    obj->updated();

    QTimer *waitabit = new QTimer();
    waitabit->setSingleShot(true);
    waitabit->start(15000);
    connect(waitabit, SIGNAL(timeout()), this, SLOT(calibPhase2()));

}


void ConfigAHRSWidget::calibPhase2()
{
    m_ahrs->calibInstructions->setText("Confirming settings...");

    // Now update size of all the graphs in log scale
    // 'log(1/val)'

    m_ahrs->ahrsCalibStart->setEnabled(true);
}


