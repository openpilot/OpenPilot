#include "opmapwidget.h"
#include <QtGui>
#include <QMetaObject>
namespace mapcontrol
{
    OPMapWidget::OPMapWidget(QWidget *parent):QGraphicsView(parent),useOpenGL(false)
    {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        core=new Core;
        map=new MapGraphicItem(core);
        //text.setZValue(20);
        //QGraphicsTextItem *t=new QGraphicsTextItem(map);
       // t->setPos(10,10);
        mscene.addItem(map);
        map->setZValue(-1);
        //t->setZValue(10);
        this->setScene(&mscene);
        this->adjustSize();
       // t->setFlag(QGraphicsItem::ItemIsMovable,true);
        connect(&mscene,SIGNAL(sceneRectChanged(QRectF)),map,SLOT(resize(QRectF)));
        connect(map,SIGNAL(zoomChanged(double)),this,SIGNAL(zoomChanged(double)));
        QMetaObject metaObject = this->staticMetaObject;
        QMetaEnum metaEnum= metaObject.enumerator( metaObject.indexOfEnumerator("internals::MouseWheelZoomType::Types"));
        QString s=metaEnum.valueToKey(1);
        QString ss=s;
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

}
