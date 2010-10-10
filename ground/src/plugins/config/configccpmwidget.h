/**
 ******************************************************************************
 *
 * @file       configccpmtwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief ccpm configuration panel
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
#ifndef CONFIGccpmWIDGET_H
#define CONFIGccpmWIDGET_H

#include "ui_ccpm.h"
#include "configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>
#include <QtGui/QWidget>
#include <QList>

class Ui_Widget;

class ConfigccpmWidget: public ConfigTaskWidget
{
    Q_OBJECT

public:
    ConfigccpmWidget(QWidget *parent = 0);
    ~ConfigccpmWidget();

private:
        Ui_ccpmWidget *m_ccpm;
        QGraphicsSvgItem *SwashplateImg;
        QGraphicsSvgItem *CurveImg;
        QGraphicsSvgItem *ServoW;
        QGraphicsSvgItem *ServoX;
        QGraphicsSvgItem *ServoY;
        QGraphicsSvgItem *ServoZ;

    private slots:
        void requestccpmUpdate();
        void sendccpmUpdate();
        void saveccpmUpdate();
        void ccpmSwashplateUpdate();
        void UpdateCurveSettings();
        void GenerateCurve();

};

#endif // CONFIGccpmWIDGET_H
