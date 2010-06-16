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
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/positionactual.h"

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
//    void gcsButtonClick();
//    void uavButtonClick(bool checked);

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent* event);

private slots:
   void updatePosition();

private:
//   void addUserControls();

    bool follow_uav;    // true if the map is to stay centered on the UAV

    double heading;	// compass/uav heading

    mapcontrol::OPMapWidget *map;

   QTimer *m_updateTimer;

   PositionActual *m_positionActual;

    QPushButton *gcsButton;
    QPushButton *uavButton;
};

#endif /* OPMAP_GADGETWIDGET_H_ */
