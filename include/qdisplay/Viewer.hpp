#ifndef _QDISPLAY_VIEWER_H_
#define _QDISPLAY_VIEWER_H_

#include <QGraphicsView>
#include <QStatusBar>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMap>

namespace jafar {
namespace qdisplay {

class ImageView;
class Shape;
class Line;
class PolyLine;
class Viewer;

class MouseGraphicsScene: public QGraphicsScene
{
	private:
		Viewer* viewer;
	public:
		MouseGraphicsScene(Viewer* viewer): viewer(viewer) {}
		void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
		void mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent );
};

/**
 * @ingroup qdisplay
 * This is the viewer, you can add ImageView to display on it, or overlay using Shape.
 */
class Viewer : public QGraphicsView {
  Q_OBJECT
  public:
    /**
     * Create a new viewer
     * @param mosaicWidth the width of one cell of the mosaic
     * @param mosaicHeight the height of one cell of the mosaic
     */
    Viewer(int mosaicWidth = 0, int mosaicHeight = 0, QGraphicsScene* scene = 0);
    
    ~Viewer();
    void setBackgroundColor(int R, int G, int B);
    QGraphicsScene* scene() { return m_scene; }
    /**
     * @return the image item of cell (row,col)
     * @param row row number
     * @param col column number
     */
    ImageView* imageItem(int row = 0, int col = 0)
    {
      return m_imageMosaic[row][col];
    }
    /**
     * @param ii an ImageView to display on the scene.
     * @param row the number of the row in the mosaic
     * @param col the number of the column in the mosaic
     */
    void setImageView(ImageView* ii, int row = 0, int col= 0);
    int rows();
    int cols();
    /**
     * @param si a Shape to display on the scene.
     */
    void addShape(qdisplay::Shape* si);
    
    void splitVertical();
    void splitHorizontal();

    /**
     * @param si a Line to display on the scene.
     */
    void addLine(qdisplay::Line* li);

    void addPolyLine(qdisplay::PolyLine* pl);
    
    bool isVisible() { return !QGraphicsView::isHidden(); }
    /** Scale the view
     * @param scaleFactor the scale factor to appply to the view
     */
    void scaleView(qreal scaleFactor);
		/**
		 * Set the window size parameters
		 * @param width width of window
		 * @param height height of window
		 */
		void setWindowSize( int width, int height );
    /**
     * Close the view
     */
    void close();

		void setTitle(const std::string& _title );
		void setTitleWithMouseCoordinates(double x, double y);
		std::string const& getTitle() { return title; }

	/** Set the status message appearing in the status bar
	 *
	 * Shadow text will also be set.
	 *
	 * @param infoString QString(const char*) object. String object to display
	 * @param timeout Milliseconds before the message disappears (same effect as statusBar->clearMessage())
	 */
	void setStatusMessage(QString& infoString, int timeout=0);

	/** Overloading of function setStatusMessage(QString&, int)
	 * @param infoString String to display
	 * @param timeout Milliseconds before the message disappears (same effect as statusBar->clearMessage())
	 */
	void setStatusMessage(const char* infoString, int timeout=0);

    /**
     * Export the view to the given file name.
     */
    void exportView( const std::string& fileName );
  public slots:
		void exportView();
		//void updateSceneRect();
	signals:
		void onKeyPress(QKeyEvent *event);
		void onMouseClick(QGraphicsSceneMouseEvent *mouseEvent, bool isClick);
		void onMouseMove(QGraphicsSceneMouseEvent *mouseEvent);
	protected:
    virtual void contextMenuEvent ( QContextMenuEvent * event );
    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void wheelEvent(QWheelEvent *event);
    virtual void resizeEvent(QResizeEvent  * event);
    virtual void mouseReleaseEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );
  private:
    QGraphicsScene* m_scene;
		friend class MouseGraphicsScene;
    QMap< int, QMap< int, ImageView* > > m_imageMosaic;
    int m_mosaicWidth, m_mosaicHeight;
		int m_windowWidth, m_windowHeight;			// added
    double m_currentZ;
    QAction *m_exportView;
		std::string title;
	/** This cursor is a custom cursor created in the Viewer constructor to
	 * provide a precise cross cursor similar to the crossHair cursor without
	 * double bars
	 */
	QCursor* crossCursor;
	/** Draw the custom cursor crossCursor by build our own QPixmap and
	 * adjusting the perfect hostpot for precise pointing 
	 */
	void createCursor();

	
	/** Status bar to display information  statusBar->showMessage( QString) 
	 *
	 * @see setStatusMessage
	 */
	QStatusBar *statusBar;
	QStatusBar *statusBarShadow;

};

}
}

#endif
