#include "basicdial.h"

#include <QPainter>
#include <QDebug>

BasicDial::BasicDial(QWidget *parent) : QWidget(parent)
{
    renderer = new QSvgRenderer();
    backgroundFile = "bg.svg";
    angle = 0;
}

void BasicDial::paintEvent(QPaintEvent *event) {
    qDebug() << "in painEvent()";
    renderBackground();
    int side = qMin(width(), height());
    QPainter painter(this);
    QSize ps = bg.size();
    QSize ws = this->size();
    QSize fs = (ws-ps)/2;
    painter.drawPixmap(QPoint(fs.width(), fs.height()), bg);

    renderNeedle(angle);

    qDebug() << "out paintEvent()";
}

void BasicDial::renderBackground(void) {
    int side = qMin(width(), height());
    if( bg.size() == QSize(side, side) ) {
        qDebug() << "BasicDial::renderBackground(): Size not changed! Abort rerendering stuff";
        return;
    }
    QPixmap pixmap(QSize(side, side));
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF frame( QPoint(0, 0), QPoint(side-1, side-1));
    renderer->load(backgroundFile);
    renderer->render(&painter, frame);
    bg = pixmap;
}

void BasicDial::renderNeedle(qreal angle) {
    int side = qMin(width(), height());
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);
    painter.rotate(angle);
    painter.setPen(pen);
    painter.drawLine(QPoint(0,0), QPoint(0,90));
}

void BasicDial::setAngle(int i) {
    qDebug() << "BasicDial::setAngle()";
    angle = value2angle(i);
    update();
}

void BasicDial::setRange(qreal bottom, qreal top) {
    bottomValue = bottom;
    topValue = top;
}

void BasicDial::setAngles(qreal bottom, qreal top) {
    bottomAngle = bottom;
    topAngle = top;
    angleSpan = topAngle - bottomAngle;
}

qreal BasicDial::value2angle(qreal value) {
    return bottomAngle + (value/topValue)*angleSpan;
}

void BasicDial::setValue(qreal value) {
    angle = value2angle(value);
    currentValue = value;
    update();
}

qreal BasicDial::getValue(void) {
    return currentValue;
}

void BasicDial::setBackgroundFile(QString file) {
    backgroundFile = file;
    bg = QPixmap();
    update();
}

void BasicDial::setPen(QPen p) {
    pen = p;
    pen.setWidth(2);
}
