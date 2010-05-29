#ifndef PUREIMAGECACHE_H
#define PUREIMAGECACHE_H

#include <QSqlDatabase>
#include <QString>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QBuffer>
#include "maptype.h"
#include "point.h"
#include <QVariant>
#include "pureimage.h"
#include <QList>
#include <QMutex>



class PureImageCache
{

public:
    PureImageCache();
    static bool CreateEmptyDB(const QString &file);
    bool PutImageToCache(const QByteArray &tile,const MapType::Types &type,const Point &pos, const int &zoom);
    QByteArray GetImageFromCache(MapType::Types type, Point pos, int zoom);
    QString GtileCache();
    void setGtileCache(const QString &value);
    static bool ExportMapDataToDB(QString sourceFile, QString destFile);
private:
    QString gtilecache;
    QMutex Mcounter;
    static qlonglong ConnCounter;
};

#endif // PUREIMAGECACHE_H
