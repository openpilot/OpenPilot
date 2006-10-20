#include "qdisplay/ShapeItem.hpp"

#include <QPainter>
#include <QStyleOption>

#include <kernel/jafarMacro.hpp>

namespace jafar {
namespace qdisplay {

ShapeItem::ShapeItem(ShapeType shapeType, double x, double y, double w, double h) :
    m_shapeType(shapeType), m_boundingRect(-w * 0.5, -h * 0.5, w, h)
{
  setPos(x,y);
}

void ShapeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  painter->setPen(m_color);
  switch(m_shapeType)
  {
    case ShapeRectangle:
      painter->drawRect(m_boundingRect);
      break;
    case ShapeCross:
      painter->drawLine(m_boundingRect.left(), 0., m_boundingRect.right(), 0.);
      painter->drawLine(0., m_boundingRect.top(), 0., m_boundingRect.bottom());
      break;
  }
}

  
}
}
