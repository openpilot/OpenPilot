#include "qdisplay/Line.hpp"

#include <QPainter>
#include <QPen>
#include <QStyleOption>
#include <QGraphicsScene>

#include <kernel/jafarMacro.hpp>

namespace jafar {
namespace qdisplay {

Line::Line(double x1, double y1, double x2, double y2) : QGraphicsLineItem(x1,y1,x2,y2)
{
}
void Line::setColor(int r, int g, int b)
{
  setPen(QPen(QColor(r,g,b)));
}

  
}
}
