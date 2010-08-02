/**
******************************************************************************
*
* @file       mapgraphicitem.cpp
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
* @brief      The main graphicsItem used on the widget, contains the map and map logic
* @see        The GNU Public License (GPL) Version 3
* @defgroup   OPMapWidget
* @{
*
*****************************************************************************/
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "uavitem.h"
#include "homeitem.h"
#include "mapgraphicitem.h"

namespace mapcontrol
{
    MapGraphicItem::MapGraphicItem(internals::Core *core, Configuration *configuration):core(core),config(configuration),MapRenderTransform(1), maxZoom(17),minZoom(2),zoomReal(0),isSelected(false),rotation(0)
    {

        showTileGridLines=false;
        isMouseOverMarker=false;
        maprect=QRectF(0,0,1022,680);
        core->SetCurrentRegion(internals::Rectangle(0, 0, maprect.width(), maprect.height()));
        core->SetMapType(MapType::GoogleHybrid);
        this->SetZoom(2);
        connect(core,SIGNAL(OnNeedInvalidation()),this,SLOT(Core_OnNeedInvalidation()));
        connect(core,SIGNAL(OnMapDrag()),this,SLOT(ChildPosRefresh()));
        connect(core,SIGNAL(OnMapZoomChanged()),this,SLOT(ChildPosRefresh()));
        //resize();
    }
    void MapGraphicItem::start()
    {
        core->StartSystem();
    }

    void MapGraphicItem::resize(const QRectF &rect)
    {

        {
            this->prepareGeometryChange();
            maprect=boundingBox(scene()->sceneRect(),rotation);
            this->setTransform(QTransform().translate(-(maprect.width()-scene()->width())/2,-(maprect.height()-scene()->height())/2));
            this->setTransformOriginPoint(maprect.center().x(),maprect.center().y());
            this->setRotation(rotation);
        }

        core->OnMapSizeChanged(maprect.width(),maprect.height());
        core->SetCurrentRegion(internals::Rectangle(0, 0, maprect.width(), maprect.height()));
        if(isVisible())
        {
            core->GoToCurrentPosition();
        }
    }

    QRectF MapGraphicItem::boundingRect() const
    {
        const int Margin = 1;
        return maprect.adjusted(-Margin, -Margin, +Margin, +Margin);
    }
    void MapGraphicItem::Core_OnNeedInvalidation()
    {
        this->update();
        foreach(QGraphicsItem* i,this->childItems())
        {
            WayPointItem* w=qgraphicsitem_cast<WayPointItem*>(i);
            if(w)
                w->RefreshPos();
            UAVItem* ww=qgraphicsitem_cast<UAVItem*>(i);
            if(ww)
                ww->RefreshPos();
            HomeItem* www=qgraphicsitem_cast<HomeItem*>(i);
            if(www)
                www->RefreshPos();

        }
    }
    void MapGraphicItem::ChildPosRefresh()
    {
        foreach(QGraphicsItem* i,this->childItems())
        {
            WayPointItem* w=qgraphicsitem_cast<WayPointItem*>(i);
            if(w)
                w->RefreshPos();
            UAVItem* ww=qgraphicsitem_cast<UAVItem*>(i);
            if(ww)
                ww->RefreshPos();
            HomeItem* www=qgraphicsitem_cast<HomeItem*>(i);
            if(www)
                www->RefreshPos();
        }
    }
    void MapGraphicItem::ConstructLastImage(int const& zoomdiff)
    {
        QImage temp;
        QSize size=boundingRect().size().toSize();
        size.setWidth(size.width()*2*zoomdiff);
        size.setHeight(size.height()*2*zoomdiff);
        temp=QImage(size,
                               QImage::Format_ARGB32_Premultiplied);
        temp.fill(0);
        QPainter imagePainter(&temp);
        imagePainter.translate(-boundingRect().topLeft());
        imagePainter.scale(2*zoomdiff,2*zoomdiff);
        paintImage(&imagePainter);
        imagePainter.end();
        lastimagepoint=Point(core->GetrenderOffset().X()*2*zoomdiff,core->GetrenderOffset().Y()*2*zoomdiff);
        lastimage=temp;
    }
    void MapGraphicItem::paintImage(QPainter *painter)
    {

        if(MapRenderTransform!=1)
        {
            QTransform transform;
            transform.scale(MapRenderTransform,MapRenderTransform);
            painter->setWorldTransform(transform);
            {
                DrawMap2D(painter);
            }
            painter->resetTransform();
        }
        else
        {
            DrawMap2D(painter);
        }
        //painter->drawRect(maprect);
    }
    void MapGraphicItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {

        if(MapRenderTransform!=1)
        {
            QTransform transform;
            transform.scale(MapRenderTransform,MapRenderTransform);
            painter->setWorldTransform(transform);
            {
                DrawMap2D(painter);
            }
            painter->resetTransform();
        }
        else
        {
            DrawMap2D(painter);
        }
    }
    void MapGraphicItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        if(core->IsDragging())
        {
            core->mouseCurrent.SetX(event->pos().x());
            core->mouseCurrent.SetY(event->pos().y());

            {
                core->Drag(core->mouseCurrent);
            }

        }
        QGraphicsItem::mouseMoveEvent(event);
    }
    void MapGraphicItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        if(!IsMouseOverMarker())
        {
            if(event->button() == config->DragButton && CanDragMap())
            {
                core->mouseDown.SetX(event->pos().x());
                core->mouseDown.SetY(event->pos().y());


                this->setCursor(Qt::SizeAllCursor);

                core->BeginDrag(core->mouseDown);
                this->update();

            }
            else if(!isSelected)
            {
                isSelected = true;
                SetSelectedArea (internals::RectLatLng::Empty);
                selectionEnd = internals::PointLatLng::Empty;
                selectionStart = FromLocalToLatLng(event->pos().x(), event->pos().y());
            }
        }
    }
    void MapGraphicItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        if(isSelected)
        {
            isSelected = false;
        }

        if(core->IsDragging())
        {
            core->EndDrag();

            this->setCursor(Qt::ArrowCursor);
            if(!BoundsOfMap.IsEmpty() && !BoundsOfMap.Contains(core->CurrentPosition()))
            {
                if(!core->LastLocationInBounds.IsEmpty())
                {
                    core->SetCurrentPosition(core->LastLocationInBounds);
                }
            }
        }
        else
        {
            if(!selectionEnd.IsEmpty() && !selectionStart.IsEmpty())
            {
                if(!selectedArea.IsEmpty() && event->modifiers() == Qt::ShiftModifier)
                {
                    //   SetZoomToFitRect(SelectedArea());TODO
                }
            }

        }
    }
    void MapGraphicItem::wheelEvent(QGraphicsSceneWheelEvent *event)
    {
        if(!IsMouseOverMarker() && !IsDragging())
        {
            if(core->GetmouseLastZoom().X() != event->pos().x() && core->mouseLastZoom.Y() != event->pos().y())
            {
                if(GetMouseWheelZoomType() == internals::MouseWheelZoomType::MousePositionAndCenter)
                {
                    core->SetCurrentPosition(FromLocalToLatLng(event->pos().x(), event->pos().y()));
                }
                else if(GetMouseWheelZoomType() == internals::MouseWheelZoomType::ViewCenter)
                {
                    core->SetCurrentPosition(FromLocalToLatLng((int) maprect.width()/2, (int) maprect.height()/2));
                }
                else if(GetMouseWheelZoomType() == internals::MouseWheelZoomType::MousePositionWithoutCenter)
                {
                    core->SetCurrentPosition(FromLocalToLatLng(event->pos().x(), event->pos().y()));

                }

                core->mouseLastZoom.SetX((event->pos().x()));
                core->mouseLastZoom.SetY((event->pos().y()));
            }

            // set mouse position to map center
            if(GetMouseWheelZoomType() != internals::MouseWheelZoomType::MousePositionWithoutCenter)
            {
                {
                    //                      System.Drawing.Point p = PointToScreen(new System.Drawing.Point(Width/2, Height/2));
                    //                      Stuff.SetCursorPos((int) p.X, (int) p.Y);
                }
            }

            core->MouseWheelZooming = true;

            if(event->delta() > 0)
            {
                SetZoom(Zoom()+1);
            }
            else if(event->delta() < 0)
            {
                SetZoom(Zoom()-1);
            }

            core->MouseWheelZooming = false;
        }
    }
    void MapGraphicItem::DrawMap2D(QPainter *painter)
    {

        if(!lastimage.isNull())
            painter->drawImage(core->GetrenderOffset().X()-lastimagepoint.X(),core->GetrenderOffset().Y()-lastimagepoint.Y(),lastimage);

        for(int i = -core->GetsizeOfMapArea().Width(); i <= core->GetsizeOfMapArea().Width(); i++)
        {
            for(int j = -core->GetsizeOfMapArea().Height(); j <= core->GetsizeOfMapArea().Height(); j++)
            {
                core->SettilePoint (core->GetcenterTileXYLocation());
                core->SettilePoint(Point(core->GettilePoint().X()+ i,core->GettilePoint().Y()+j));



                {
                    internals::Tile* t = core->Matrix.TileAt(core->GettilePoint());
                    //qDebug()<<"OPMapControl::DrawMap2D tile:"<<t->GetPos().ToString()<<" as "<<t->Overlays.count()<<" overlays";
                    //Tile t = core->Matrix[tileToDraw];
                    if(true)
                    {
                        //qDebug()<< "opmapcontrol:draw2d TileHasValue:"<<t->GetPos().ToString();
                        core->tileRect.SetX(core->GettilePoint().X()*core->tileRect.Width());
                        core->tileRect.SetY(core->GettilePoint().Y()*core->tileRect.Height());
                        core->tileRect.Offset(core->GetrenderOffset());
                        //qDebug()<<core->GetrenderOffset().ToString();

                        if(core->GetCurrentRegion().IntersectsWith(core->tileRect))
                        {
                            bool found = false;

                            // render tile
                            //lock(t.Overlays)
                            if(t!=0)
                            {
                                foreach(QByteArray img,t->Overlays)
                                {
                                    if(img.count()!=0)
                                    {
                                        if(!found)
                                            found = true;
                                        {
                                            painter->drawPixmap(core->tileRect.X(),core->tileRect.Y(), core->tileRect.Width(), core->tileRect.Height(),PureImageProxy::FromStream(img));

                                        }
                                    }
                                }
                            }

                            if(showTileGridLines)
                            {
                                painter->setPen(config->EmptyTileBorders);
                                painter->drawRect(core->tileRect.X(), core->tileRect.Y(), core->tileRect.Width(), core->tileRect.Height());
                                {
                                    painter->setFont(config->MissingDataFont);
                                    painter->setPen(Qt::red);
                                    painter->drawText(QRectF(core->tileRect.X(), core->tileRect.Y(), core->tileRect.Width(), core->tileRect.Height()),Qt::AlignCenter,(core->GettilePoint() == core->GetcenterTileXYLocation()? "CENTER: " :"TILE: ")+core->GettilePoint().ToString());
                                    //qDebug()<<"ShowTileGridLine:"<<core->GettilePoint().ToString()<<"=="<<core->GetcenterTileXYLocation().ToString();
                                }
                            }

                            // add text if tile is missing
                            if(false)
                            {

                                painter->fillRect(QRectF(core->tileRect.X(), core->tileRect.Y(), core->tileRect.Width(), core->tileRect.Height()),config->EmptytileBrush);
                                painter->setFont(config->MissingDataFont);
                                painter->drawText(QRectF(core->tileRect.X(), core->tileRect.Y(), core->tileRect.Width(), core->tileRect.Height()),config->EmptyTileText);



                                painter->setPen(config->EmptyTileBorders);
                                painter->drawRect(core->tileRect.X(), core->tileRect.Y(), core->tileRect.Width(), core->tileRect.Height());

                                // raise error

                            }
                        }
                    }
                }
            }
        }
        // painter->drawRect(core->GetrenderOffset().X()-lastimagepoint.X()-3,core->GetrenderOffset().Y()-lastimagepoint.Y()-3,lastimage.width(),lastimage.height());
    }


    core::Point MapGraphicItem::FromLatLngToLocal(internals::PointLatLng const& point)
    {
        core::Point ret = core->FromLatLngToLocal(point);
        if(MapRenderTransform!=1)
        {
            ret.SetX((int) (ret.X() / MapRenderTransform));
            ret.SetY((int) (ret.Y() / MapRenderTransform));
        }
        return ret;
    }
    internals::PointLatLng MapGraphicItem::FromLocalToLatLng(int x, int y)
    {
        if(MapRenderTransform!=-1)
        {
            x = (int) (x * MapRenderTransform);
            y = (int) (y * MapRenderTransform);
        }
        return core->FromLocalToLatLng(x, y);
    }

    double MapGraphicItem::Zoom()
    {
        return zoomReal;
    }
    void MapGraphicItem::SetZoom(double const& value)
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

                this->update();

            }
            else
            {

                MapRenderTransform = 1;

                SetZoomStep ((qint32)(value));
                zoomReal = ZoomStep();
                this->update();
            }
        }
    }
    int MapGraphicItem::ZoomStep()const
    {
        return core->Zoom();
    }
    void MapGraphicItem::SetZoomStep(int const& value)
    {
        if(value-core->Zoom()>0 && value<= MaxZoom())
            ConstructLastImage(value-core->Zoom());
        else
            lastimage=QImage();
        if(value > MaxZoom())
        {
            core->SetZoom(MaxZoom());
            emit zoomChanged(MaxZoom());
        }
        else if(value < MinZoom())
        {
            core->SetZoom(MinZoom());
            emit zoomChanged(MinZoom());
        }
        else
        {
            core->SetZoom(value);
            emit zoomChanged(value);
        }

    }

    void MapGraphicItem::Offset(int const& x, int const& y)
    {
        core->DragOffset(Point(x, y));
    }
    void MapGraphicItem::mapRotate(qreal angle)
    {
        rotation=angle;
        resize(scene()->sceneRect());
    }
    QRectF MapGraphicItem::boundingBox(const QRectF &rect, const qreal &angle)
    {
        QRectF ret(rect);
        float c=cos(angle*2*M_PI/360);
        float s=sin(angle*2*M_PI/360);
        ret.setHeight(rect.height()*fabs(c)+rect.width()*fabs(s));
        ret.setWidth(rect.width()*fabs(c)+rect.height()*fabs(s));
        return ret;
    }
    QSize MapGraphicItem::sizeHint()const
    {
        core::Size size=core->projection->GetTileMatrixMaxXY(MinZoom());
        core::Size tilesize=core->projection->TileSize();
        QSize rsize((size.Width()+1)*tilesize.Width(),(size.Height()+1)*tilesize.Height());
        return rsize;
    }
}
