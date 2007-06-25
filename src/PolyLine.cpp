
#include "qdisplay/PolyLine.hpp"

#include <QGraphicsScene>
#include <QPainter>
#include <QPen>

using namespace jafar::qdisplay;


PolyLine::PolyLine(double scale, QGraphicsItem *parent )
    : QAbstractGraphicsShapeItem(parent), m_scale(scale)
{
    
}

PolyLine::~PolyLine()
{
}

void PolyLine::addPoint(double x, double y)
{
  m_points.append(QPointF(x,y));
  m_boundingRect = QRectF();
  if(scene()) scene()->update( boundingRect() );
}

QRectF PolyLine::boundingRect() const
{
  if (m_boundingRect.isNull()) {
    qreal pw = pen().width(); // Pen width
    m_boundingRect = shape().controlPointRect().adjusted(-pw/2, -pw/2, pw, pw);
  }
  return m_boundingRect;
}

void PolyLine::setColor(int r, int g, int b)
{
  setPen(QPen(QColor(r,g,b)));
}


QPainterPath PolyLine::shape() const
{
  if(m_points.empty())
  {
    return QPainterPath();
  }
    
  QPainterPath path(m_points[0] * m_scale);
  foreach(const QPointF& p, m_points)
  {
    path.lineTo( p * m_scale);
  }
    
    // Convert to a shape from a path
  if (path == QPainterPath())
    return path;
    
  QPainterPathStroker ps;
  ps.setCapStyle(pen().capStyle());
  ps.setWidth(pen().widthF());
  ps.setJoinStyle(pen().joinStyle());
  ps.setMiterLimit(pen().miterLimit());
  QPainterPath p = ps.createStroke(path);
  p.addPath(path);
    
  return p;
}

void PolyLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  Q_UNUSED(widget);
  painter->setPen(pen());
  painter->setBrush(brush());
    
  if(not m_points.empty())
  {
    QPainterPath path(m_points[0] * m_scale);
    foreach(const QPointF& p, m_points)
    {
      path.lineTo( p * m_scale);
    }
    painter->drawPath(path);
  }
}
