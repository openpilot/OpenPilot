/**
 ******************************************************************************
 *
 * @file       mapgadgetwidget.h
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

#ifndef MAPGADGETWIDGET_H_
#define MAPGADGETWIDGET_H_

#include "qmapcontrol/qmapcontrol.h"
#include <QtGui/QWidget>
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/positionactual.h"

using namespace qmapcontrol;

class MapGadgetWidget : public QWidget
{
    Q_OBJECT

public:
    MapGadgetWidget(QWidget *parent = 0);
   ~MapGadgetWidget();
   void setZoom(int value);
   void setPosition(QPointF pos);
   void setMapProvider(QString provider);

public slots:
    void coordinateClicked(const QMouseEvent* evnt, const QPointF coordinate);	// cmoss
    void geometryClicked(Geometry* geom, QPoint coord_px);	// cmoss

protected:
   void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent* evnt);	// cmoss

private slots:
   void updatePosition();

private:
   void addZoomButtons();

   MapControl *m_mc;
   MapAdapter *m_osmAdapter;
   MapAdapter *m_googleAdapter;
   MapAdapter *m_googleSatAdapter;
   MapAdapter *m_yahooAdapter;
   Layer *m_osmLayer;
   Layer *m_googleLayer;
   Layer *m_googleSatLayer;
   Layer *m_yahooLayer;
   Layer *m_customLayer;
   QTimer *m_updateTimer;
   PositionActual *m_positionActual;

   bool follow_uav; // cmoss
};

#endif /* MAPGADGETWIDGET_H_ */
