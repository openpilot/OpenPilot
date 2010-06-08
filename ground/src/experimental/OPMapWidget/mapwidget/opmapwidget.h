#ifndef OPMAPWIDGET_H
#define OPMAPWIDGET_H

#include "../mapwidget/mapgraphicitem.h"
#include <QObject>
namespace mapcontrol
{
    class OPMapWidget:public QGraphicsView
    {
        Q_OBJECT

        Q_PROPERTY(int MaxZoom READ MaxZoom WRITE SetMaxZoom)
        Q_PROPERTY(int MinZoom READ MinZoom WRITE SetMinZoom)
        Q_PROPERTY(internals::MouseWheelZoomType::Types MouseWheelZoom READ GetMouseWheelZoomType WRITE SetMouseWheelZoomType)
        Q_PROPERTY(QString MouseWheelZoomStr READ GetMouseWheelZoomTypeStr WRITE SetMouseWheelZoomTypeByStr)
        Q_PROPERTY(bool ShowTileGridLines READ ShowTileGridLines WRITE SetShowTileGridLines)
        Q_PROPERTY(double Zoom READ Zoom WRITE SetZoom)
        Q_PROPERTY(qreal Rotate READ Rotate WRITE SetRotate)
    public:
        OPMapWidget(QWidget *parent=0);
        bool ShowTileGridLines()const {return map->showTileGridLines;}
        void SetShowTileGridLines(bool const& value){map->showTileGridLines=value;map->update();}
        int MaxZoom()const{return map->maxZoom;}
        void SetMaxZoom(int const& value){map->maxZoom = value;}
        int MinZoom()const{return map->minZoom;}
        void SetMinZoom(int const& value){map->minZoom = value;}
        MouseWheelZoomType::Types GetMouseWheelZoomType(){return  map->core->GetMouseWheelZoomType();}
        void SetMouseWheelZoomType(MouseWheelZoomType::Types const& value){map->core->SetMouseWheelZoomType(value);}
        void SetMouseWheelZoomTypeByStr(const QString &value){map->core->SetMouseWheelZoomType(MouseWheelZoomType::TypeByStr(value));}
        QString GetMouseWheelZoomTypeStr(){return map->GetMouseWheelZoomTypeStr();}
        RectLatLng SelectedArea()const{return  map->selectedArea;}
        void SetSelectedArea(RectLatLng const& value){ map->selectedArea = value;this->update();}
        bool CanDragMap()const{return map->CanDragMap();}
        void SetCanDragMap(bool const& value){map->SetCanDragMap(value);}
        PointLatLng CurrentPosition()const{return map->core->CurrentPosition();}
        void SetCurrentPosition(PointLatLng const& value){map->core->SetCurrentPosition(value);}
        double Zoom(){map->Zoom();}
        void SetZoom(double const& value){map->SetZoom(value);}
        qreal Rotate(){return map->rotation;}
        void SetRotate(qreal const& value){return map->mapRotate(value);}
    private:
        internals::Core *core;
        MapGraphicItem *map;
        QGraphicsScene mscene;
    protected:
           void resizeEvent(QResizeEvent *event);
   //    private slots:

    };
}
#endif // OPMAPWIDGET_H
