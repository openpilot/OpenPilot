/**
 ******************************************************************************
 *
 * @file       opmap_mapoverlaywidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   opmap
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

#include "opmap_mapoverlaywidget.h"
#include "ui_opmap_mapoverlaywidget.h"

OPMap_MapOverlayWidget::OPMap_MapOverlayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OPMap_MapOverlayWidget)
{
    ui->setupUi(this);
}

OPMap_MapOverlayWidget::~OPMap_MapOverlayWidget()
{
    delete ui;
}

void OPMap_MapOverlayWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void OPMap_MapOverlayWidget::on_verticalSlider_sliderMoved(int position)
{

}

void OPMap_MapOverlayWidget::on_dial_sliderMoved(int position)
{

}
