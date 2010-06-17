/**
 ******************************************************************************
 *
 * @file       opmapgadgetwidget.h
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

#ifndef OPMAP_GADGETWIDGET_H_
#define OPMAP_GADGETWIDGET_H_

#include "opmapcontrol/opmapcontrol.h"
#include <QtGui/QWidget>
#include <QtGui/QMainWindow>
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/positionactual.h"

namespace Ui { class OPMapControlPanel; }

using namespace mapcontrol;

class OPMapGadgetWidget : public QWidget
{
    Q_OBJECT

public:
    OPMapGadgetWidget(QWidget *parent = 0);
   ~OPMapGadgetWidget();

   void setZoom(int value);
   void setPosition(QPointF pos);
   void setMapProvider(QString provider);

public slots:

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent* event);

private slots:
   void updatePosition();

    // control panel slots
    void on_checkBox_clicked(bool checked);
    void on_comboBox_currentIndexChanged(QString );
    void on_pushButtonGO_clicked();
    void on_pushButtonReload_clicked();
    void on_pushButtonRR_clicked();
    void on_pushButtonRC_clicked();
    void on_pushButtonRL_clicked();
    void on_pushButtonZoomM_clicked();
    void on_pushButtonZoomP_clicked();
    void on_pushButtonGeoFenceM_clicked();
    void on_pushButtonGeoFenceP_clicked();
    void zoomChanged(double zoom);
    void on_checkBox_2_clicked(bool checked);

private:
    bool follow_uav;    // true if the map is to stay centered on the UAV

    double heading;	// compass/uav heading

   QTimer *m_updateTimer;

   PositionActual *m_positionActual;

    Ui::OPMapControlPanel *controlpanel_ui;
    mapcontrol::OPMapWidget *map;
};

#endif /* OPMAP_GADGETWIDGET_H_ */
