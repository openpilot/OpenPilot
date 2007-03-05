#include "qdisplay/Line.hpp"

#include <QPainter>
#include <QPen>
#include <QStyleOption>
#include <QGraphicsScene>

#include <kernel/jafarMacro.hpp>

namespace jafar {
namespace qdisplay {

Line::Line(double x, double y, double w, double h) : QGraphicsLineItem(x,y,w,h)
{
}
void Line::setColor(int r, int g, int b)
{
  setPen(QPen(QColor(r,g,b)));
}

  
}
}
