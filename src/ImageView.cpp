#include "qdisplay/ImageView.hpp"

#include <QAction>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QMessageBox>

#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
 
#include <kernel/jafarMacro.hpp>

#include "qdisplay/AbstractEventHandler.hpp"
#include "qdisplay/Line.hpp"
#include "qdisplay/Shape.hpp"

namespace jafar {
namespace qdisplay {

ImageView::ImageView(const jafar::image::Image& img) :
    m_pixmapItem(new QGraphicsPixmapItem()),
    m_currentZ(0.),
    m_eventHandler(0)
{
  setImage(img);
  m_pixmapItem->setZValue(m_currentZ++);
  addToGroup(m_pixmapItem);
  m_lutRandomizeAction = new QAction("Randomize the lut", (QObject*)this);
  this->connect(m_lutRandomizeAction, SIGNAL(triggered()), this, SLOT(lutRandomize()));
  m_lutGrayscaleAction = new QAction("Use a grayscale lut", (QObject*)this);
  this->connect(m_lutGrayscaleAction, SIGNAL(triggered()), this, SLOT(lutGrayscale()));
  m_lutInvertGrayscaleAction = new QAction("Use an invert grayscale lut", (QObject*)this);
  this->connect(m_lutInvertGrayscaleAction, SIGNAL(triggered()), this, SLOT(lutInvertGrayscale()));
  m_exportView = new QAction("Export the view", (QObject*)this);
  this->connect(m_exportView, SIGNAL(triggered()), this, SLOT(exportView()));
}

void ImageView::addShape(Shape* si)
{
  JFR_PRED_RUN_TIME(scene(), "You first need to add the ImageView to a scene");
  addToGroup(si);
  scene()->addItem(si);
  si->setZValue(m_currentZ++);
  si->moveBy(pos().x(), pos().y());
}

void ImageView::addLine(Line* li)
{
  JFR_PRED_RUN_TIME(scene(), "You first need to add the ImageView to a scene");
  addToGroup(li);
  scene()->addItem(li);
  li->setZValue(m_currentZ++);
  li->moveBy(pos().x(), pos().y());
}

void ImageView::setImage(const jafar::image::Image& jfrimg)
{
  int width = jfrimg.width();
  int height = jfrimg.height();
  QColor c;
  if(jfrimg.channels() == 1)
  {
    m_image = QImage(width, height, QImage::Format_Indexed8);
    QVector<QRgb> colorTable;
    for(int i = 0; i < 255; i++)
    {
      colorTable.push_back( qRgb(i,i,i));
    }
    m_image.setColorTable( colorTable );
    for(int x = 0; x < width; x++)
    {
      for(int y = 0; y < height; y++)
      {
        const uchar* v = jfrimg.data( y) + x;
        m_image.setPixel(x, y, *v);
      }
    }
  } else if(jfrimg.channels() == 3) {
    m_image = QImage(width, height, QImage::Format_RGB32);
    for(int x = 0; x < width; x++)
    {
      for(int y = 0; y < height; y++)
      {
        const uchar* v = jfrimg.data( y) + 3*x;
        c.setRgb(v[2], v[1], v[0]);
        m_image.setPixel(x, y, c.rgb());
      }
    }
  }
  m_pixmapItem->setPixmap(QPixmap::fromImage(m_image));
}

void ImageView::lutRandomize()
{
  
  if(m_image.format() == QImage::Format_Indexed8)
  {
    QVector<QRgb> colorTable;
    for(int i = 0; i < 256; i++)
    {
      colorTable.push_back( qRgb(255 * rand(),255 * rand(),255 * rand()));
    }
    m_image.setColorTable( colorTable );
    m_pixmapItem->setPixmap(QPixmap::fromImage(m_image));
  }
}

void ImageView::lutGrayscale()
{
  
  if(m_image.format() == QImage::Format_Indexed8)
  {
    QVector<QRgb> colorTable;
    for(int i = 0; i < 256; i++)
    {
      colorTable.push_back( qRgb(i, i, i));
    }
    m_image.setColorTable( colorTable );
    m_pixmapItem->setPixmap(QPixmap::fromImage(m_image));
  }
}

void ImageView::lutInvertGrayscale()
{
  if(m_image.format() == QImage::Format_Indexed8)
  {
    QVector<QRgb> colorTable;
    for(int i = 0; i < 256; i++)
    {
      colorTable.push_back( qRgb(255 - i, 255 - i, 255 - i));
    }
    m_image.setColorTable( colorTable );
    m_pixmapItem->setPixmap(QPixmap::fromImage(m_image));
  }
}

void ImageView::exportView()
{
  QString fileName = QFileDialog::getSaveFileName ( 0, "Export viewer content", "", "Supported format (*.pdf *.ps *.png *.tiff)" );
  if(fileName == "") return;
  QString extension = fileName.split(".").last().toLower();
  if(extension == "pdf" or extension == "ps")
  {
    QPrinter printer;
    printer.setOutputFileName(fileName);
    QSizeF sF = scene()->sceneRect().size().toSize();
    if(sF.height() < sF.width() ) printer.setOrientation(QPrinter::Landscape);
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
        img.save(fileName, "PNG", 100);
    }
    else {
        img.save(fileName, "TIFF", 100);
    }
  } else {
    QMessageBox::critical(0, "Unsupported format", "This format " + extension + " is unsupported by the viewer export");
  }
}

void ImageView::contextMenuEvent ( QGraphicsSceneContextMenuEvent * event )
{
  QMenu menu;
  if(m_image.format() == QImage::Format_Indexed8)
  {
    menu.addAction(m_lutRandomizeAction);
    menu.addAction(m_lutGrayscaleAction);
    menu.addAction(m_lutInvertGrayscaleAction);
  }
  menu.addAction(m_exportView);
  menu.exec(event->screenPos());
}

void ImageView::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
  if(m_eventHandler)
  {
    m_eventHandler->mouseReleaseEvent( event->button(), event->pos().x(), event->pos().y() );
  }
  QGraphicsItemGroup::mouseReleaseEvent(event);
}

}

}

#include "ImageView.moc"
