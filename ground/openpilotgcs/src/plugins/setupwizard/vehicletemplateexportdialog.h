/**
 ******************************************************************************
 *
 * @file       vehicletemplateexportdialog.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup [Group]
 * @{
 * @addtogroup VehicleTemplateExportDialog
 * @{
 * @brief [Brief]
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

#ifndef VEHICLETEMPLATEEXPORTDIALOG_H
#define VEHICLETEMPLATEEXPORTDIALOG_H

#include <QDialog>
#include "uavobjectmanager.h"
#include "vehicleconfigurationsource.h"

namespace Ui {
class VehicleTemplateExportDialog;
}

class VehicleTemplateExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit VehicleTemplateExportDialog(QWidget *parent = 0);
    ~VehicleTemplateExportDialog();

public slots:
    void accept();

private slots:
    void importImage();

private:
    Ui::VehicleTemplateExportDialog *ui;
    UAVObjectManager *m_uavoManager;
    QString setupVehicleType();
    VehicleConfigurationSource::VEHICLE_TYPE m_type;
    VehicleConfigurationSource::VEHICLE_SUB_TYPE m_subType;
    QPixmap m_image;
};

#endif // VEHICLETEMPLATEEXPORTDIALOG_H
