#ifndef CORE_H
#define CORE_H

#include "../internals/PointLatlng.h"
#include "mousewheelzoomtype.h"
#include "../core/size.h"
#include "../core/maptype.h"
#include "rectangle.h"
#include "QThreadPool"
#include "tilematrix.h"
#include <QQueue>
#include "loadtask.h"
#include "copyrightstrings.h"
#include "rectlatlng.h"
#include "../internals/projections/lks94projection.h"
#include "../internals/projections/mercatorprojection.h"
#include "../internals/projections/mercatorprojectionyandex.h"
#include "../internals/projections/platecarreeprojection.h"
#include "../internals/projections/platecarreeprojectionpergo.h"
#include "../core/geodecoderstatus.h"
#include "../core/gmaps.h"
#include <QSemaphore>
#include <QThread>
#include <QDateTime>

//#include <QObject>

class Core:public QObject,public QRunnable
{
    Q_OBJECT
public:
    Core();
    void run();
    PointLatLng CurrentPosition(){return currentPosition;};
    void SetCurrentPosition(const PointLatLng &value);

    Point GetcurrentPositionGPixel(){return currentPositionPixel;};
    void SetcurrentPositionGPixel(const Point &value){currentPositionPixel=value;};

    Point GetrenderOffset(){return renderOffset;};
    void SetrenderOffset(const Point &value){renderOffset=value;};

    Point GetcenterTileXYLocation(){return centerTileXYLocation;};
    void SetcenterTileXYLocation(const Point &value){centerTileXYLocation=value;};

    Point GetcenterTileXYLocationLast(){return centerTileXYLocationLast;};
    void SetcenterTileXYLocationLast(const Point &value){centerTileXYLocationLast=value;};

    Point GetdragPoint(){return dragPoint;};
    void SetdragPoint(const Point &value){dragPoint=value;};

    Point GetmouseDown(){return mouseDown;};
    void SetmouseDown(const Point &value){mouseDown=value;};

    Point GetmouseCurrent(){return mouseCurrent;};
    void SetmouseCurrent(const Point &value){mouseCurrent=value;};

    Point GetmouseLastZoom(){return mouseLastZoom;};
    void SetmouseLastZoom(const Point &value){mouseLastZoom=value;};

    MouseWheelZoomType::Types GetMouseWheelZoomType(){return mousewheelzoomtype;};
    void SetMouseWheelZoomType(const MouseWheelZoomType::Types &value){mousewheelzoomtype=value;};

    PointLatLng GetLastLocationInBounds(){return LastLocationInBounds;}
    void SetLastLocationInBounds(const PointLatLng &value){LastLocationInBounds=value;}

    Size GetsizeOfMapArea(){return sizeOfMapArea;}
    void SetsizeOfMapArea(const Size &value){sizeOfMapArea=value;}

    Size GetminOfTiles(){return minOfTiles;}
    void SetminOfTiles(const Size &value){minOfTiles=value;}

    Size GetmaxOfTiles(){return maxOfTiles;}
    void SetmaxOfTiles(const Size &value){maxOfTiles=value;}

    Rectangle GettileRect(){return tileRect;}
    void SettileRect(const Rectangle &value){tileRect=value;}

    Point GettilePoint(){return tilePoint;}
    void SettilePoint(const Point &value){tilePoint=value;}

    Rectangle GetCurrentRegion(){return CurrentRegion;}
    void SetCurrentRegion(const Rectangle &value){CurrentRegion=value;}

    QList<Point> tileDrawingList;

    PureProjection* Projection()
    {
        return projection;
    }
    void SetProjection(PureProjection* value)
    {
        projection=value;
        tileRect=Rectangle(Point(0,0),value->TileSize());
    }
    bool IsDragging(){return isDragging;}

    int Zoom(){return zoom;}

    void SetZoom(int const& value);


    void UpdateBounds();

    MapType::Types GetMapType(){return mapType;}
    void SetMapType(MapType::Types const& value);

    void StartSystem();

    void UpdateCenterTileXYLocation();

    void OnMapSizeChanged(int const& width, int const& height);//TODO had as slot

    void OnMapClose();//TODO had as slot

    GeoCoderStatusCode::Types SetCurrentPositionByKeywords(QString const& keys);

    RectLatLng CurrentViewArea();

    PointLatLng FromLocalToLatLng(int const& x, int const& y);


    Point FromLatLngToLocal(PointLatLng const& latlng);

    int GetMaxZoomToFitRect(RectLatLng const& rect);

    void BeginDrag(Point const& pt);

    void EndDrag();

    void ReloadMap();

    void GoToCurrentPosition();

    bool MouseWheelZooming;

    void DragOffset(Point const& offset);

    void Drag(Point const& pt);

    void CancelAsyncTasks();

    void FindTilesAround(QList<Point> &list);

    void UpdateGroundResolution();
signals:
    void OnCurrentPositionChanged(PointLatLng point);
    void OnTileLoadComplete();
    void OnTileLoadStart();
    void OnMapDrag();
    void OnMapZoomChanged();
    void OnMapTypeChanged(MapType::Types type);
    void OnEmptyTileError(int zoom, Point pos);
    void OnNeedInvalidation();
private:

    PointLatLng currentPosition;
    Point currentPositionPixel;
    Point renderOffset;
    Point centerTileXYLocation;
    Point centerTileXYLocationLast;
    Point dragPoint;
    Point mouseDown;
    Point mouseCurrent;
    Point mouseLastZoom;

    MouseWheelZoomType::Types mousewheelzoomtype;
    PointLatLng LastLocationInBounds;

    Size sizeOfMapArea;
    Size minOfTiles;
    Size maxOfTiles;

    Rectangle tileRect;

    Point tilePoint;

    Rectangle CurrentRegion;

    TileMatrix Matrix;

    QQueue<LoadTask> tileLoadQueue;

    int zoom;

    PureProjection* projection;

    bool isDragging;

    QMutex MtileLoadQueue;

    QMutex Moverlays;

    QMutex MtileDrawingList;

    Size TooltipTextPadding;

    MapType::Types mapType;

    QSemaphore loaderLimit;

    QThreadPool ProcessLoadTaskCallback;

protected:
    bool started;

    int Width;
    int Height;
    int pxRes100m;  // 100 meters
    int pxRes1000m;  // 1km
    int pxRes10km; // 10km
    int pxRes100km; // 100km
    int pxRes1000km; // 1000km
    int pxRes5000km; // 5000km
    void SetCurrentPositionGPixel(Point const& value){currentPositionPixel = value;}
    void GoToCurrentPositionOnZoom();

};

#endif // CORE_H
