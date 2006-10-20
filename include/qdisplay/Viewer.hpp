#ifndef _IMAGE_VIEWER_H_
#define _IMAGE_VIEWER_H_

#include <QGraphicsView>

#include <QHash>

namespace jafar {
namespace qdisplay {

class ImageItem;
class ShapeItem;

/**
 * This is the viewer, you can add ImageItem to display on it, or overlay using ShapeItem.
 */
class Viewer : public QGraphicsView {

  public:
    /**
     * Create a new viewer
   * @param mosaicWidth the width of one cell of the mosaic
   * @param mosaicHeight the height of one cell of the mosaic
     */
    Viewer(int mosaicWidth = 0, int mosaicHeight = 0);

    QGraphicsScene* scene() { return m_scene; }
    /**
     * @return the image item of cell (row,col)
     * @param row row number
     * @param col column number
     */
    ImageItem* imageItem(int row = 0, int col = 0)
    {
      return m_imageMosaic[row][col];
    }
    /**
     * @param ii an ImageItem to display on the scene.
     * @param row the number of the row in the mosaic
     * @param col the number of the column in the mosaic
     */
    void setImageItem(ImageItem* ii, int row = 0, int col= 0);
    /**
     * @param si a ShapeItem to display on the scene.
     */
    void addShapeItem(ShapeItem* si);
  private:
    QGraphicsScene* m_scene;
    QHash< int, QHash< int, ImageItem* > > m_imageMosaic;
    int m_mosaicWidth, m_mosaicHeight;
    double m_currentZ;
};

}
}

#endif
