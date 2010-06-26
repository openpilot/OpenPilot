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
#include <QStringList>
#include <QStandardItemModel>

#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/positionactual.h"

#include "opmap_waypointeditor_dialog.h"

namespace Ui {
    class OPMap_Widget;
}

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
   void setAccessMode(QString accessMode);
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
    void on_comboBoxFindPlace_returnPressed();
    void on_toolButtonFindPlace_clicked();
    void on_toolButtonZoomM_clicked();
    void on_toolButtonZoomP_clicked();
    void on_toolButtonWaypointsTreeViewShowHide_clicked();
    void on_toolButtonFlightControlsShowHide_clicked();
    void on_toolButtonMapHome_clicked();
    void on_toolButtonMapUAV_clicked();
    void on_horizontalSliderZoom_sliderMoved(int position);
    void on_toolButtonWaypointEditor_clicked();

    void on_treeViewWaypoints_clicked(QModelIndex index);

    void on_toolButtonHome_clicked();
    void on_toolButtonHoldPosition_clicked();
    void on_toolButtonGo_clicked();

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
    void on_followUAVpositionAct_toggled(bool checked);
    void on_followUAVheadingAct_toggled(bool checked);
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

    int prev_tile_number;

    QStringList findPlaceWordList;
    QCompleter *findPlaceCompleter;

   QTimer *m_updateTimer;
   QTimer *m_statusUpdateTimer;

   PositionActual *m_positionActual;

    Ui::OPMap_Widget *m_widget;

    mapcontrol::OPMapWidget *m_map;

    opmap_waypointeditor_dialog *waypoint_editor;

    QPushButton * createTransparentButton(QWidget *parent, QString text, QString icon);
    void createMapOverlayUserControls();


    QStandardItemModel *wayPoint_treeView_model;

    void createActions();

    QAction *closeAct;
    QAction *reloadAct;
    QAction *findPlaceAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *goMouseClickAct;
    QAction *goHomeAct;
    QAction *goUAVAct;
    QAction *followUAVpositionAct;
    QAction *followUAVheadingAct;
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
