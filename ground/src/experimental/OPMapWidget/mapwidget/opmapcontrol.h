#ifndef OPMAPCONTROL_H
#define OPMAPCONTROL_H

#include "../internals/core.h"
#include <QtGui>
#include <QTransform>
#include <QWidget>
#include <QBrush>
#include <QFont>

namespace mapcontrol
{

using namespace core;
//using namespace internals;

class internals::Core;

class OPMapControl:public QWidget
{

    Q_OBJECT

//    Q_PROPERTY(int MaxZoom READ MaxZoom WRITE SetMaxZoom)
//    Q_PROPERTY(int MinZoom READ MinZoom WRITE SetMinZoom)
//    Q_PROPERTY(internals::MouseWheelZoomType::Types MouseWheelZoom READ GetMouseWheelZoomType WRITE SetMouseWheelZoomType)
//    Q_PROPERTY(QString MouseWheelZoomStr READ GetMouseWheelZoomTypeStr WRITE SetMouseWheelZoomTypeByStr)
//    Q_PROPERTY(bool ShowTileGridLines READ ShowTileGridLines WRITE SetShowTileGridLines)
//    Q_PROPERTY(double Zoom READ Zoom WRITE SetZoom)
public:
    OPMapControl(QWidget *parent=0);

protected:
    void paintEvent ( QPaintEvent* evnt );
    void mousePressEvent ( QMouseEvent* evnt );
    void mouseReleaseEvent ( QMouseEvent* evnt );
    void mouseMoveEvent ( QMouseEvent* evnt );
    void resizeEvent ( QResizeEvent * event );
    void showEvent ( QShowEvent * event );
    void closeEvent ( QCloseEvent * event );
    bool IsDragging()const{return core.IsDragging();}
    bool IsMouseOverMarker()const{return isMouseOverMarker;}
    void wheelEvent ( QWheelEvent * event );
    int ZoomStep()const;
    void SetZoomStep(int const& value);
private:
    QBrush EmptytileBrush;
    QString EmptyTileText;
    QPen EmptyTileBorders;
    QPen ScalePen;
    QPen SelectionPen;
//    bool ShowTileGridLines()const {return showTileGridLines;}
//    void SetShowTileGridLines(bool const& value){showTileGridLines=value;this->repaint();}
    int MaxZoom()const{return maxZoom;}
//    void SetMaxZoom(int const& value){maxZoom = value;}
    int MinZoom()const{return minZoom;}
//    void SetMinZoom(int const& value){minZoom = value;}
    MouseWheelZoomType::Types GetMouseWheelZoomType(){return core.GetMouseWheelZoomType();}
//    void SetMouseWheelZoomType(MouseWheelZoomType::Types const& value){core.SetMouseWheelZoomType(value);}
//    void SetMouseWheelZoomTypeByStr(const QString &value){core.SetMouseWheelZoomType(MouseWheelZoomType::TypeByStr(value));}
    QString GetMouseWheelZoomTypeStr(){return MouseWheelZoomType::TypesStrList().at((int)core.GetMouseWheelZoomType());}
    bool MapScaleInfoEnabled;
    Qt::MouseButton DragButton;
//    RectLatLng SelectedArea()const{return selectedArea;}
    void SetSelectedArea(RectLatLng const& value){selectedArea = value;this->update();}
     RectLatLng BoundsOfMap;
    void Offset(int const& x, int const& y);
    bool CanDragMap()const{return core.CanDragMap;}
    void SetCanDragMap(bool const& value){core.CanDragMap = value;}
//    PointLatLng CurrentPosition()const{return core.CurrentPosition();}
//    void SetCurrentPosition(PointLatLng const& value){core.SetCurrentPosition(value);}
    double Zoom();
    void SetZoom(double const& value);
    bool showTileGridLines;
    Core core;
    qreal MapRenderTransform;
    void DrawMap2D(QPainter &painter);
    QFont MissingDataFont;
    void resize();
    int maxZoom;
    int minZoom;
    RectLatLng selectedArea;
    PointLatLng selectionStart;
    PointLatLng selectionEnd;
    double zoomReal;
    bool isSelected;
    bool isMouseOverMarker;
    void SetIsMouseOverMarker(bool const& value){isMouseOverMarker = value;}
    PointLatLng FromLocalToLatLng(int x, int y);
private slots:
    void Core_OnNeedInvalidation();
};
}
#endif // OPMAPCONTROL_H
