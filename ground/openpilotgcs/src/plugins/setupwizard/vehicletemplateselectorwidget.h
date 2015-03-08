/**
 ******************************************************************************
 *
 * @file       vehicletemplateselectorwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup [Group]
 * @{
 * @addtogroup VehicleTemplateSelectorWidget
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

#ifndef VEHICLETEMPLATESELECTORWIDGET_H
#define VEHICLETEMPLATESELECTORWIDGET_H

#include <QWidget>

namespace Ui {
class VehicleTemplateSelectorWidget;
}

class VehicleTemplateSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VehicleTemplateSelectorWidget(QWidget *parent = 0);
    ~VehicleTemplateSelectorWidget();
    void setTemplateInfo(QString path, int vehicleType, int vehicleSubType) {
        m_templateFolder = path;
        m_vehicleType = vehicleType;
        m_vehicleSubType = vehicleSubType;
        updateTemplates();
    }

private:
    Ui::VehicleTemplateSelectorWidget *ui;
    QString m_templateFolder;
    int m_vehicleType;
    int m_vehicleSubType;

private slots:
    void updateTemplates();
};

#endif // VEHICLETEMPLATESELECTORWIDGET_H
