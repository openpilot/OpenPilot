#include "qdisplay/Viewer.hpp"

#include <QGraphicsScene>

#include <kernel/jafarMacro.hpp>
#include "qdisplay/ImageItem.hpp"

namespace jafar {
namespace qdisplay {

Viewer::Viewer(int mosaicWidth, int mosaicHeight ) : m_scene(new QGraphicsScene()), m_mosaicWidth(mosaicWidth), m_mosaicHeight(mosaicHeight)
{
  show();
  setScene(m_scene);
}

void Viewer::setImageItem(ImageItem* ii, int row, int col)
{
  if(scene()->items().contains(ii)) return;
  scene()->addItem(ii);
  if(m_imageMosaic[row][col])
  {
    scene()->removeItem(m_imageMosaic[row][col]);
    delete m_imageMosaic[row][col];
  }
  m_imageMosaic[row][col] = ii;
  if(m_mosaicWidth == 0 && m_mosaicHeight == 0)
  {
    QRect imageArea = ii->boundingRect().toRect();
    m_mosaicWidth = imageArea.width() + 5;
    m_mosaicHeight = imageArea.height() + 5;
    resize( m_mosaicWidth, m_mosaicHeight );
  }
  ii->setPos( row * m_mosaicWidth, col*m_mosaicHeight);
}

}
}

