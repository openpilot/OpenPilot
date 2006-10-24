#include "qdisplay/Shape.hpp"

#include <QPainter>
#include <QStyleOption>
#include <QGraphicsScene>

#include <kernel/jafarMacro.hpp>

namespace jafar {
namespace qdisplay {

Shape::Shape(ShapeType shapeType, double x, double y, double w, double h) :
    m_shapeType(shapeType), m_boundingRect(-w * 0.5, -h * 0.5, w, h), m_label(0)
{
  setPos(x,y);
}

void Shape::paint(QPainter *painter, const QStyleOptionGraphicsItem * opt, QWidget * wdg)
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
  QGraphicsItemGroup::paint(painter, opt, wdg);
}
void Shape::setLabel(char * text)
{
  if(!m_label)
  {
    m_label = new QGraphicsTextItem(this, scene());
    addToGroup(m_label);
    if(scene()) scene()->addItem(m_label);
  }
  m_label->setPlainText(text);
}
  
}
}
