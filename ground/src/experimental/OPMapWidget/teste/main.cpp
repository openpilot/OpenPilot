#include <QApplication>

#include "gmaps.h"
#include <QDebug>
#include "../core/pureimagecache.h"
#include "QPixmap"
#include <QIODevice>
#include <QtGui/QLabel>
#include "../core/pureimage.h"
#include "../core/rawtile.h"
#include "../core/cache.h"
#include "../core/languagetype.h"
#include "../core/tilecachequeue.h"
#include "../core/cacheitemqueue.h"
#include "../core/maptype.h"
#include "../core/alllayersoftype.h"
#include "geodecoderstatus.h"
//#include "QTest"
#include "../internals/core.h"
#include "../core/size.h"
#include "../internals/rectangle.h"
#include "../internals/tile.h"
#include "../internals/tilematrix.h"
#include "../core/point.h"
#include "../core/size.h"
#include "../internals/copyrightstrings.h"
#include "../internals/projections/lks94projection.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
 //   GMaps g;
//    qDebug()<<g.GoogleMapsAPIKey;
//    qDebug()<<QString(g.levelsForSigPacSpainMap[2]);
//    qDebug()<<g.MakeImageUrl(GoogleMap,Point(18,10),5,QString("en"));
//    qDebug()<<g.MakeImageUrl(GoogleMap,Point(5,7),3,QString("en"));

//    qDebug()<<"DB creation="<<PureImageCache::CreateEmptyDB("C:/Users/Xapo/Documents/QT/QMapControlXx/main.cpp");
//    qDebug()<<"end";
      PureImageCache p;
      PureImageProxy proxy;
      QPixmap pixmap("C:/Users/Xapo/Pictures/x.jpg");
      QPixmap pixmapb;
      QByteArray ba;
      QByteArray bb;
      QBuffer buffer( &ba );
      buffer.open(QIODevice::WriteOnly);
      pixmap.save( &buffer, "PNG" ); // writes pixmap into ba in PNG format
      //pixmapb=Cache::Instance()->ImageCache.GetImageFromCache(GoogleMap,Point(1,2),3);
    //  pixmapb=p.GetImageFromCache(GoogleMap,Point(1,2),3);
//      RawTile r(GoogleMap,Point(1,2),3);
//      g.AddTileToMemoryCache(r,pixmapb);
//pixmap=QPixmap("C:/Users/Xapo/Pictures/y.jpg");
//g.AddTileToMemoryCache(RawTile(GoogleMap,Point(2,2),3),pixmap);
//
//      //pixmapb= QPixmap::fromImage(QImage::fromData(bb, "png"));;
//      qDebug()<<pixmapb.width();
//     // pixmapb=proxy.FromStream(bb);
//      bb.clear();
//      QLabel label;
      QLabel labell;
//  pixmapb=QPixmap();
//  pixmapb=g.GetTileFromMemoryCache(r);
//      label.setPixmap(pixmapb);
//
//      label.show();
//      pixmapb=g.GetTileFromMemoryCache(RawTile(GoogleMap,Point(4,2),3));

//          LanguageType::Types l;//=LanguageType::English;
//          l=LanguageType::PortuguesePortugal;
//          qDebug()<<LanguageType().toString(LanguageType::PortuguesePortugal);
//qDebug()<<(l==LanguageType::Arabic);
//// TileCacheQueue queue(&p);
////for(int x=0;x<10;++x)
////{
////    CacheItemQueue item(GoogleMap,Point(69,79),QByteArray(),x);
////   // QTest::qsleep(200);
////    queue.EnqueueCacheTask(item);
////}
//pixmapb=PureImageProxy::FromStream(GMaps::Instance()->GetImageFrom(MapType::GoogleMap,Point(7,5),8));
//qDebug()<<"WITH"<<pixmapb.width();
labell.setPixmap(pixmapb);
////
labell.show();
// QCoreApplication::processEvents(QEventLoop::AllEvents);
////pixmapb=GMaps::Instance()->GetImageFrom(MapType::GoogleSatellite,Point(1,0),1);
////labell.setPixmap(pixmapb);
////GeoCoderStatusCode::Types f;
////qDebug()<<"LAT"<<GMaps::Instance()->GetLatLngFromGeodecoder("lisbon",f).Lat();
////QString s=GMaps::Instance()->GetPlacemarkFromGeocoder(GMaps::Instance()->GetLatLngFromGeodecoder("lisbon",f)).Address();
////labell.show();
////labell.setText(s);
//threadpool *g=new threadpool();
//              ;
//QThreadPool tp;
//g->setAutoDelete(false);
////tp.tryStart(new threadpool());
////tp.tryStart(new threadpool());
////tp.tryStart(new threadpool());
//int i=10;
//while(i>0)
//{
//    tp.tryStart(g);
//    tp.tryStart(g);
//    //tp.tryStart(g);
//   // qDebug()<<tp.activeThreadCount();
//   // qDebug()<<"IDEAL"<<QThread::idealThreadCount();
//   // while(tp.activeThreadCount()!=1){}
//    while(tp.activeThreadCount()!=0){}
//    --i;
//}
//foreach(QByteArray arr,g->pix)
//    {
//        QLabel *lab=new QLabel();
//        QPixmap p;
//
//        p=PureImageProxy::FromStream(arr);
//        lab->setPixmap(p);
//        lab->show();
//
//    }
// //QCoreApplication::processEvents(QEventLoop::AllEvents);
//  //  QCoreApplication::processEvents(QEventLoop::AllEvents);
//    //while (tp.activeThreadCount()!=0)
//    //    qDebug()<<tp.activeThreadCount();
//
Core c;
c.SetCurrentPosition(PointLatLng(10,20));
//qDebug()<<"GetLatLng:"<<c.GetCurrentPosition().Lat();
Size s(Point(10,10));
Size ss(5,5);
Point ppp(2,3);
//Point pp(1,2);
Rectangle rt(0,0,10,11);
Rectangle rtt(0,0,10,12);

qDebug()<<"+"<<(s+ss).Height()<<" "<<(s+ss).Width();
Point pp(1,2);
RawTile r(MapType::ArcGIS_Map,Point(1,1),2);
RawTile rr(MapType::ArcGIS_Map,Point(1,1),3);
qDebug()<<pp.X()<<","<<pp.Y()<<qHash(r)<<qHash(rr)<<qHash(rt)<<qHash(rtt)<<qHash(pp)<<qHash(ppp);
Tile ttt;
qDebug()<<ttt.GetZoom();
qDebug()<<googleCopyright;
LKS94Projection lp;
PointLatLng point=lp.FromPixelToLatLng(210, 200,2);
Point pointp=lp.FromLatLngToPixel(point.Lat(),point.Lng(),1);
Size size=lp.TileSize();
qDebug()<<point.ToString()<<" "<<size.ToString()<<pointp.ToString();
      return a.exec();
}
