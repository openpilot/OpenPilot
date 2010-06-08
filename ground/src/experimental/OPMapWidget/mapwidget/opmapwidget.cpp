#include "opmapwidget.h"
#include <QtGui>
namespace mapcontrol
{
    OPMapWidget::OPMapWidget(QWidget *parent):QGraphicsView(parent)
    {
        core=new internals::Core;
        map=new MapGraphicItem(core);
        //text.setZValue(20);
        QGraphicsTextItem *t=new QGraphicsTextItem(map);
        t->setPos(10,10);
        mscene.addItem(map);
        map->setZValue(-1);
        t->setZValue(10);
        this->setScene(&mscene);
        this->adjustSize();
        t->setFlag(QGraphicsItem::ItemIsMovable,true);
        connect(&mscene,SIGNAL(sceneRectChanged(QRectF)),map,SLOT(resize(QRectF)));
    }
    void OPMapWidget::resizeEvent(QResizeEvent *event)
    {
        if (scene())
            scene()->setSceneRect(
                    QRect(QPoint(0, 0), event->size()));
        QGraphicsView::resizeEvent(event);
    }

}
