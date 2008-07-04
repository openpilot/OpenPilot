#include "qdisplay/Line.hpp"

#include <QPainter>
#include <QPen>
#include <QStyleOption>
#include <QGraphicsScene>

#include <kernel/jafarMacro.hpp>

namespace jafar {
namespace qdisplay {

Line::Line(double x1, double y1, double x2, double y2) : QGraphicsLineItem(x1,y1,x2,y2), m_label(0), m_fontSize(5)
{
}
void Line::setColor(int r, int g, int b)
{
  setPen(QPen(QColor(r,g,b)));
}

void Line::setLabel(char * text)
{
  if(!m_label)
  {
    m_label = new QGraphicsTextItem(this, scene());
    m_label->setFont(QFont( m_label->font().family(), m_fontSize * 10));
//     addToGroup(m_label);
    m_label->scale( 0.1, 0.1);
    m_label->setPos( 0.5*(line().p1() + line().p2()));
  }
  m_label->setPlainText(text);
}

  
}
}
