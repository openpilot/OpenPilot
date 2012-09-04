/**
 ******************************************************************************
 *
 * @file       homeeditor.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin OpenPilot Map Plugin
 * @{
 * @brief The OpenPilot Map plugin
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
#include "homeeditor.h"
#include "ui_homeeditor.h"

homeEditor::homeEditor(HomeItem *home, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::homeEditor),
    myhome(home)
{
    if(!home)
    {
        deleteLater();
        return;
    }
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose,true);
    ui->altitude->setValue(home->Altitude());
    ui->latitude->setValue(home->Coord().Lat());
    ui->longitude->setValue(home->Coord().Lng());
    this->show();
}

homeEditor::~homeEditor()
{
    delete ui;
}

void homeEditor::on_buttonBox_accepted()
{
    myhome->SetCoord(internals::PointLatLng(ui->latitude->value(),ui->longitude->value()));
    myhome->SetAltitude(ui->altitude->value());
}

void homeEditor::on_buttonBox_rejected()
{
    this->close();
}
