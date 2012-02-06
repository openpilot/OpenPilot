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
   /* QList<int> * rateGroup=new QList<int>();
    rateGroup->append(0);
     addApplySaveButtons(m_stabilization->saveStabilizationToRAM,m_stabilization->saveStabilizationToSD);
    addUAVObjectToWidgetRelation("StabilizationSettings","RollRatePID",m_stabilization->rateRollKp,"Kp",1,true,rateGroup);

    addDefaultButton(m_stabilization->defaultButton,0);
    addReloadButton(m_stabilization->reloadButton,0);
    addWidgetToDefaultReloadGroups(m_stabilization->rateRollKp,rateGroup);
    addShadowWidget("StabilizationSettings","RollRatePID",m_stabilization->rateRollKi,0,1,true,rateGroup);*/
    autoLoadWidgets();
}


ConfigStabilizationWidget::~ConfigStabilizationWidget()
{
   // Do nothing
}


