#include "qdisplay/Viewer.hpp"

#include <QGraphicsScene>
#include <QSplitter>
#include <QWheelEvent>
#include <QSvgGenerator>
#include <QMenu>

#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QCursor>
#include <QString>
#include <QLabel>


// #include <QtOpenGL/QGLWidget>

#include <kernel/jafarMacro.hpp>

#include "qdisplay/ImageView.hpp"
#include "qdisplay/Line.hpp"
#include "qdisplay/PolyLine.hpp"
#include "qdisplay/Shape.hpp"
#include "qdisplay/ViewerManager.hpp"

#include <QSplitter>

using namespace jafar::qdisplay;


Viewer::Viewer(int mosaicWidth, int mosaicHeight, QGraphicsScene* scene ) : m_scene(scene), m_mosaicWidth(mosaicWidth), m_mosaicHeight(mosaicHeight), m_currentZ(0.)
{
	m_windowWidth = -1;	// norman
	m_windowHeight = -1;	// norman
	
	//--- Setting up the scene
	if(not m_scene) {
		m_scene = new MouseGraphicsScene(this);
	}
	m_scene->setBackgroundBrush( Qt::white );
	setDragMode(QGraphicsView::ScrollHandDrag);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse); 

	// Creating a custom cursor for precise pointing
	createCursor();
	// The cursor type is defined by the viewport displaying the scene.
	// So to have an effective change of the cursor type we must set the cursor on the viewport itself.
	viewport()->unsetCursor();
	viewport()->setCursor(*crossCursor);

	// added by norman
	if ((m_windowHeight > 0) and (m_windowWidth > 0))
	{
		setGeometry(0,0,m_windowWidth,m_windowHeight);
	}
	setScene(m_scene);

	// Creating a Status bar for the viewer. It is a widget with slot showMessage(QString&).
	statusBar = new QStatusBar(this);
	// Setting position and initial geometry. Status bar is on top to avoid repositionning issues.
	statusBar->setGeometry(3, 2, width(), 15);
	// Make the geometry dynamic so the bottrom right corner sticks to the parent right border
	statusBar->setSizeGripEnabled(true);
	// Status bar style as Cascading Style Sheet
	statusBar->setStyleSheet(QString("background-color: transparent; color: #BBBBBB;"));
	
	// Creating private shadow statusBar
	statusBarShadow =  new QStatusBar(statusBar);
	statusBarShadow->setGeometry(-1, -1, statusBar->width(), statusBar->height());
	statusBarShadow->setStyleSheet(QString("background-color: transparent; color: blue;"));
	
	// Set current widget and all its children to be Visible 
	show();

	m_exportView = new QAction("Export the view", (QObject*)this);
	connect(m_exportView, SIGNAL(triggered()), this, SLOT(exportView()));
	//connect(m_scene, SIGNAL(changed  ( const QList<QRectF> &)), this, SLOT(updateSceneRect()));
	// Register and GO for actions !
	ViewerManager::registerViewer( this );
}

Viewer::~Viewer()
{
  ViewerManager::unregisterViewer( this );
}

void Viewer::setBackgroundColor(int R, int G, int B)
{
	m_scene->setBackgroundBrush( QColor(R,G,B) );
}

//void Viewer::updateSceneRect()
//{/*{{{*/
//	std::cout << "Viewer::updateSceneRect(): Hello" << std::endl;
//}/*}}}*/

// added by norman
void Viewer::setWindowSize( int width, int height )
{
	m_windowWidth = width;
	m_windowHeight = height;
}

void Viewer::createCursor()
{ /*{{{*/
	// Creating a custom cursor 32x32 px. Drawing a simple cross
	// on 31x31 pixels and setting the hotspot in this 31x31 center
	const int cursorSquareSize = 32;
	// Preparing a PPM P6 format
	const int offset = 13;
	unsigned char cursorData[offset+cursorSquareSize*cursorSquareSize*3];
	snprintf((char*)cursorData, offset, "P6 %d %d 255\n", cursorSquareSize, cursorSquareSize);

	unsigned char cursorDataAlpha[offset+cursorSquareSize*cursorSquareSize*3];
	snprintf((char*)cursorDataAlpha, offset, "P6 %d %d 255\n", cursorSquareSize, cursorSquareSize);

	int cursorHotSpot  = cursorSquareSize/2;
	unsigned char cursorColor1 = 200;
	unsigned char cursorColor2 = 30;
	for (int j=0; j<cursorSquareSize; j++)
	{
		for (int i=0; i<cursorSquareSize; i++)
		{
			// Preparing the Alpha mask
			if( !(i==(cursorSquareSize-1) || j==(cursorSquareSize-1))
				&& !( (cursorHotSpot-2) < i && i < (cursorHotSpot+2) && (cursorHotSpot-2) < j && j < (cursorHotSpot+2) ))
			{
				cursorDataAlpha[offset+j*cursorSquareSize*3+i*3] = 255;
				cursorDataAlpha[offset+j*cursorSquareSize*3+i*3+1] = 255;
				cursorDataAlpha[offset+j*cursorSquareSize*3+i*3+2] = 255;
			} else {
				cursorDataAlpha[offset+j*cursorSquareSize*3+i*3] = 0;
				cursorDataAlpha[offset+j*cursorSquareSize*3+i*3+1] = 0;
				cursorDataAlpha[offset+j*cursorSquareSize*3+i*3+2] = 0;
			}

			// Drawing the cross
			if ( (i==cursorHotSpot) || (j ==cursorHotSpot) )
			{
				if ((i >= cursorHotSpot) && (j >= cursorHotSpot))
				{
					cursorData[offset+j*cursorSquareSize*3+i*3] = cursorColor1;
					cursorData[offset+j*cursorSquareSize*3+i*3+1] = cursorColor1;
					cursorData[offset+j*cursorSquareSize*3+i*3+2] = cursorColor1;
				} else {
					cursorData[offset+j*cursorSquareSize*3+i*3] = cursorColor2;
					cursorData[offset+j*cursorSquareSize*3+i*3+1] = cursorColor2;
					cursorData[offset+j*cursorSquareSize*3+i*3+2] = cursorColor2;
				}
			} else {
				cursorData[offset+j*cursorSquareSize*3+i*3] = 255;
				cursorData[offset+j*cursorSquareSize*3+i*3+1] = 255;
				cursorData[offset+j*cursorSquareSize*3+i*3+2] = 255;
				cursorDataAlpha[offset+j*cursorSquareSize*3+i*3] = 0;
				cursorDataAlpha[offset+j*cursorSquareSize*3+i*3+1] = 0;
				cursorDataAlpha[offset+j*cursorSquareSize*3+i*3+2] = 0;
			}
		}
	}

	// Creating the cursor image pixmap with its drawing+alpha
	QPixmap cursorDraw(cursorSquareSize, cursorSquareSize);
	QPixmap cursorDrawAlpha(cursorSquareSize, cursorSquareSize);

	// Creating our custom QCursor
	if ( cursorDraw.loadFromData(cursorData, offset+cursorSquareSize*cursorSquareSize*3) 
		&& cursorDrawAlpha.loadFromData(cursorDataAlpha, offset+cursorSquareSize*cursorSquareSize*3) )
	{
		cursorDraw.setAlphaChannel(cursorDrawAlpha);
		crossCursor = new QCursor(cursorDraw, cursorHotSpot, cursorHotSpot);
	} else {
		crossCursor = new QCursor(Qt::CrossCursor);
	}
} /*}}}*/

void Viewer::addShape(qdisplay::Shape* si) {
  scene()->addItem(si);
  si->setZValue(m_currentZ++);
}

void Viewer::addLine(qdisplay::Line* li) {
  scene()->addItem(li);
  li->setZValue(m_currentZ++);
}

void Viewer::addPolyLine(qdisplay::PolyLine* pl)
{
  scene()->addItem(pl);
  pl->setZValue(m_currentZ++);
}

void Viewer::setImageView(ImageView* ii, int row, int col)
{
	// In case the ImageView is NULL
	if (ii == NULL)
	{
		if (m_imageMosaic[row][col])
		{
			scene()->removeItem(m_imageMosaic[row][col]);
			delete m_imageMosaic[row][col];
			m_imageMosaic[row][col] = NULL;
		}
		return;
	}	

	// In case the ImageView is already in the scene
	if(scene()->items().contains(ii)) return;

	// In case the ImageView is not in the scene. We add it
	// and we update the QMap m_imageMosaic
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

  	// Position Image View
  ii->setPos( row * m_mosaicWidth, col*m_mosaicHeight);
	
	// added by norman
	if ((m_windowHeight > 0) and (m_windowWidth > 0))
	{
		setGeometry(0,0,m_windowWidth,m_windowHeight);
	}
}

int Viewer::rows()
{
  if( m_imageMosaic.size() == 0)
    return 0;
  int maxrows = 0;
  for(QMap< int, QMap< int, ImageView* > >::iterator it = m_imageMosaic.begin(); it != m_imageMosaic.end(); it++)
  {
    int rows = (--it.value().end()).key();
    if( rows > maxrows) maxrows = rows;
  }
  return maxrows + 1;
}

int Viewer::cols()
{
  if( m_imageMosaic.size() == 0)
    return 0;
  return (--m_imageMosaic.end()).key() + 1;
}

void Viewer::keyPressEvent ( QKeyEvent * event )
{
  switch (event->key()) {
    case Qt::Key_Plus:
      scaleView(1.2);
      break;
    case Qt::Key_Minus:
      scaleView(1 / 1.2);
      break;
  }
  emit onKeyPress(event);
}


void MouseGraphicsScene::mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
	QPointF dp = mouseEvent->buttonDownScreenPos(mouseEvent->button()) - mouseEvent->screenPos();
	bool isClick = (std::abs(dp.x()) < 4) && (std::abs(dp.y()) < 4);
	emit viewer->onMouseClick(mouseEvent, isClick);
}

void Viewer::wheelEvent(QWheelEvent *event)
{
  scaleView(pow((double)2, event->delta() / 240.0));
}

void Viewer::scaleView(qreal scaleFactor)
{
  qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
  if (factor < 0.07 || factor > 100)
    return;

  scale(scaleFactor, scaleFactor);
}

void Viewer::close()
{
  setVisible(false);
}

void Viewer::splitVertical()
{
  QSplitter* parentSplitter = dynamic_cast<QSplitter*>(parentWidget());
  QSplitter* s = new QSplitter(Qt::Vertical, parentWidget());
  if(parentSplitter)
  {
    parentSplitter->insertWidget(parentSplitter->indexOf(this), s);
  }
  Viewer* clone = new Viewer(0,0,scene());
  s->addWidget(this);
  s->addWidget(clone);
  s->setVisible(true);
}

void Viewer::splitHorizontal()
{
  QSplitter* parentSplitter = dynamic_cast<QSplitter*>(parentWidget());
  QSplitter* s = new QSplitter(Qt::Horizontal, parentWidget());
  if(parentSplitter)
  {
    parentSplitter->insertWidget(parentSplitter->indexOf(this), s);
  }
  Viewer* clone = new Viewer(0,0,scene());
  JFR_DEBUG(scene());
  JFR_DEBUG(scene()->views().size());
  s->addWidget(this);
  s->addWidget(clone);
  s->setVisible(true);
}

void Viewer::contextMenuEvent( QContextMenuEvent * event )
{
  if(itemAt(event->pos()) )
  {
    QGraphicsView::contextMenuEvent(event);
  } else {
    QMenu menu;
    menu.addAction(m_exportView);
    menu.exec(event->globalPos());
  }
}

void Viewer::resizeEvent(QResizeEvent  * event)
{/*{{{*/
	QGraphicsView::resizeEvent(event);
	//statusBar->setWidth(event->size()->width());
}/*}}}*/

//#include <QGraphicsSceneMouseEvent>
//#include <QApplication>
void Viewer::mouseReleaseEvent( QMouseEvent* event )
{
  
// WORKAROUND Qt4.4 regression where mouseReleaseEvent are not forwarded to QGraphicsItem
// is this problem still there with Qt4.6 ? Disabling it because it is messing with
// DragMode ScrollHandDrag (not releasing the drag)
#if 0
  if( QGraphicsItem* qgi = itemAt(event->pos() ) )
  {
    ImageView* iv = dynamic_cast<ImageView*>( qgi->group() );
    if( iv )
    {
      QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
      mouseEvent.setWidget(viewport());
      QPointF mousePressScenePoint = mapToScene(event->pos());
      mouseEvent.setButtonDownScenePos(event->button(), mousePressScenePoint);
      mouseEvent.setButtonDownScreenPos(event->button(), event->globalPos());
      mouseEvent.setScenePos(mapToScene(event->pos()));
      mouseEvent.setScreenPos(event->globalPos());
      mouseEvent.setButtons(event->buttons());
      mouseEvent.setButton(event->button());
      mouseEvent.setModifiers(event->modifiers());
      mouseEvent.setAccepted(false);
      mouseEvent.setPos( iv->mapFromScene( mouseEvent.scenePos() ) );
      iv->mouseReleaseEvent( &mouseEvent );
      return;
    }
  }
#endif

	QGraphicsView::mouseReleaseEvent( event );
	viewport()->unsetCursor();
	viewport()->setCursor(*(this->crossCursor));
}

//------------------------------- Mouse Move Events 
// QGraphicsView mouse event
void Viewer::mouseMoveEvent( QMouseEvent* event )
{/*{{{*/
//JFR_DEBUG("Viewer::mouseMoveEvent");
	QGraphicsView::mouseMoveEvent( event );

	/*
	  // Get pointer position in the QGraphicsView and display it in the status
	QPointF cursorPosition = event->posF();

	// Left aligned text for x position. Precision of 3 digits on a raw number (no scientific style)
	QString infoString = QString("Cursor position (%1").arg((double)(cursorPosition.x()), -10, 'f', 5);
	// Right aligned text for y position. Precision of 3 digits on a raw number (no scientific style)
	QString yposString = QString(", %1)").arg((double)(cursorPosition.y()), -10, 'f', 5);
	// Composition the string to display
	infoString.append(yposString);

	// Updating status message
	setStatusMessage( infoString);
	*/
}/*}}}*/



// QGraphicsScene mouse events
void MouseGraphicsScene::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{
//JFR_DEBUG("MouseGraphicsScene::mouseMoveEvent pos " << event->pos().x() << "," << event->pos().y() << ", scenePos " << event->scenePos().x() << "," << event->scenePos().y());
	QPointF pos = event->scenePos();
	// another possibility is to register the Viewer when it is added to
	QList<QGraphicsView *> const &views_ = views();
	for(QList<QGraphicsView *>::const_iterator it = views_.begin(); it != views_.end(); ++it)
	{
		qdisplay::Viewer *viewer = dynamic_cast<qdisplay::Viewer*>(*it);
		if (viewer) viewer->setTitleWithMouseCoordinates(pos.x(), pos.y());
	}
	emit viewer->onMouseMove(event);
	QGraphicsScene::mouseMoveEvent(event);
}


//------------------------------------- Displaying messages in the status bar
void Viewer::setStatusMessage(QString& infoString, int timeout)
{/*{{{*/
	if (statusBar != NULL)
	{
		statusBar->showMessage(infoString, timeout);
		if (statusBarShadow != NULL)
		{
			statusBarShadow->showMessage(infoString, timeout);
		}
	}
} /*}}}*/

void Viewer::setStatusMessage(const char* infoString, int timeout)
{/*{{{*/
	QString msg = QString(infoString);
	setStatusMessage(msg, timeout);
} /*}}}*/

//------------------------------------- Exporting the view
void Viewer::exportView()
{
  QString fileName = QFileDialog::getSaveFileName ( 0, "Export viewer content", "", "PDF Document (*.pdf);;Postscript (*.ps);;PNG Image (*.png);;Tiff Image (*.tiff);;Scalable Vector Graphics (*.svg)" );
  if(fileName == "") return;
  exportView( fileName.toAscii().data() );
}
  
void Viewer::exportView( const std::string& _fileName )
{
  QString fileName = _fileName.c_str();
  QString extension = fileName.split(".").last().toLower();
  if(extension == "pdf" or extension == "ps")
  {
    QPrinter printer;
    printer.setOutputFileName(fileName);
    QSizeF sF = scene()->sceneRect().size().toSize();
    printer.setPageSize(QPrinter::Custom);
    printer.setPaperSize(QSizeF(sF.width(), sF.height() ), QPrinter::DevicePixel);
    printer.setPageMargins(0,0,0,0, QPrinter::DevicePixel);
    if(extension == "pdf") printer.setOutputFormat(QPrinter::PdfFormat);
    else printer.setOutputFormat(QPrinter::PostScriptFormat);
    QPainter painter(&printer);
    this->scene()->render(&painter);
  } else if ( extension == "png" or extension == "tiff" )
  {
    QImage img( scene()->sceneRect().size().toSize() , QImage::Format_RGB32);
    QPainter painter(&img);
    this->scene()->render(&painter);
    if( extension == "png")
    {
        img.save(fileName, "PNG");
    }
    else {
        img.save(fileName, "TIFF");
    }
  } else if ( extension == "svg" )
  {
    QSvgGenerator generator;
    generator.setFileName(fileName);
    generator.setSize(scene()->sceneRect().size().toSize());
    QPainter painter(&generator);
    this->scene()->render(&painter);
    painter.end();
  } else {
    QMessageBox::critical(0, "Unsupported format", "This format " + extension + " is unsupported by the viewer export");
  }
}

void Viewer::setTitle(const std::string& _title )
{
	title = _title;
	setWindowTitle(title.c_str());
}


void Viewer::setTitleWithMouseCoordinates(double x, double y)
{
	std::ostringstream oss;
	oss.precision(2); oss.setf(std::ios::fixed, std::ios::floatfield);
	if (title.length() > 0) oss << title << "   |   ";
	oss << "(" << x << "  " << y << ")";
	setWindowTitle(oss.str().c_str());
}




#include "Viewer.moc"
