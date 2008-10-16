/* $Id:$ */

#include "qdisplay/Shape.hpp"

#include <QPainter>
#include <QStyleOption>
#include <QGraphicsScene>

#include <kernel/jafarMacro.hpp>

namespace jafar {
namespace qdisplay {

Shape::Shape(ShapeType shapeType, double x, double y, double w, double h) :
    m_shapeType(shapeType), m_label(0), m_fontSize(5)
{
  setBoundingBox(x, y, w, h );
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
      painter->drawLine(QPointF(m_boundingRect.left(), 0.), QPointF( m_boundingRect.right(), 0.));
      painter->drawLine(QPointF(0., m_boundingRect.top()), QPointF(0., m_boundingRect.bottom()));
      break;
    case ShapeEllipse:
    	painter->drawEllipse(m_boundingRect);
    	break;
  }
  QGraphicsItemGroup::paint(painter, opt, wdg);
}
void Shape::setLabel(char * text)
{
  if(!m_label)
  {
    m_label = new QGraphicsTextItem(this, scene());
    m_label->setFont(QFont( m_label->font().family(), m_fontSize));
    m_label->setDefaultTextColor( m_fontColor );
    addToGroup(m_label);
  }
  m_label->setPlainText(text);
}
  
void Shape::setBoundingBox( double x, double y, double w, double h)
{
  m_boundingRect = QRectF( -w * 0.5, -h * 0.5, w, h );
  setPos(x,y);
}
  
}
}
