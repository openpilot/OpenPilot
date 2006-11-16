#ifndef _IMAGE_ITEM_HPP_
#define _IMAGE_ITEM_HPP_

#include <QObject>
#include <QGraphicsItemGroup>

#include <image/Image.hpp>

class QAction;

namespace jafar {
namespace qdisplay {
class Viewer;
class Shape;
/**
 * This class manipulate the view of an image on a Viewer. You can add specific overlay to this image using
 * the command addShape
 */
class ImageView : public QObject, public QGraphicsItemGroup {
#ifndef SWIG
  Q_OBJECT
#endif
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
#ifndef SWIG
  public slots:
#endif
    void lutRandomize();
  protected:
    void contextMenuEvent ( QGraphicsSceneContextMenuEvent * event );
  private:
    QAction *lutRandomizeAction;
    QImage m_image;
    QGraphicsPixmapItem* m_pixmapItem;
    double m_currentZ;
};

}

}

#endif
