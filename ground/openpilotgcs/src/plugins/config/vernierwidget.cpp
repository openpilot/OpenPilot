/**
 ******************************************************************************
 *
 * @file       mixercurvewidget.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief A widget which displays an adjustable mixer curve
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

#include "vernierwidget.h"

#include <QtGui>
#include <QDebug>



/*
 * Initialize the widget
 */
VernierWidget::VernierWidget(QWidget *parent) : QWidget(parent)
{

    m_vernier = new Ui_VernierWidget();
    m_vernier->setupUi(this);


    curveMin=0.0;
    curveMax=1.0;


}

VernierWidget::~VernierWidget()
{

}


void VernierWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // Thit fitInView method should only be called now, once the
    // widget is shown, otherwise it cannot compute its values and
    // the result is usually a ahrsbargraph that is way too small.
    //fitInView(plot, Qt::KeepAspectRatio);

}

void VernierWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    //fitInView(plot, Qt::KeepAspectRatio);
}


void VernierWidget::setMin(double value)
{
    curveMin = value;
}
void VernierWidget::setMax(double value)
{
    curveMax = value;
}
void VernierWidget::setRange(double min, double max)
{
    curveMin = min;
    curveMax = max;
}
