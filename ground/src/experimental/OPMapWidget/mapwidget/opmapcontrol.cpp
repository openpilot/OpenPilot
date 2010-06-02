#include "opmapcontrol.h"
namespace mapcontrol
{
OPMapControl::OPMapControl(QWidget *parent):QWidget(parent),MapRenderTransform(1), maxZoom(17),minZoom(2),zoomReal(0),isSelected(false)
{
    EmptytileBrush = Qt::cyan;
    MissingDataFont =QFont ("Times",10,QFont::Bold);
    EmptyTileText = "We are sorry, but we don't\nhave imagery at this zoom\nlevel for this region.";
    EmptyTileBorders = QPen(Qt::white);
    ScalePen = QPen(Qt::blue);
    SelectionPen = QPen(Qt::blue);
    MapScaleInfoEnabled = true;
    showTileGridLines=true;
    DragButton = Qt::RightButton;
    isMouseOverMarker=false;
    core.SetCurrentRegion(Rectangle(-50, -50, this->width()+100, this->height()+100));
    core.SetMapType(MapType::OpenStreetMap);
    core.SetZoom(3);

    connect(&core,SIGNAL(OnNeedInvalidation()),this,SLOT(Core_OnNeedInvalidation()));


}
void OPMapControl::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    core.StartSystem();
    resize();
}

void OPMapControl::Core_OnNeedInvalidation()
{
    this->repaint();
}

void OPMapControl::paintEvent(QPaintEvent* evnt)
{
    QWidget::paintEvent(evnt);
    QPainter painter(this);
   // painter.setBrush(palette().foreground().color());
   // painter.fillRect(this->rect(),painter.background());
    if(MapRenderTransform!=1)
    {
        QTransform transform;
        transform.scale(MapRenderTransform,MapRenderTransform);
        painter.setWorldTransform(transform);
        {
            DrawMap2D(painter);
        }
        painter.resetTransform();
    }
    else
    {
        DrawMap2D(painter);
    }
    //  painter.drawText(10,10,"TESTE");
}

void OPMapControl::DrawMap2D(QPainter &painter)
{
   // qDebug()<<core.Matrix.count();
   // painter.drawText(10,10,"TESTE");
    for(int i = -core.GetsizeOfMapArea().Width(); i <= core.GetsizeOfMapArea().Width(); i++)
             {
                for(int j = -core.GetsizeOfMapArea().Height(); j <= core.GetsizeOfMapArea().Height(); j++)
                {
                   core.SettilePoint (core.GetcenterTileXYLocation());
                   core.SettilePoint(Point(core.GettilePoint().X()+ i,core.GettilePoint().Y()+j));



                   {
                      Tile* t = core.Matrix.TileAt(core.GettilePoint());
                      //qDebug()<<"OPMapControl::DrawMap2D tile:"<<t->GetPos().ToString()<<" as "<<t->Overlays.count()<<" overlays";
                      //Tile t = Core.Matrix[tileToDraw];
                      if(t!=0)
                      {
                         //qDebug()<< "opmapcontrol:draw2d TileHasValue:"<<t->GetPos().ToString();
                         core.tileRect.SetX(core.GettilePoint().X()*core.tileRect.Width());
                         core.tileRect.SetY(core.GettilePoint().Y()*core.tileRect.Height());
                         core.tileRect.Offset(core.GetrenderOffset());

                         if(core.GetCurrentRegion().IntersectsWith(core.tileRect))
                         {
                            bool found = false;

                            // render tile
                            //lock(t.Overlays)
                            {
                               foreach(QByteArray img,t->Overlays)
                               {
                                  if(img.count()!=0)
                                  {
                                     if(!found)
                                        found = true;
                                     {
                                         painter.drawPixmap(core.tileRect.X(), core.tileRect.Y(), core.tileRect.Width(), core.tileRect.Height(),PureImageProxy::FromStream(img));

                                     }
                                  }
                               }
                            }

                            if(showTileGridLines)
                            {
                                painter.setPen(EmptyTileBorders);
                                painter.drawRect(core.tileRect.X(), core.tileRect.Y(), core.tileRect.Width(), core.tileRect.Height());
                                {
                                    painter.setFont(MissingDataFont);
                                    painter.setPen(Qt::red);
                                    painter.drawText(QRectF(core.tileRect.X(), core.tileRect.Y(), core.tileRect.Width(), core.tileRect.Height()),Qt::AlignCenter,(core.GettilePoint() == core.GetcenterTileXYLocation()? "CENTER: " :"TILE: ")+core.GettilePoint().ToString());
                                    //qDebug()<<"ShowTileGridLine:"<<core.GettilePoint().ToString()<<"=="<<core.GetcenterTileXYLocation().ToString();
                                }
                            }

                            // add text if tile is missing
                            if(!found)
                            {

                               painter.fillRect(QRectF(core.tileRect.X(), core.tileRect.Y(), core.tileRect.Width(), core.tileRect.Height()),EmptytileBrush);
                               painter.setFont(MissingDataFont);
                               painter.drawText(QRectF(core.tileRect.X(), core.tileRect.Y(), core.tileRect.Width(), core.tileRect.Height()),EmptyTileText);



                               painter.setPen(EmptyTileBorders);
                               painter.drawRect(core.tileRect.X(), core.tileRect.Y(), core.tileRect.Width(), core.tileRect.Height());

                               // raise error

                            }
                         }
                      }
                   }
                }
             }
}

void OPMapControl::mousePressEvent ( QMouseEvent* evnt )
{
    if(!IsMouseOverMarker())
            {
               if(evnt->button() == DragButton && CanDragMap())
               {
                  core.mouseDown.SetX(evnt->x());
                  core.mouseDown.SetY(evnt->y());


                  this->setCursor(Qt::SizeAllCursor);

                  core.BeginDrag(core.mouseDown);
                    this->repaint();

               }
               else if(!isSelected)
               {
                  isSelected = true;
                  SetSelectedArea (RectLatLng::Empty);
                  selectionEnd = PointLatLng::Empty;
                  selectionStart = FromLocalToLatLng(evnt->x(), evnt->y());
               }
            }

    QWidget::mousePressEvent(evnt);
}
PointLatLng OPMapControl::FromLocalToLatLng(int x, int y)
{
    if(MapRenderTransform!=-1)
    {
        x = (int) (x * MapRenderTransform);
        y = (int) (y * MapRenderTransform);
    }
    return core.FromLocalToLatLng(x, y);
}
void OPMapControl::mouseReleaseEvent ( QMouseEvent* evnt )
{
    QWidget::mouseReleaseEvent(evnt);

            if(isSelected)
            {
               isSelected = false;
            }

            if(core.IsDragging())
            {
               core.EndDrag();

               this->setCursor(Qt::ArrowCursor);
               if(!BoundsOfMap.IsEmpty() && !BoundsOfMap.Contains(CurrentPosition()))
               {
                  if(!core.LastLocationInBounds.IsEmpty())
                  {
                     SetCurrentPosition(core.LastLocationInBounds);
                  }
               }
            }
            else
            {
               if(!selectionEnd.IsEmpty() && !selectionStart.IsEmpty())
               {
                   if(!SelectedArea().IsEmpty() && evnt->modifiers() == Qt::ShiftModifier)
                  {
                  //   SetZoomToFitRect(SelectedArea());TODO
                  }
               }

            }

}

void OPMapControl::mouseMoveEvent ( QMouseEvent* evnt )
{
    if(core.IsDragging())
            {
                  core.mouseCurrent.SetX(evnt->x());
                  core.mouseCurrent.SetY(evnt->y());

                  {
                     core.Drag(core.mouseCurrent);
                  }

            }
}
void OPMapControl::resizeEvent ( QResizeEvent * event )
{
    QWidget::resizeEvent(event);
    resize();
}
void OPMapControl::resize()
{

    core.OnMapSizeChanged(this->width(),this->height());
    core.SetCurrentRegion(Rectangle(-50, -50, this->width()+100, this->height()+100));
    if(isVisible())
    {
        core.GoToCurrentPosition();
    }
}
void OPMapControl::wheelEvent(QWheelEvent *event)
{
    QWidget::wheelEvent(event);

             if(!IsMouseOverMarker() && !IsDragging())
             {
                if(core.GetmouseLastZoom().X() != event->pos().x() && core.mouseLastZoom.Y() != event->pos().y())
                {
                    if(GetMouseWheelZoomType() == MouseWheelZoomType::MousePositionAndCenter)
                   {
                      core.SetCurrentPosition(FromLocalToLatLng(event->pos().x(), event->pos().y()));
                   }
                    else if(GetMouseWheelZoomType() == MouseWheelZoomType::ViewCenter)
                   {
                      core.SetCurrentPosition(FromLocalToLatLng((int) width()/2, (int) height()/2));
                   }
                    else if(GetMouseWheelZoomType() == MouseWheelZoomType::MousePositionWithoutCenter)
                   {
                      core.SetCurrentPosition(FromLocalToLatLng(event->pos().x(), event->pos().y()));

                   }

                   core.mouseLastZoom.SetX((event->pos().x()));
                   core.mouseLastZoom.SetY((event->pos().y()));
                }

                // set mouse position to map center
                if(GetMouseWheelZoomType() != MouseWheelZoomType::MousePositionWithoutCenter)
                {
                   {
//                      System.Drawing.Point p = PointToScreen(new System.Drawing.Point(Width/2, Height/2));
//                      Stuff.SetCursorPos((int) p.X, (int) p.Y);
                   }
                }

                core.MouseWheelZooming = true;

                if(event->delta() > 0)
                {
                   SetZoom(Zoom()+1);
                }
                else if(event->delta() < 0)
                {
                   SetZoom(Zoom()-1);
                }

                core.MouseWheelZooming = false;
             }
}
double OPMapControl::Zoom()
{


    return zoomReal;
}
void OPMapControl::SetZoom(double const& value)
{
    if(zoomReal != value)
    {
        if(value > MaxZoom())
        {
            zoomReal = MaxZoom();
        }
        else
            if(value < MinZoom())
            {
            zoomReal = MinZoom();
        }
        else
        {
            zoomReal = value;
        }

        float remainder = (float)std::fmod((float) value, (float) 1);
        if(remainder != 0)
        {
            float scaleValue = remainder + 1;
            {
                MapRenderTransform = scaleValue;
            }

            SetZoomStep((qint32)(value - remainder));

            this->repaint();

        }
        else
        {

            MapRenderTransform = 1;

            SetZoomStep ((qint32)(value));
            zoomReal = ZoomStep();
            this->repaint();
        }
    }
}
int OPMapControl::ZoomStep()const
{
    return core.Zoom();
}
void OPMapControl::SetZoomStep(int const& value)
{
    if(value > MaxZoom())
    {
        core.SetZoom(MaxZoom());
    }
    else if(value < MinZoom())
    {
        core.SetZoom(MinZoom());
    }
    else
    {
        core.SetZoom(value);
    }
}

void OPMapControl::Offset(int const& x, int const& y)
{
    core.DragOffset(Point(x, y));
}
void OPMapControl::closeEvent(QCloseEvent *event)
{
    core.OnMapClose();
    event->accept();
}
}
