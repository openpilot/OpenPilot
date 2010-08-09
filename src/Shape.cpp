/* $Id:$ */

#include "qdisplay/Shape.hpp"

#include <QPainter>
#include <QStyleOption>
#include <QGraphicsScene>

#include <kernel/jafarMacro.hpp>

namespace jafar {
namespace qdisplay {

Shape::Shape(ShapeType shapeType, double x, double y, double w, double h, double angle) :
    m_shapeType(shapeType), m_label(0), m_fontSize(5)
{
	setBoundingBox(x, y, w, h);
	QTransform transform;
	transform.rotate(angle);
	setTransform(transform);
}

void Shape::paint(QPainter *painter, const QStyleOptionGraphicsItem * opt, QWidget * wdg)
{
  painter->setPen(m_color);
  switch(m_shapeType)
  {
	case ShapeRectangleFilled:
		// Setting a brush to fill the rectangle shape with chosen color m_color
		painter->setBrush(QBrush(m_color, Qt::SolidPattern));
    case ShapeRectangle:
      painter->drawRect(m_boundingRect);
      break;
    case ShapeCross:
      painter->drawLine(QPointF(m_boundingRect.left(), 0.), QPointF( m_boundingRect.right(), 0.));
      painter->drawLine(QPointF(0., m_boundingRect.top()), QPointF(0., m_boundingRect.bottom()));
      break;
    case ShapeCrossX:
      painter->drawLine(QPointF(m_boundingRect.left(), m_boundingRect.top()), QPointF( m_boundingRect.right(), m_boundingRect.bottom()));
      painter->drawLine(QPointF(m_boundingRect.right(), m_boundingRect.top()), QPointF(m_boundingRect.left(), m_boundingRect.bottom()));
      break;
	case ShapeEllipseFilled:
		// Setting a brush to fill the ellipse shape with chosen color m_color
		painter->setBrush(QBrush(m_color, Qt::SolidPattern));
    case ShapeEllipse:
      painter->drawEllipse(m_boundingRect);
      break;
  }

	// Setting the painter brush to the default brush Qt:NoBrush
	painter->setBrush(QBrush());

	QGraphicsItemGroup::paint(painter, opt, wdg);
}

void Shape::setLabel(const char * text, double relX, double relY)
{
  if(!m_label)
  {
    m_label = new QGraphicsTextItem(this, scene());
    m_label->setFont(QFont( m_label->font().family(), m_fontSize));
    m_label->setDefaultTextColor( m_fontColor );
    m_label->translate(relX,relY);
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
