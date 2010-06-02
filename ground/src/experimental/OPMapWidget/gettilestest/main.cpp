#include <QApplication>

#include "opmaps.h"
#include <QDebug>
#include "../core/pureimagecache.h"
#include "QPixmap"
#include "QPicture"
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
    PureImageCache* p=new PureImageCache();
    QPixmap pixmap;
    //Tile Polling test
    QTime t;
    t.start();
    OPMaps::Instance()->setUseMemoryCache(false);
    OPMaps::Instance()->setAccessMode(AccessMode::ServerOnly);
    pixmap=PureImageProxy::FromStream(OPMaps::Instance()->GetImageFrom(MapType::GoogleSatellite,Point(1,0),1));
    OPMaps::Instance()->GetImageFrom(MapType::GoogleSatellite,Point(0,1),1);
    OPMaps::Instance()->GetImageFrom(MapType::GoogleSatellite,Point(1,1),1);
    qDebug()<<"elapsed="<<t.elapsed();
    QLabel label;
    label.setPixmap(pixmap);
    label.show();
    //Geocoding Test
    GeoCoderStatusCode::Types status;
    qDebug()<<"Lisbon Coordinates:"<<OPMaps::Instance()->GetLatLngFromGeodecoder("lisbon",status).ToString();

    return a.exec();
}
