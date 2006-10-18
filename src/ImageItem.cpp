#include "qdisplay/ImageItem.hpp"

#include <QGraphicsScene>

#include "qdisplay/Viewer.hpp"

namespace jafar {
namespace qdisplay {

ImageItem::ImageItem(jafar::qdisplay::Viewer* v, const jafar::image::Image& img)
{
  setImage(img);
  v->scene()->addItem(this);
  QRect rect = boundingRect().toRect();
  v->resize( rect.width() + 5, rect.height() + 5 );
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
  setPixmap(QPixmap::fromImage(img));
}

}
}

