#include "basicdial.h"

#include <QPainter>
#include <QDebug>

BasicDial::BasicDial(QWidget *parent) : QWidget(parent)
{
    renderer = new QSvgRenderer();
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
    QRectF frame( QPoint(0, 0), QPoint(side, side));
    renderer->load(QString("bg.svg"));
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
    painter.drawLine(QPoint(0,0), QPoint(0,90));
}

void BasicDial::setAngle(int i) {
    qDebug() << "BasicDial::setAngle()";
    angle = (qreal)i;
    update();
}
