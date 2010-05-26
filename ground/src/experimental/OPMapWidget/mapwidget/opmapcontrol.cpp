#include "opmapcontrol.h"

OPMapControl::OPMapControl(QWidget *parent):QWidget(parent)
{

}
void OPMapControl::paintEvent(QPaintEvent* evnt)
{
    QWidget::paintEvent(evnt);
    QPainter painter(this);
    painter.drawText(10,10,"TESTE");
}

void OPMapControl::mousePressEvent ( QMouseEvent* evnt )
{

}

void OPMapControl::mouseReleaseEvent ( QMouseEvent* evnt )
{

}

void OPMapControl::mouseMoveEvent ( QMouseEvent* evnt )
{

}
