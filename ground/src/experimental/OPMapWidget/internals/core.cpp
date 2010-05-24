#include "core.h"

Core::Core():currentPosition(0,0),currentPositionPixel(0,0),LastLocationInBounds(-1,-1),sizeOfMapArea(0,0)
        ,minOfTiles(0,0),maxOfTiles(0,0),started(false),isDragging(false),TooltipTextPadding(10,10),MouseWheelZooming(false),loaderLimit(5)
{
    mousewheelzoomtype=MouseWheelZoomType::MousePositionAndCenter;

}
void Core::run()
{
    bool last = false;

    LoadTask task;

    MtileLoadQueue.lock();
    {
        if(tileLoadQueue.count() > 0)
        {
            task = tileLoadQueue.dequeue();
            {
                last = tileLoadQueue.count() == 0;
                //Debug.WriteLine("TileLoadQueue: " + tileLoadQueue.Count);
            }
        }
    }
    MtileLoadQueue.unlock();
    if(loaderLimit.tryAcquire(1,GMaps::Instance()->Timeout))
    {
        if(task.HasValue())
        {

            {
                Tile m = Matrix.TileAt(task.Pos);

                if(m.Overlays.count() == 0)
                {
                    qDebug()<<"Fill empty TileMatrix: " + task.ToString();

                    Tile t = Tile(task.Zoom, task.Pos);
                    QVector<MapType::Types> layers= GMaps::Instance()->GetAllLayersOfType(GetMapType());

                    foreach(MapType::Types tl,layers)
                    {
                        int retry = 0;
                        do
                        {
                            QByteArray img;

                            // tile number inversion(BottomLeft -> TopLeft) for pergo maps
                            if(tl == MapType::PergoTurkeyMap)
                            {
                                img = GMaps::Instance()->GetImageFrom(tl, Point(task.Pos.X(), maxOfTiles.Height() - task.Pos.Y()), task.Zoom);
                            }
                            else // ok
                            {
                                img = GMaps::Instance()->GetImageFrom(tl, task.Pos, task.Zoom);
                            }

                            if(img.length()!=0)
                            {
                                Moverlays.lock();
                                {
                                    t.Overlays.append(img);
                                }
                                Moverlays.unlock();
                                break;
                            }
                            else if(GMaps::Instance()->RetryLoadTile > 0)
                            {
                                qDebug()<<"ProcessLoadTask: " << task.ToString()<< " -> empty tile, retry " << retry;
                                {
                                    //QThread::usleep(1111);
                                }
                            }
                        }
                        while(++retry < GMaps::Instance()->RetryLoadTile);
                    }

                    if(t.Overlays.count() > 0)
                    {
                        Matrix.SetTileAt(task.Pos,t);
                    }
                    else
                    {
                        t.Clear();
                        //t = null;
                    }

                    // layers = null;
                }
            }


            {
                // last buddy cleans stuff ;}
                if(last)
                {
                    GMaps::Instance()->kiberCacheLock.lockForWrite();
                    GMaps::Instance()->TilesInMemory.RemoveMemoryOverload();
                    GMaps::Instance()->kiberCacheLock.unlock();

                    MtileDrawingList.lock();
                    {
                        Matrix.ClearPointsNotIn(tileDrawingList);
                    }
                    MtileDrawingList.unlock();


                    emit OnTileLoadComplete();


                    emit OnNeedInvalidation();

                }
            }
            loaderLimit.release();
        }
    }
}
void Core::SetZoom(const int &value)
{
    if (zoom!=value && !isDragging)
    {
        zoom=value;
        minOfTiles=Projection()->GetTileMatrixMinXY(value);
        maxOfTiles=Projection()->GetTileMatrixMaxXY(value);
        currentPositionPixel=Projection()->FromLatLngToPixel(currentPosition,value);
        if(started)
        {
            MtileLoadQueue.lock();
            tileLoadQueue.clear();
            MtileLoadQueue.unlock();
            Matrix.Clear();
            GoToCurrentPositionOnZoom();
            UpdateBounds();
            emit OnMapDrag();
            emit OnMapZoomChanged();
            emit OnNeedInvalidation();
        }
    }
}

void Core::SetCurrentPosition(const PointLatLng &value)
{
    if(!IsDragging())
    {
        currentPosition = value;
        SetCurrentPositionGPixel(Projection()->FromLatLngToPixel(value, Zoom()));

        if(started)
        {
            GoToCurrentPosition();
            emit OnCurrentPositionChanged(currentPosition);
        }
    }
    else
    {
        currentPosition = value;
        SetCurrentPositionGPixel(Projection()->FromLatLngToPixel(value, Zoom()));

        if(started)
        {
            emit OnCurrentPositionChanged(currentPosition);
        }
    }
}
void Core::SetMapType(const MapType::Types &value)
{

    if(value != GetMapType())
    {
        mapType = value;

        switch(value)
        {


        case MapType::ArcGIS_Map:
        case MapType::ArcGIS_Satellite:
        case MapType::ArcGIS_ShadedRelief:
        case MapType::ArcGIS_Terrain:
            {
                if(Projection()->Type()!="PlateCarreeProjection")
                {
                    projection = new PlateCarreeProjection();
                }
            }
            break;

        case MapType::ArcGIS_MapsLT_Map_Hybrid:
        case MapType::ArcGIS_MapsLT_Map_Labels:
        case MapType::ArcGIS_MapsLT_Map:
        case MapType::ArcGIS_MapsLT_OrtoFoto:
            {
                if(Projection()->Type()!="LKS94Projection")
                {
                    projection = new LKS94Projection();
                }
            }
            break;

        case MapType::PergoTurkeyMap:
            {
                if(Projection()->Type()!="PlateCarreeProjectionPergo")
                {
                    projection = new PlateCarreeProjectionPergo();
                }
            }
            break;

        case MapType::YandexMapRu:
            {
                if(Projection()->Type()!="MercatorProjectionYandex")
                {
                    projection = new MercatorProjectionYandex();
                }
            }
            break;

        default:
            {
                if(Projection()->Type()!="MercatorProjection")
                {
                    projection = new MercatorProjection();
                }
            }
            break;
        }

        minOfTiles = Projection()->GetTileMatrixMinXY(Zoom());
        maxOfTiles = Projection()->GetTileMatrixMaxXY(Zoom());
        SetCurrentPositionGPixel(Projection()->FromLatLngToPixel(CurrentPosition(), Zoom()));

        if(started)
        {
            CancelAsyncTasks();
            OnMapSizeChanged(Width, Height);
            GoToCurrentPosition();
            ReloadMap();
            emit OnMapTypeChanged(value);

        }
    }

}
void Core::StartSystem()
{
    if(!started)
    {
        started = true;

        ReloadMap();
        GoToCurrentPosition();
    }
}

void Core::UpdateCenterTileXYLocation()
{
    PointLatLng center = FromLocalToLatLng(Width/2, Height/2);
    Point centerPixel = Projection()->FromLatLngToPixel(center, Zoom());
    centerTileXYLocation = Projection()->FromPixelToTileXY(centerPixel);
}

void Core::OnMapSizeChanged(int const& width, int const& height)
{
    Width = width;
    Height = height;

    sizeOfMapArea.SetWidth(1 + (Width/Projection()->TileSize().Width())/2);
    sizeOfMapArea.SetHeight(1 + (Height/Projection()->TileSize().Height())/2);

    UpdateCenterTileXYLocation();

    if(started)
    {
        UpdateBounds();

        emit OnCurrentPositionChanged(currentPosition);
    }
}
void Core::OnMapClose()
{
    //        if(waitOnEmptyTasks != null)
    //        {
    //           try
    //           {
    //              waitOnEmptyTasks.Set();
    //              waitOnEmptyTasks.Close();
    //           }
    //           catch
    //           {
    //           }
    //        }

    CancelAsyncTasks();
}
GeoCoderStatusCode::Types Core::SetCurrentPositionByKeywords(QString const& keys)
{
    GeoCoderStatusCode::Types status = GeoCoderStatusCode::Unknow;
    PointLatLng pos = GMaps::Instance()->GetLatLngFromGeodecoder(keys, status);
    if(pos.IsEmpty() && status == GeoCoderStatusCode::G_GEO_SUCCESS)
    {
        SetCurrentPosition(pos);
    }

    return status;
}
RectLatLng Core::CurrentViewArea()
{
    PointLatLng p = Projection()->FromPixelToLatLng(-renderOffset.X(), -renderOffset.Y(), Zoom());
    double rlng = Projection()->FromPixelToLatLng(-renderOffset.X() + Width, -renderOffset.Y(), Zoom()).Lng();
    double blat = Projection()->FromPixelToLatLng(-renderOffset.X(), -renderOffset.Y() + Height, Zoom()).Lat();
    return RectLatLng::FromLTRB(p.Lng(), p.Lat(), rlng, blat);

}
PointLatLng Core::FromLocalToLatLng(int const& x, int const& y)
{
    return Projection()->FromPixelToLatLng(Point(x - renderOffset.X(), y - renderOffset.Y()), Zoom());
}


Point Core::FromLatLngToLocal(PointLatLng const& latlng)
{
    Point pLocal = Projection()->FromLatLngToPixel(latlng, Zoom());
    pLocal.Offset(renderOffset);
    return pLocal;
}
int Core::GetMaxZoomToFitRect(RectLatLng const& rect)
{
    int zoom = 0;

    for(int i = 1; i <= GMaps::Instance()->MaxZoom; i++)
    {
        Point p1 = Projection()->FromLatLngToPixel(rect.LocationTopLeft(), i);
        Point p2 = Projection()->FromLatLngToPixel(rect.Bottom(), rect.Right(), i);

        if(((p2.X() - p1.X()) <= Width+10) && (p2.Y() - p1.Y()) <= Height+10)
        {
            zoom = i;
        }
        else
        {
            break;
        }
    }

    return zoom;
}
void Core::BeginDrag(Point const& pt)
{
    dragPoint.SetX(pt.X() - renderOffset.X());
    dragPoint.SetY(pt.Y() - renderOffset.Y());
    isDragging = true;
}
void Core::EndDrag()
{
    isDragging = false;
    emit OnNeedInvalidation();

}
void Core::ReloadMap()
{
    if(started)
    {
        qDebug()<<"------------------";

        MtileLoadQueue.lock();
        {
            tileLoadQueue.clear();
        }
        MtileLoadQueue.unlock();
        Matrix.Clear();

        emit OnNeedInvalidation();

    }
}
void Core::GoToCurrentPosition()
{
    // reset stuff
    renderOffset = Point::Empty;
    centerTileXYLocationLast = Point::Empty;
    dragPoint = Point::Empty;

    // goto location
    Drag(Point(-(GetcurrentPositionGPixel().X() - Width/2), -(GetcurrentPositionGPixel().Y() - Height/2)));
}
void Core::GoToCurrentPositionOnZoom()
{
    // reset stuff
    renderOffset = Point::Empty;
    centerTileXYLocationLast = Point::Empty;
    dragPoint = Point::Empty;

    // goto location and centering
    if(MouseWheelZooming)
    {
        if(mousewheelzoomtype != MouseWheelZoomType::MousePositionWithoutCenter)
        {
            Point pt = Point(-(GetcurrentPositionGPixel().X() - Width/2), -(GetcurrentPositionGPixel().Y() - Height/2));
            renderOffset.SetX(pt.X() - dragPoint.X());
            renderOffset.SetY(pt.Y() - dragPoint.Y());
        }
        else // without centering
        {
            renderOffset.SetX(-GetcurrentPositionGPixel().X() - dragPoint.X());
            renderOffset.SetY(-GetcurrentPositionGPixel().Y() - dragPoint.Y());
            renderOffset.Offset(mouseLastZoom);
        }
    }
    else // use current map center
    {
        mouseLastZoom = Point::Empty;

        Point pt = Point(-(GetcurrentPositionGPixel().X() - Width/2), -(GetcurrentPositionGPixel().Y() - Height/2));
        renderOffset.SetX(pt.X() - dragPoint.X());
        renderOffset.SetY(pt.Y() - dragPoint.Y());
    }

    UpdateCenterTileXYLocation();
}
void Core::DragOffset(Point const& offset)
{
    renderOffset.Offset(offset);

    UpdateCenterTileXYLocation();

    if(centerTileXYLocation != centerTileXYLocationLast)
    {
        centerTileXYLocationLast = centerTileXYLocation;
        UpdateBounds();
    }

    {
        LastLocationInBounds = CurrentPosition();
        SetCurrentPosition (FromLocalToLatLng((int) Width/2, (int) Height/2));
    }

    emit OnNeedInvalidation();
    emit OnMapDrag();
}
void Core::Drag(Point const& pt)
{
    renderOffset.SetX(pt.X() - dragPoint.X());
    renderOffset.SetY(pt.Y() - dragPoint.Y());

    UpdateCenterTileXYLocation();

    if(centerTileXYLocation != centerTileXYLocationLast)
    {
        centerTileXYLocationLast = centerTileXYLocation;
        UpdateBounds();
    }

    if(IsDragging())
    {
        LastLocationInBounds = CurrentPosition();
        SetCurrentPosition(FromLocalToLatLng((int) Width/2, (int) Height/2));
    }

    emit OnNeedInvalidation();


    emit OnMapDrag();

}
void Core::CancelAsyncTasks()
{
    if(started)
    {
        MtileLoadQueue.lock();
        {
            tileLoadQueue.clear();
        }
        MtileLoadQueue.unlock();
    }
}
void Core::UpdateBounds()
    {
       MtileDrawingList.lock();
       {
          //FindTilesAround(tileDrawingList);

          qDebug()<<"OnTileLoadStart: " << tileDrawingList.count() << " tiles to load at zoom " << Zoom() << ", time: " << QDateTime::currentDateTime().date();

          emit OnTileLoadStart();


          foreach(Point p,tileDrawingList)
          {
             LoadTask task = LoadTask(p, Zoom());
             {
                MtileLoadQueue.lock();
                {
                   if(!tileLoadQueue.contains(task))
                   {
                      tileLoadQueue.enqueue(task);
                      //this->.QueueUserWorkItem(ProcessLoadTaskCallback);
                   }
                }
                MtileLoadQueue.unlock();
             }

          }
       }

       //UpdateGroundResolution();
    }
