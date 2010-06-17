#ifndef OPMAPWIDGET_H
#define OPMAPWIDGET_H

#include "../mapwidget/mapgraphicitem.h"
#include "../core/geodecoderstatus.h"
#include "../core/maptype.h"
#include "../core/languagetype.h"
#include "configuration.h"
#include <QObject>
#include <QtOpenGL/QGLWidget>
namespace mapcontrol
{
    class Helper
    {
    public:
        static MapType::Types MapTypeFromString(QString const& value){return MapType::TypeByStr(value);}
        static QString StrFromMapType(MapType::Types const& value){return MapType::StrByType(value);}
        static QStringList MapTypes(){return MapType::TypesList();}

        static GeoCoderStatusCode::Types GeoCoderStatusCodeFromString(QString const& value){return GeoCoderStatusCode::TypeByStr(value);}
        static QString StrFromGeoCoderStatusCode(GeoCoderStatusCode::Types const& value){return GeoCoderStatusCode::StrByType(value);}
        static QStringList GeoCoderTypes(){return GeoCoderStatusCode::TypesList();}


        static internals::MouseWheelZoomType::Types MouseWheelZoomTypeFromString(QString const& value){return internals::MouseWheelZoomType::TypeByStr(value);}
        static QString StrFromMouseWheelZoomType(internals::MouseWheelZoomType::Types const& value){return internals::MouseWheelZoomType::StrByType(value);}
        static QStringList MouseWheelZoomTypes(){return internals::MouseWheelZoomType::TypesList();}

        static core::LanguageType::Types LanguageTypeFromString(QString const& value){return core::LanguageType::TypeByStr(value);}
        static QString StrFromLanguageType(core::LanguageType::Types const& value){return core::LanguageType::StrByType(value);}
        static QStringList LanguageTypes(){return core::LanguageType::TypesList();}

        static core::AccessMode::Types AccessModeFromString(QString const& value){return core::AccessMode::TypeByStr(value);}
        static QString StrFromAccessMode(core::AccessMode::Types const& value){return core::AccessMode::StrByType(value);}
        static QStringList AccessModeTypes(){return core::AccessMode::TypesList();}
    };

    class OPMapWidget:public QGraphicsView
    {
        Q_OBJECT

        Q_PROPERTY(int MaxZoom READ MaxZoom WRITE SetMaxZoom)
        Q_PROPERTY(int MinZoom READ MinZoom WRITE SetMinZoom)
     //   Q_PROPERTY(internals::MouseWheelZoomType::Types MouseWheelZoom READ GetMouseWheelZoomType WRITE SetMouseWheelZoomType)
     //   Q_PROPERTY(QString MouseWheelZoomStr READ GetMouseWheelZoomTypeStr WRITE SetMouseWheelZoomTypeByStr)
        Q_PROPERTY(bool ShowTileGridLines READ ShowTileGridLines WRITE SetShowTileGridLines)
        Q_PROPERTY(double Zoom READ Zoom WRITE SetZoom)
        Q_PROPERTY(qreal Rotate READ Rotate WRITE SetRotate)
        Q_ENUMS(internals::MouseWheelZoomType::Types)
        Q_ENUMS(internals::GeoCoderStatusCode::Types)

    public:
        QSize sizeHint() const;
        OPMapWidget(QWidget *parent=0,Configuration *config=new Configuration);
        ~OPMapWidget();

        bool ShowTileGridLines()const {return map->showTileGridLines;}
        void SetShowTileGridLines(bool const& value){map->showTileGridLines=value;map->update();}

        int MaxZoom()const{return map->maxZoom;}
        void SetMaxZoom(int const& value){map->maxZoom = value;}

        int MinZoom()const{return map->minZoom;}
        void SetMinZoom(int const& value){map->minZoom = value;}

        internals::MouseWheelZoomType::Types GetMouseWheelZoomType(){return  map->core->GetMouseWheelZoomType();}
        void SetMouseWheelZoomType(internals::MouseWheelZoomType::Types const& value){map->core->SetMouseWheelZoomType(value);}
      //  void SetMouseWheelZoomTypeByStr(const QString &value){map->core->SetMouseWheelZoomType(internals::MouseWheelZoomType::TypeByStr(value));}
      //  QString GetMouseWheelZoomTypeStr(){return map->GetMouseWheelZoomTypeStr();}

        internals::RectLatLng SelectedArea()const{return  map->selectedArea;}
        void SetSelectedArea(internals::RectLatLng const& value){ map->selectedArea = value;this->update();}

        bool CanDragMap()const{return map->CanDragMap();}
        void SetCanDragMap(bool const& value){map->SetCanDragMap(value);}

        internals::PointLatLng CurrentPosition()const{return map->core->CurrentPosition();}
        void SetCurrentPosition(internals::PointLatLng const& value){map->core->SetCurrentPosition(value);}

        double Zoom(){return map->Zoom();}
        void SetZoom(double const& value){map->SetZoom(value);}

        qreal Rotate(){return map->rotation;}
        void SetRotate(qreal const& value){map->mapRotate(value);}

        void ReloadMap(){map->ReloadMap(); map->resize();}

        GeoCoderStatusCode::Types SetCurrentPositionByKeywords(QString const& keys){return map->SetCurrentPositionByKeywords(keys);}

        bool UseOpenGL(){return useOpenGL;}
        void SetUseOpenGL(bool const& value);

        MapType::Types GetMapType(){return map->core->GetMapType();}
        void SetMapType(MapType::Types const& value){map->core->SetMapType(value);}

        bool isStarted(){return map->core->isStarted();}

        Configuration* configuration;

    private:
        internals::Core *core;
        MapGraphicItem *map;
        QGraphicsScene mscene;
        bool useOpenGL;
        GeoCoderStatusCode x;
        MapType y;
        core::AccessMode xx;


    protected:
        void resizeEvent(QResizeEvent *event);
        void showEvent ( QShowEvent * event );
        void closeEvent(QCloseEvent *event);
        //    private slots:
     signals:
        void zoomChanged(double zoom);
    signals:
        void OnCurrentPositionChanged(internals::PointLatLng point);
        void OnTileLoadComplete();
        void OnTileLoadStart();
        void OnMapDrag();
        void OnMapZoomChanged();
        void OnMapTypeChanged(MapType::Types type);
        void OnEmptyTileError(int zoom, core::Point pos);
        void OnTilesStillToLoad(int number);

    };
}
#endif // OPMAPWIDGET_H
