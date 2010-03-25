/**
 ******************************************************************************
 *
 * @file       mapgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   map
 * @{
 * 
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

#include "mapgadgetoptionspage.h"
#include "mapgadgetconfiguration.h"
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>


MapGadgetOptionsPage::MapGadgetOptionsPage(IUAVGadgetConfiguration *config, QObject *parent) :
    IOptionsPage(parent),
    m_config(qobject_cast<MapGadgetConfiguration*>(config))
{
}

QWidget *MapGadgetOptionsPage::createPage(QWidget *parent)
{
    QWidget *widget = new QWidget;
    QVBoxLayout *vl = new QVBoxLayout();
    widget->setLayout(vl);

    QHBoxLayout *zoomLayout = new QHBoxLayout();
    QWidget *x = new QWidget;
    x->setLayout(zoomLayout);
    QWidget *label = new QLabel("Default zoom:");
    m_zoomSpin = new QSpinBox();
    m_zoomSpin->setMaximum(18);
    zoomLayout->addWidget(label);
    zoomLayout->addWidget(m_zoomSpin);

    QHBoxLayout *latLayout = new QHBoxLayout();
    QWidget *y = new QWidget;
    y->setLayout(latLayout);
    label = new QLabel("Default latitude:");
    m_latSpin = new QDoubleSpinBox();
    m_latSpin->setDecimals(8);
    m_latSpin->setMinimum(-90);
    m_latSpin->setMaximum(90);
    latLayout->addWidget(label);
    latLayout->addWidget(m_latSpin);


    QHBoxLayout *longLayout = new QHBoxLayout();
    QWidget *z = new QWidget;
    z->setLayout(longLayout);
    label = new QLabel("Default longitude:");
    m_longSpin = new QDoubleSpinBox();
    m_longSpin->setDecimals(8);
    m_longSpin->setMinimum(-180);
    m_longSpin->setMaximum(180);
    longLayout->addWidget(label);
    longLayout->addWidget(m_longSpin);
    QSpacerItem *item = new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding);

    vl->addWidget(x);
    vl->addWidget(y);
    vl->addWidget(z);

    m_zoomSpin->setValue(m_config->zoom());
    m_latSpin->setValue(m_config->latitude());
    m_longSpin->setValue(m_config->longitude());

    return widget;
}

void MapGadgetOptionsPage::apply()
{
    m_config->setZoom(m_zoomSpin->value());
    m_config->setLatitude(m_latSpin->value());
    m_config->setLongitude(m_longSpin->value());

}

void MapGadgetOptionsPage::finish()
{

}

