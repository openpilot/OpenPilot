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
    if( bg.size() == size() ) {
        qDebug() << "BasicDial::renderBackground(): Size not changed! Abort rerendering stuff";
        return;
    }
    /* Create buffer pixmap and make it transparent */
    QPixmap pixmap(size());
    pixmap.fill(Qt::transparent);

    /* Configure painter */
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    renderer->load(backgroundFile);          /* load file; needed to calculate frame         */
    QRectF frame = calculateCenteredFrame(); /* viewport from renderer (based on loaded file */
    renderer->render(&painter, frame);       /* dump bg on pixmap                            */
    bg = pixmap;                             /* save it                                      */
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

QRectF BasicDial::calculateCenteredFrame(void) {
    QRectF vb = renderer->viewBoxF();                                   /* get SVG viewport                      */
    qreal scale = qMax( (vb.height()/height()), (vb.width()/width()));  /* calc scale to fit SVG into widget     */
    vb.setWidth( vb.width()/scale );                                    /* scale viewport so SVG fit widget size */
    vb.setHeight( vb.height()/scale );

    QRectF frame;                             /* let's prepare render frame for bg */
    frame.setX( (width()-vb.width())/2.0 );   /* frame is centered on widget       */
    frame.setY( (height()-vb.height())/2.0 );
    frame.setWidth(  vb.width() );            /* derive size from scaled viewport  */
    frame.setHeight( vb.height() );
    return frame;
}
