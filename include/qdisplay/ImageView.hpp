#ifndef _IMAGE_ITEM_HPP_
#define _IMAGE_ITEM_HPP_

#include <QGraphicsItemGroup>

#include <image/Image.hpp>

namespace jafar {
namespace qdisplay {
class Viewer;
class Shape;
/**
 * This class manipulate the view of an image on a Viewer. You can add specific overlay to this image using
 * the command addShape
 */
class ImageView : public QGraphicsItemGroup {
  public:
    /**
     * Create an ImageView to use with a Viewer to display a jafar::image::Image in the Viewer.
     * @param img the jafar image to display
     */
    ImageView(const jafar::image::Image& img);
    /**
     * @param img a jafar image to display
     */
    void setImage(const jafar::image::Image& img);
    /**
     * @param si a Shape to display on the scene.
     */
    void addShape(Shape* si);
  private:
    QGraphicsPixmapItem* m_pixmapItem;
    double m_currentZ;
};

}

}

#endif
