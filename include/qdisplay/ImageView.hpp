#ifndef _IMAGE_ITEM_HPP_
#define _IMAGE_ITEM_HPP_

#include <QObject>
#include <QGraphicsItemGroup>

#include <image/Image.hpp>
#include <string>

class QAction;

namespace jafar {
namespace qdisplay {

  class AbstractEventHandler;
  class Line;
  class Shape;
  class Viewer;
  class PolyLine;
/**
 * @ingroup qdisplay
 * This class manipulate the view of an image on a Viewer. You can add specific overlay to this image using
 * the command addShape
 */
class ImageView : public QObject, public QGraphicsItemGroup {
  Q_OBJECT
  public:
    ImageView();
    /**
     * Create an ImageView to use with a Viewer to display a jafar::image::Image in the Viewer.
     * @param img the jafar image to display
     */
    ImageView(const jafar::image::Image& img);

	/** Create an ImageView from a file
	 * @param filename Path to an image file. its format should be readable by jafar::image module (OpenCV)
	 */
	ImageView(const std::string& filename);

	/** Destructor */
    ~ImageView();

    void connectEvents();
    int imageWidth() const;
    int imageHeight() const;
    /**
     * @param img a jafar image to display
     */
    void setImage(const jafar::image::Image& img);
    /**
     * @param si a Shape to display on the scene.
     */
    void addShape(Shape* si);
    /**
     * @param si the shape to remove from this ImageView
     */
    void removeShape(Shape* si);
    /**
     * @param li a Line to display on the scene.
     */
    void addLine(Line* li);
    /**
     * @param li the Line to remove from this ImageView.
     */
    void removeLine(Line* li);

    void addPolyLine(qdisplay::PolyLine* pl);

    /**
     * Define the event handler for this view
     * @param eh the event handler
     * Note: event handler can be shared between views, so it won't be deleted when
     * this view is deleted
     */
    inline void setEventHandler(AbstractEventHandler* eh) { m_eventHandler = eh; }

	/** Exports the view to different image format */
    void exportView( const std::string& _fileName );

  public slots:
    void lutRandomize();
    void lutGrayscale();
    void lutInvertGrayscale();
    void lutRedHot();
    void exportView();
    void splitVertical();
    void splitHorizontal();
    void addShape();
  protected:
    void contextMenuEvent ( QGraphicsSceneContextMenuEvent * event );
  public:
		QRectF boundingRect () const;
		void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );
		void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
  private:
    QAction *m_lutRandomizeAction, *m_lutGrayscaleAction, *m_lutInvertGrayscaleAction, *m_lutRedHotAction, *m_exportView, * m_splitVerticalAction,*m_splitHorizontalAction,*m_addShapeAction;
    QImage m_image;
    QGraphicsPixmapItem* m_pixmapItem;
    double m_currentZ;
    AbstractEventHandler* m_eventHandler;
    Viewer* m_lastViewer;
};

}

}

#endif
