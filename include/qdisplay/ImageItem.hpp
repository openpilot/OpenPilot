#ifndef _IMAGE_ITEM_HPP_
#define _IMAGE_ITEM_HPP_

#include <QGraphicsItemGroup>

#include <image/Image.hpp>

namespace jafar {
namespace qdisplay {
class Viewer;
class ShapeItem;
/**
 * This class manipulate the view of an image on a Viewer. You can add specific overlay to this image using
 * the command addShapeItem
 */
class ImageItem : public QGraphicsItemGroup {
  public:
    /**
     * Create an ImageItem to use with a Viewer to display a jafar::image::Image in the Viewer.
     * @param img the jafar image to display
     */
    ImageItem(const jafar::image::Image& img);
    /**
     * @param img a jafar image to display
     */
    void setImage(const jafar::image::Image& img);
    /**
     * @param si a ShapeItem to display on the scene.
     */
    void addShapeItem(ShapeItem* si);
  private:
    QGraphicsPixmapItem* m_pixmapItem;
    double m_currentZ;
};

}

}

#endif
