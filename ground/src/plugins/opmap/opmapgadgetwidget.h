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
#include <QtGui/QMenu>
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/positionactual.h"

class Ui_OPMap_Widget;

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
    void setUseMemoryCache(bool useMemoryCache);
    void setCacheLocation(QString cacheLocation);

public slots:

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void keyPressEvent(QKeyEvent* event);

private slots:
    void updatePosition();

    void updateMousePos();

    void zoomIn();
    void zoomOut();

    // user control signals
    void on_toolButtonReload_clicked();
    void on_toolButtonFindPlace_clicked();
    void on_toolButtonRR_clicked();
    void on_toolButtonRC_clicked();
    void on_toolButtonRL_clicked();
    void on_toolButtonZoomM_clicked();
    void on_toolButtonZoomP_clicked();
    void on_pushButtonGeoFenceM_clicked();
    void on_pushButtonGeoFenceP_clicked();
    void on_comboBoxZoom_currentIndexChanged(int index);
    void on_toolButtonFlightControlsShowHide_clicked();


    // map signals
    void zoomChanged(double zoom);
    void OnCurrentPositionChanged(internals::PointLatLng point);
    void OnTileLoadComplete();
    void OnTileLoadStart();
    void OnMapDrag();
    void OnMapZoomChanged();
    void OnMapTypeChanged(MapType::Types type);
    void OnEmptyTileError(int zoom, core::Point pos);
    void OnTilesStillToLoad(int number);

    // context menu signals
    void reload();
    void findPlace();
    void goZoomIn();
    void goZoomOut();
    void goMouseClick();
    void goHome();
    void goUAV();
    void followUAV();
    void openWayPointEditor();
    void addWayPoint();
    void deleteWayPoint();
    void clearWayPoints();
    void gridLines();
    void openGL();
    void zoom2() { setZoom(2); }
    void zoom3() { setZoom(3); }
    void zoom4() { setZoom(4); }
    void zoom5() { setZoom(5); }
    void zoom6() { setZoom(6); }
    void zoom7() { setZoom(7); }
    void zoom8() { setZoom(8); }
    void zoom9() { setZoom(9); }
    void zoom10() { setZoom(10); }
    void zoom11() { setZoom(11); }
    void zoom12() { setZoom(12); }
    void zoom13() { setZoom(13); }
    void zoom14() { setZoom(14); }
    void zoom15() { setZoom(15); }
    void zoom16() { setZoom(16); }
    void zoom17() { setZoom(17); }
    void zoom18() { setZoom(18); }
    void zoom19() { setZoom(19); }

private:
    double m_heading;	// uav heading

    internals::PointLatLng mouse_lat_lon;

    QLabel statusLabel;

   QTimer *m_updateTimer;
   QTimer *m_statusUpdateTimer;

   PositionActual *m_positionActual;

    Ui_OPMap_Widget *m_widget;

    mapcontrol::OPMapWidget *m_map;

    QPushButton * createTransparentButton(QWidget *parent, QString text, QString icon);
    void createMapOverlayUserControls();

    void createActions();

    QAction *closeAct;
    QAction *reloadAct;
    QAction *findPlaceAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *goMouseClickAct;
    QAction *goHomeAct;
    QAction *goUAVAct;
    QAction *followUAVAct;
    QAction *wayPointEditorAct;
    QAction *addWayPointAct;
    QAction *deleteWayPointAct;
    QAction *clearWayPointsAct;
    QAction *gridLinesAct;
    QAction *openGLAct;

    QActionGroup *zoomActGroup;
    QAction *zoom2Act;
    QAction *zoom3Act;
    QAction *zoom4Act;
    QAction *zoom5Act;
    QAction *zoom6Act;
    QAction *zoom7Act;
    QAction *zoom8Act;
    QAction *zoom9Act;
    QAction *zoom10Act;
    QAction *zoom11Act;
    QAction *zoom12Act;
    QAction *zoom13Act;
    QAction *zoom14Act;
    QAction *zoom15Act;
    QAction *zoom16Act;
    QAction *zoom17Act;
    QAction *zoom18Act;
    QAction *zoom19Act;
};

#endif /* OPMAP_GADGETWIDGET_H_ */
