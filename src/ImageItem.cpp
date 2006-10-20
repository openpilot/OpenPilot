#include "qdisplay/ImageItem.hpp"

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>

#include "qdisplay/ShapeItem.hpp"

namespace jafar {
namespace qdisplay {

ImageItem::ImageItem(const jafar::image::Image& img) :
    m_pixmapItem(new QGraphicsPixmapItem()),
    m_currentZ(0.)
{
  setImage(img);
  m_pixmapItem->setZValue(m_currentZ++);
  addToGroup(m_pixmapItem);
}

void ImageItem::addShapeItem(ShapeItem* si)
{
  addToGroup(si);
  scene()->addItem(si);
  si->setZValue(m_currentZ++);
}

void ImageItem::setImage(const jafar::image::Image& jfrimg)
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
        const uchar* v = jfrimg.data( y) + x;
        c.setRgb(v[0], v[1], v[2]);
        img.setPixel(x, y, c.rgb());
      }
    }
  }
  m_pixmapItem->setPixmap(QPixmap::fromImage(img));
}

}
}
