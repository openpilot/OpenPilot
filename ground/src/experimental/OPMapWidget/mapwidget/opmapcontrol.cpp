#include "opmapcontrol.h"

OPMapControl::OPMapControl(QWidget *parent):QWidget(parent),MapRenderTransform(1), maxZoom(2),minZoom(2)
{
    EmptytileBrush = Qt::blue;
    MissingDataFont =QFont ("Times",10,QFont::Bold);
    EmptyTileText = "We are sorry, but we don't\nhave imagery at this zoom\nlevel for this region.";
    EmptyTileBorders = QPen(Qt::white);
    ShowTileGridLines=true;
    core.SetCurrentRegion(Rectangle(-50, -50, this->width()+100, this->height()+100));
    core.SetMapType(MapType::GoogleSatellite);
    core.SetZoom(2);
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
                         qDebug()<< "opmapcontrol:draw2d TileHasValue:"<<t->GetPos().ToString();
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

                            if(ShowTileGridLines)
                            {
                                painter.setPen(EmptyTileBorders);
                                painter.drawRect(core.tileRect.X(), core.tileRect.Y(), core.tileRect.Width(), core.tileRect.Height());
                                {
                                    painter.setFont(MissingDataFont);
                                    painter.setPen(Qt::red);
                                    painter.drawText(QRectF(core.tileRect.X(), core.tileRect.Y(), core.tileRect.Width(), core.tileRect.Height()),Qt::AlignCenter,(core.GettilePoint() == core.GetcenterTileXYLocation()? "CENTER: " :"TILE: ")+core.GettilePoint().ToString());
                                    qDebug()<<"ShowTileGridLine:"<<core.GettilePoint().ToString()<<"=="<<core.GetcenterTileXYLocation().ToString();
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

}

void OPMapControl::mouseReleaseEvent ( QMouseEvent* evnt )
{

}

void OPMapControl::mouseMoveEvent ( QMouseEvent* evnt )
{

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
