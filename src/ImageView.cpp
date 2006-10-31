#include "qdisplay/ImageView.hpp"

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>

#include <kernel/jafarMacro.hpp>
#include "qdisplay/Shape.hpp"

namespace jafar {
namespace qdisplay {

ImageView::ImageView(const jafar::image::Image& img) :
    m_pixmapItem(new QGraphicsPixmapItem()),
    m_currentZ(0.)
{
  setImage(img);
  m_pixmapItem->setZValue(m_currentZ++);
  addToGroup(m_pixmapItem);
}

void ImageView::addShape(Shape* si)
{
  JFR_PRED_RUN_TIME(scene(), "You first need to add the ImageView to a scene");
  addToGroup(si);
  scene()->addItem(si);
  si->setZValue(m_currentZ++);
  si->moveBy(pos().x(), pos().y());
}

void ImageView::setImage(const jafar::image::Image& jfrimg)
{
  int width = jfrimg.width();
  int height = jfrimg.height();
  QImage img(width, height, QImage::Format_RGB32);
  QColor c;
  if(jfrimg.channels() == 1)
  {
    for(int x = 0; x < width; x++)
    {
      for(int y = 0; y < height; y++)
      {
        const uchar* v = jfrimg.data( y) + x;
        c.setRgb(*v, *v, *v);
        img.setPixel(x, y, c.rgb());
      }
    }
  } else if(jfrimg.channels() == 3) {
    for(int x = 0; x < width; x++)
    {
      for(int y = 0; y < height; y++)
      {
        const uchar* v = jfrimg.data( y) + 3*x;
        c.setRgb(v[2], v[1], v[0]);
        img.setPixel(x, y, c.rgb());
      }
    }
  }
  m_pixmapItem->setPixmap(QPixmap::fromImage(img));
}

}
}
