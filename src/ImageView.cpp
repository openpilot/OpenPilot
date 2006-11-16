#include "qdisplay/ImageView.hpp"

#include <QAction>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

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
  lutRandomizeAction = new QAction("Randomize the lut", (QObject*)this);
  this->connect(lutRandomizeAction, SIGNAL(triggered()), this, SLOT(lutRandomize()));

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
  QColor c;
  if(jfrimg.channels() == 1)
  {
    m_image = QImage(width, height, QImage::Format_Indexed8);
    QVector<QRgb> colorTable;
    for(int i = 0; i < 255; i++)
    {
      colorTable.push_back( qRgb(i,i,i));
    }
    m_image.setColorTable( colorTable );
    for(int x = 0; x < width; x++)
    {
      for(int y = 0; y < height; y++)
      {
        const uchar* v = jfrimg.data( y) + x;
        m_image.setPixel(x, y, *v);
      }
    }
  } else if(jfrimg.channels() == 3) {
    m_image = QImage(width, height, QImage::Format_RGB32);
    for(int x = 0; x < width; x++)
    {
      for(int y = 0; y < height; y++)
      {
        const uchar* v = jfrimg.data( y) + 3*x;
        c.setRgb(v[2], v[1], v[0]);
        m_image.setPixel(x, y, c.rgb());
      }
    }
  }
  m_pixmapItem->setPixmap(QPixmap::fromImage(m_image));
}

void ImageView::lutRandomize()
{
  
  if(m_image.format() == QImage::Format_Indexed8)
  {
    QVector<QRgb> colorTable;
    for(int i = 0; i < 255; i++)
    {
      colorTable.push_back( qRgb(255 * rand(),255 * rand(),255 * rand()));
    }
    m_image.setColorTable( colorTable );
    m_pixmapItem->setPixmap(QPixmap::fromImage(m_image));
  }
}

void ImageView::contextMenuEvent ( QGraphicsSceneContextMenuEvent * event )
{
  QMenu menu;
  if(m_image.format() == QImage::Format_Indexed8)
  {
    menu.addAction(lutRandomizeAction);
  }
  menu.exec(event->screenPos());
}

}

}

#include "ImageView.moc"
