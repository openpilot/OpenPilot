#ifndef MODELMAPPROXY_H
#define MODELMAPPROXY_H
#include <QWidget>
#include "opmapcontrol/opmapcontrol.h"
#include "pathaction.h"
#include "waypoint.h"
#include "QMutexLocker"
#include "QPointer"
#include "flightdatamodel.h"
#include <QItemSelectionModel>
#include <widgetdelegates.h>


using namespace mapcontrol;
class modelMapProxy:public QObject
{
    typedef enum {OVERLAY_LINE,OVERLAY_CIRCLE_RIGHT,OVERLAY_CIRCLE_LEFT} overlayType;
    Q_OBJECT
public:
    explicit modelMapProxy(QObject *parent,OPMapWidget * map,flightDataModel * model,QItemSelectionModel * selectionModel);
    WayPointItem *findWayPointNumber(int number);
    void createWayPoint(internals::PointLatLng coord);
    void deleteWayPoint(int number);
    void deleteAll();
private slots:
    void on_dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight );
    void on_rowsInserted ( const QModelIndex & parent, int first, int last );
    void on_rowsRemoved ( const QModelIndex & parent, int first, int last );

    void on_WPDeleted(int wp_numberint, WayPointItem *);
    void on_WPInserted(int,WayPointItem*);
    void on_WPValuesChanged(WayPointItem *wp);
    void on_currentRowChanged(QModelIndex,QModelIndex);
    void on_selectedWPChanged(QList<WayPointItem*>);
private:
    overlayType overlayTranslate(int type);
    void createOverlay(WayPointItem * from,WayPointItem * to,overlayType type,QColor color);
    OPMapWidget * myMap;
    flightDataModel * model;
    void refreshOverlays();
    QItemSelectionModel * selection;
};

#endif // MODELMAPPROXY_H
