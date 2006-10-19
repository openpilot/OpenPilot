#ifndef _IMAGE_VIEWER_H_
#define _IMAGE_VIEWER_H_

#include <QGraphicsView>

#include <QMap>

namespace jafar {
namespace qdisplay {

class ImageItem;

class Viewer : public QGraphicsView {

  public:
    Viewer(int mosaicWidth = 0, int mosaicHeight = 0);

    QGraphicsScene* scene() { return m_scene; }
    void setImageItem(ImageItem* ii, int row = 0, int col= 0);
  private:
    QGraphicsScene* m_scene;
    QMap< int, QMap< int, ImageItem* > > m_imageMosaic;
    int m_mosaicWidth, m_mosaicHeight;
};

}
}

#endif
