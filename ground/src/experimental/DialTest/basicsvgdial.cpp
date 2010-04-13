#include "basicsvgdial.h"

#include <QGraphicsSvgItem>
#include <QDebug>

BasicSvgDial::BasicSvgDial(QWidget *parent) : QGraphicsView(parent)
{
    m_renderer = Native;
    backgroundFile = "bg.svg";
    needleFile = "need.svg";
    foregroundFile = "fg.svg";
    angle = 0;

    setScene(new QGraphicsScene(this));

}

void BasicSvgDial::setRenderer(RendererType type)
{
    m_renderer = type;
    setViewport(new QWidget);
}

void BasicSvgDial::paintEvent(QPaintEvent *event) {
    qDebug() << "in painEvent()";

    QGraphicsScene *s = scene();
    s->clear();
    m_backgroundItem = new QGraphicsSvgItem(backgroundFile);
    m_backgroundItem->setZValue(-1);

    m_needleItem = new QGraphicsSvgItem(needleFile);
    QRectF rect = m_needleItem->boundingRect();
    m_needleItem->translate(rect.width()/2,rect.height()/2);
    m_needleItem->rotate(angle);
    m_needleItem->translate(-rect.width()/2,-rect.height()/2);
    m_needleItem->setZValue(0);

    m_foregroundItem = new QGraphicsSvgItem(foregroundFile);
    m_foregroundItem->setZValue(1);

    s->addItem(m_backgroundItem);
    s->addItem(m_needleItem);
    s->addItem(m_foregroundItem);

    s->setSceneRect(m_backgroundItem->boundingRect());

    QGraphicsView::paintEvent(event);

    qDebug() << "out paintEvent()";
}






void BasicSvgDial::setAngle(int i) {
    qDebug() << "BasicDial::setAngle()";
    angle = value2angle(i);
    update();
}

void BasicSvgDial::setRange(qreal bottom, qreal top) {
    bottomValue = bottom;
    topValue = top;
}

void BasicSvgDial::setAngles(qreal bottom, qreal top) {
    bottomAngle = bottom;
    topAngle = top;
    angleSpan = topAngle - bottomAngle;
}

qreal BasicSvgDial::value2angle(qreal value) {
    return bottomAngle + (value/topValue)*angleSpan;
}

void BasicSvgDial::setValue(qreal value) {
    angle = value2angle(value);
    currentValue = value;
    update();
}

qreal BasicSvgDial::getValue(void) {
    return currentValue;
}

void BasicSvgDial::setBackgroundFile(QString file) {
    backgroundFile = file;
    update();
}

void BasicSvgDial::setForegroundFile(QString file) {
    foregroundFile = file;
    update();
}

void BasicSvgDial::setNeedleFile(QString file) {
    needleFile = file;
    update();
}
