#include "opmapwidget.h"
#include <QtGui>
#include <QMetaObject>
namespace mapcontrol
{
//    OPMapWidget::OPMapWidget(QWidget *parent):QGraphicsView(parent),useOpenGL(false)
//    {
//        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
//
//        core=new internals::Core;
//        map=new MapGraphicItem(core);
//        //text.setZValue(20);
//        //QGraphicsTextItem *t=new QGraphicsTextItem(map);
//       // t->setPos(10,10);
//        mscene.addItem(map);
//        map->setZValue(-1);
//        //t->setZValue(10);
//        this->setScene(&mscene);
//        this->adjustSize();
//       // t->setFlag(QGraphicsItem::ItemIsMovable,true);
//        connect(&mscene,SIGNAL(sceneRectChanged(QRectF)),map,SLOT(resize(QRectF)));
//        connect(map,SIGNAL(zoomChanged(double)),this,SIGNAL(zoomChanged(double)));
//    }
    OPMapWidget::OPMapWidget(QWidget *parent, Configuration *config):QGraphicsView(parent),configuration(config),followmouse(true)
    {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        core=new internals::Core;
        map=new MapGraphicItem(core,config);
        mscene.addItem(map);
        map->setZValue(-1);
        this->setScene(&mscene);
        this->adjustSize();
        connect(&mscene,SIGNAL(sceneRectChanged(QRectF)),map,SLOT(resize(QRectF)));
        connect(map,SIGNAL(zoomChanged(double)),this,SIGNAL(zoomChanged(double)));
        connect(map->core,SIGNAL(OnCurrentPositionChanged(internals::PointLatLng)),this,SIGNAL(OnCurrentPositionChanged(internals::PointLatLng)));
        connect(map->core,SIGNAL(OnEmptyTileError(int,core::Point)),this,SIGNAL(OnEmptyTileError(int,core::Point)));
        connect(map->core,SIGNAL(OnMapDrag()),this,SIGNAL(OnMapDrag()));
        connect(map->core,SIGNAL(OnMapTypeChanged(MapType::Types)),this,SIGNAL(OnMapTypeChanged(MapType::Types)));
        connect(map->core,SIGNAL(OnMapZoomChanged()),this,SIGNAL(OnMapZoomChanged()));
        connect(map->core,SIGNAL(OnTileLoadComplete()),this,SIGNAL(OnTileLoadComplete()));
        connect(map->core,SIGNAL(OnTileLoadStart()),this,SIGNAL(OnTileLoadStart()));
        connect(map->core,SIGNAL(OnTilesStillToLoad(int)),this,SIGNAL(OnTilesStillToLoad(int)));
        this->setMouseTracking(followmouse);
    }

    void OPMapWidget::resizeEvent(QResizeEvent *event)
    {
        if (scene())
            scene()->setSceneRect(
                    QRect(QPoint(0, 0), event->size()));
        QGraphicsView::resizeEvent(event);
    }
    QSize OPMapWidget::sizeHint() const
    {
       return map->sizeHint();
    }
    void OPMapWidget::showEvent(QShowEvent *event)
    {
        map->start();
        QGraphicsView::showEvent(event);
    }
    OPMapWidget::~OPMapWidget()
    {
        delete map;
        delete core;
        delete configuration;
    }
    void OPMapWidget::closeEvent(QCloseEvent *event)
    {
        core->OnMapClose();
        event->accept();
    }
    void OPMapWidget::SetUseOpenGL(const bool &value)
    {
        useOpenGL=value;
        if (useOpenGL)
            setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
        else
            setupViewport(new QWidget());
        update();
    }
    internals::PointLatLng OPMapWidget::currentMousePosition()
    {
        return currentmouseposition;
    }
    void OPMapWidget::mouseMoveEvent(QMouseEvent *event)
    {
        QGraphicsView::mouseMoveEvent(event);
        QPointF p=event->posF();
        p=map->mapFromParent(p);
        currentmouseposition=map->FromLocalToLatLng(p.x(),p.y());
    }

}
