#ifndef MAPGRAPHICITEM_H
#define MAPGRAPHICITEM_H

#include <QGraphicsItem>
#include "../internals/core.h"
#include "configuration.h"
#include <QtGui>
#include <QTransform>
#include <QWidget>
#include <QBrush>
#include <QFont>
#include <QObject>
namespace mapcontrol
{
class OPMapWidget;
class MapGraphicItem:public QObject,public QGraphicsItem
{
    friend class mapcontrol::OPMapWidget;
    Q_OBJECT
public:
    MapGraphicItem(internals::Core *core,Configuration *configuration);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget);

    QSize sizeHint()const;


protected:
    void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    void wheelEvent ( QGraphicsSceneWheelEvent * event );
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    bool IsMouseOverMarker()const{return isMouseOverMarker;}
    bool IsDragging()const{return core->IsDragging();}
    int ZoomStep()const;
    void SetZoomStep(int const& value);

private:
    internals::Core *core;
    Configuration *config;
    bool showTileGridLines;
    qreal MapRenderTransform;
    void DrawMap2D(QPainter *painter);
    ///
   // QFont MissingDataFont;
    ///
    int maxZoom;
    int minZoom;
    internals::RectLatLng selectedArea;
    internals::PointLatLng selectionStart;
    internals::PointLatLng selectionEnd;
    double zoomReal;
    QRectF maprect;
    bool isSelected;
    bool isMouseOverMarker;
    void SetIsMouseOverMarker(bool const& value){isMouseOverMarker = value;}
    internals::PointLatLng FromLocalToLatLng(int x, int y);
    qreal rotation;
    QRectF boundingBox(QRectF const& rect, qreal const& angle);
///
//    QBrush EmptytileBrush;
//    QString EmptyTileText;
 //   QPen EmptyTileBorders;
  //  QPen ScalePen;
  //  QPen SelectionPen;
    ///
    int MaxZoom()const{return maxZoom;}
    int MinZoom()const{return minZoom;}
    internals::MouseWheelZoomType::Types GetMouseWheelZoomType(){return core->GetMouseWheelZoomType();}
   // QString GetMouseWheelZoomTypeStr(){return internals::MouseWheelZoomType::TypesStrList().at((int)core->GetMouseWheelZoomType());}
  //  bool MapScaleInfoEnabled;
    Qt::MouseButton DragButton;
    void SetSelectedArea(internals::RectLatLng const& value){selectedArea = value;this->update();}
    internals::RectLatLng BoundsOfMap;
    void Offset(int const& x, int const& y);
    bool CanDragMap()const{return core->CanDragMap;}
    void SetCanDragMap(bool const& value){core->CanDragMap = value;}
    double Zoom();
    void SetZoom(double const& value);
    void mapRotate ( qreal angle );
    void start();
    void  ReloadMap(){core->ReloadMap();}
    GeoCoderStatusCode::Types SetCurrentPositionByKeywords(QString const& keys){return core->SetCurrentPositionByKeywords(keys);}
    MapType::Types GetMapType(){return core->GetMapType();}
    void SetMapType(MapType::Types const& value){core->SetMapType(value);}

private slots:
    void Core_OnNeedInvalidation();
public slots:
    void resize ( QRectF const &rect=QRectF() );
signals:
    void zoomChanged(double zoom);
};
}
#endif // MAPGRAPHICITEM_H
