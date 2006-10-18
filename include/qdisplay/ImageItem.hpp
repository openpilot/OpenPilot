#ifndef _IMAGE_ITEM_HPP_
#define _IMAGE_ITEM_HPP_

#include <QGraphicsPixmapItem>

#include <image/Image.hpp>

namespace jafar {
namespace qdisplay {
class Viewer;
class ImageItem : public QGraphicsPixmapItem {
  public:
    /**
     * Create an ImageItem to use with a Viewer to display a jafar::image::Image in the Viewer.
     * @param img the jafar image to display
     */
    ImageItem(jafar::qdisplay::Viewer* v, const jafar::image::Image& img);
    /**
     * @param img a jafar image to display
     */
    void setImage(const jafar::image::Image& img);
};

}

}

#endif
