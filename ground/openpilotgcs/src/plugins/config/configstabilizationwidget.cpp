/**
 ******************************************************************************
 *
 * @file       configstabilizationwidget.h
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
#include "configstabilizationwidget.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QList>

ConfigStabilizationWidget::ConfigStabilizationWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_stabilization = new Ui_StabilizationWidget();
    m_stabilization->setupUi(this);

    // To bring old style sheet back without adding it manually do this:
    // Alternatively apply a global stylesheet to the QGroupBox
    // setStyleSheet("QGroupBox {background-color: qlineargradient(spread:pad, x1:0.507, y1:0.869318, x2:0.507, y2:0.0965909, stop:0 rgba(243, 243, 243, 255), stop:1 rgba(250, 250, 250, 255)); border: 1px outset #999; border-radius: 3; }");

    autoLoadWidgets();
    realtimeUpdates=new QTimer(this);
    connect(m_stabilization->realTimeUpdates_6,SIGNAL(stateChanged(int)),this,SLOT(realtimeUpdatesSlot(int)));
    connect(m_stabilization->realTimeUpdates_7,SIGNAL(stateChanged(int)),this,SLOT(realtimeUpdatesSlot(int)));
    connect(realtimeUpdates,SIGNAL(timeout()),this,SLOT(apply()));

    connect(m_stabilization->checkBox_7,SIGNAL(stateChanged(int)),this,SLOT(linkCheckBoxes(int)));
    connect(m_stabilization->checkBox_2,SIGNAL(stateChanged(int)),this,SLOT(linkCheckBoxes(int)));
    connect(m_stabilization->checkBox_8,SIGNAL(stateChanged(int)),this,SLOT(linkCheckBoxes(int)));
    connect(m_stabilization->checkBox_3,SIGNAL(stateChanged(int)),this,SLOT(linkCheckBoxes(int)));

    connect(this,SIGNAL(widgetContentsChanged(QWidget*)),this,SLOT(processLinkedWidgets(QWidget*)));

    disableMouseWheelEvents();

    // This is needed because new style tries to compact things as much as possible in grid
    // and on OSX the widget sizes of PushButtons is reported incorrectly:
    // https://bugreports.qt-project.org/browse/QTBUG-14591
    m_stabilization->saveStabilizationToRAM_6->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->saveStabilizationToSD_6->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->stabilizationReloadBoardData_6->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->saveStabilizationToRAM_7->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->saveStabilizationToSD_7->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->pushButton_2->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->pushButton_3->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->pushButton_4->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->pushButton_19->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->pushButton_20->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->pushButton_21->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->pushButton_22->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->pushButton_23->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    m_stabilization->pushButton_24->setAttribute(Qt::WA_LayoutUsesWidgetRect);
}


ConfigStabilizationWidget::~ConfigStabilizationWidget()
{
    // Do nothing
}

void ConfigStabilizationWidget::realtimeUpdatesSlot(int value)
{
    m_stabilization->realTimeUpdates_6->setCheckState((Qt::CheckState)value);
    m_stabilization->realTimeUpdates_7->setCheckState((Qt::CheckState)value);
    if(value==Qt::Checked && !realtimeUpdates->isActive())
        realtimeUpdates->start(300);
    else if(value==Qt::Unchecked)
        realtimeUpdates->stop();
}

void ConfigStabilizationWidget::linkCheckBoxes(int value)
{
    if(sender()== m_stabilization->checkBox_7)
        m_stabilization->checkBox_3->setCheckState((Qt::CheckState)value);
    else if(sender()== m_stabilization->checkBox_3)
        m_stabilization->checkBox_7->setCheckState((Qt::CheckState)value);
    else if(sender()== m_stabilization->checkBox_8)
        m_stabilization->checkBox_2->setCheckState((Qt::CheckState)value);
    else if(sender()== m_stabilization->checkBox_2)
        m_stabilization->checkBox_8->setCheckState((Qt::CheckState)value);
}

void ConfigStabilizationWidget::processLinkedWidgets(QWidget * widget)
{
    if(m_stabilization->checkBox_7->checkState()==Qt::Checked)
    {
        if(widget== m_stabilization->RateRollKp_2)
        {
            m_stabilization->RatePitchKp->setValue(m_stabilization->RateRollKp_2->value());
        }
        else if(widget== m_stabilization->RateRollKi_2)
        {
            m_stabilization->RatePitchKi->setValue(m_stabilization->RateRollKi_2->value());
        }
        else if(widget== m_stabilization->RateRollILimit_2)
        {
            m_stabilization->RatePitchILimit->setValue(m_stabilization->RateRollILimit_2->value());
        }
        else if(widget== m_stabilization->RatePitchKp)
        {
            m_stabilization->RateRollKp_2->setValue(m_stabilization->RatePitchKp->value());
        }
        else if(widget== m_stabilization->RatePitchKi)
        {
            m_stabilization->RateRollKi_2->setValue(m_stabilization->RatePitchKi->value());
        }
        else if(widget== m_stabilization->RatePitchILimit)
        {
            m_stabilization->RateRollILimit_2->setValue(m_stabilization->RatePitchILimit->value());
        }
    }
    if(m_stabilization->checkBox_8->checkState()==Qt::Checked)
    {
        if(widget== m_stabilization->AttitudeRollKp)
        {
            m_stabilization->AttitudePitchKp_2->setValue(m_stabilization->AttitudeRollKp->value());
        }
        else if(widget== m_stabilization->AttitudeRollKi)
        {
            m_stabilization->AttitudePitchKi_2->setValue(m_stabilization->AttitudeRollKi->value());
        }
        else if(widget== m_stabilization->AttitudeRollILimit)
        {
            m_stabilization->AttitudePitchILimit_2->setValue(m_stabilization->AttitudeRollILimit->value());
        }
        else if(widget== m_stabilization->AttitudePitchKp_2)
        {
            m_stabilization->AttitudeRollKp->setValue(m_stabilization->AttitudePitchKp_2->value());
        }
        else if(widget== m_stabilization->AttitudePitchKi_2)
        {
            m_stabilization->AttitudeRollKi->setValue(m_stabilization->AttitudePitchKi_2->value());
        }
        else if(widget== m_stabilization->AttitudePitchILimit_2)
        {
            m_stabilization->AttitudeRollILimit->setValue(m_stabilization->AttitudePitchILimit_2->value());
        }
    }
}


