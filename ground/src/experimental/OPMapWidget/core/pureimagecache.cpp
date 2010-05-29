#include "pureimagecache.h"

qlonglong PureImageCache::ConnCounter=0;

PureImageCache::PureImageCache()
{
    gtilecache=QDir::currentPath()+QDir::separator()+"mapscache"+QDir::separator();
}

void PureImageCache::setGtileCache(const QString &value)
{
    gtilecache=value;
}

QString PureImageCache::GtileCache()
{
    return gtilecache;
}


bool PureImageCache::CreateEmptyDB(const QString &file)
{
    qDebug()<<"Create database at:"<<file;
    QFileInfo File(file);
    QDir dir=File.absoluteDir();
    QString path=dir.absolutePath();
    QString filename=File.fileName();
    if(File.exists())   QFile(filename).remove();
    if(!dir.exists())
    {
        qDebug()<<"CreateEmptyDB: Cache path doesn't exist, try to create";
        if(!dir.mkpath(path))
        {
            qDebug()<<"CreateEmptyDB: Could not create path";
            return false;
        }
    }
    QSqlDatabase db;

    db = QSqlDatabase::addDatabase("QSQLITE",QLatin1String("CreateConn"));
    db.setDatabaseName(file);
    if (!db.open())
    {
        qDebug()<<"CreateEmptyDB: Unable to create database";

        return false;
    }
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS Tiles (id INTEGER NOT NULL PRIMARY KEY, X INTEGER NOT NULL, Y INTEGER NOT NULL, Zoom INTEGER NOT NULL, Type INTEGER NOT NULL)");
    if(query.numRowsAffected()==-1)
    {
        qDebug()<<"CreateEmptyDB: "<<query.lastError().driverText();
        db.close();
        return false;
    }
    query.exec("CREATE TABLE IF NOT EXISTS TilesData (id INTEGER NOT NULL PRIMARY KEY CONSTRAINT fk_Tiles_id REFERENCES Tiles(id) ON DELETE CASCADE, Tile BLOB NULL)");
    if(query.numRowsAffected()==-1)
    {
        qDebug()<<"CreateEmptyDB: "<<query.lastError().driverText();
        db.close();
        return false;
    }
    query.exec(           
            "CREATE TRIGGER fki_TilesData_id_Tiles_id "
            "BEFORE INSERT ON [TilesData] "
            "FOR EACH ROW BEGIN "
            "SELECT RAISE(ROLLBACK, 'insert on table TilesData violates foreign key constraint fki_TilesData_id_Tiles_id') "
            "WHERE (SELECT id FROM Tiles WHERE id = NEW.id) IS NULL; "
            "END");
    if(query.numRowsAffected()==-1)
    {
        qDebug()<<"CreateEmptyDB: "<<query.lastError().driverText();
        db.close();
        return false;
    }
    query.exec(
            "CREATE TRIGGER fku_TilesData_id_Tiles_id "
            "BEFORE UPDATE ON [TilesData] "
            "FOR EACH ROW BEGIN "
            "SELECT RAISE(ROLLBACK, 'update on table TilesData violates foreign key constraint fku_TilesData_id_Tiles_id') "
            "WHERE (SELECT id FROM Tiles WHERE id = NEW.id) IS NULL; "
            "END");
    if(query.numRowsAffected()==-1)
    {
        qDebug()<<"CreateEmptyDB: "<<query.lastError().driverText();
        db.close();
        return false;
    }
    query.exec(
            "CREATE TRIGGER fkdc_TilesData_id_Tiles_id "
            "BEFORE DELETE ON Tiles "
            "FOR EACH ROW BEGIN "
            "DELETE FROM TilesData WHERE TilesData.id = OLD.id; "
            "END");
    if(query.numRowsAffected()==-1)
    {
        qDebug()<<"CreateEmptyDB: "<<query.lastError().driverText();
        db.close();
        return false;
    }
    db.close();
    return true;
}
bool PureImageCache::PutImageToCache(const QByteArray &tile, const MapType::Types &type,const Point &pos,const int &zoom)
{
    qDebug()<<"PutImageToCache Start:";//<<pos;
    bool ret=true;
    QDir d;
    QString dir=gtilecache;
    qDebug()<<"PutImageToCache Cache dir="<<dir;
    qDebug()<<"PutImageToCache Cache dir="<<dir<<" Try to PUT:"<<pos.ToString();
    if(!d.exists(dir))
    {
        d.mkdir(dir);
        qDebug()<<"Create Cache directory";
    }
    {
        QString db=dir+"Data.qmdb";
        if(!QFileInfo(db).exists())
        {
            qDebug()<<"Try to create EmptyDB";
            ret=CreateEmptyDB(db);
        }
        if(ret)
        {
            QSqlDatabase cn;
            Mcounter.lock();
            qlonglong id=++ConnCounter;
            Mcounter.unlock();
            cn = QSqlDatabase::addDatabase("QSQLITE",QString::number(id));

            cn.setDatabaseName(db);
            if(cn.open())
            {
                {
                    QSqlQuery query(cn);
                    query.prepare("INSERT INTO Tiles(X, Y, Zoom, Type) VALUES(?, ?, ?, ?)");
                    query.addBindValue(pos.X());
                    query.addBindValue(pos.Y());
                    query.addBindValue(zoom);
                    query.addBindValue((int)type);
                    query.exec();
                }
                {
                    QSqlQuery query(cn);
                    query.prepare("INSERT INTO TilesData(id, Tile) VALUES((SELECT last_insert_rowid()), ?)");
                    query.addBindValue(tile);
                    query.exec();
                }
                cn.close();
            }
            else return false;
        }
        else
        {
            qDebug()<<"PutImageToCache Could not create DB";
            return false;
        }
    }
    return true;
}
QByteArray PureImageCache::GetImageFromCache(MapType::Types type, Point pos, int zoom)
{
    bool ret=true;
    QByteArray ar;
    QString dir=gtilecache;
    qDebug()<<"Cache dir="<<dir<<" Try to GET:"<<pos.X()+","+pos.Y();

    {
        QString db=dir+"Data.qmdb";
        ret=QFileInfo(db).exists();
        if(ret)
        {
            QSqlDatabase cn;
            Mcounter.lock();
            qlonglong id=++ConnCounter;
            Mcounter.unlock();
            cn = QSqlDatabase::addDatabase("QSQLITE",QString::number(id));
            cn.setDatabaseName(db);
            if(cn.open())
            {
                {
                    QSqlQuery query(cn);
                    query.exec(QString("SELECT Tile FROM TilesData WHERE id = (SELECT id FROM Tiles WHERE X=%1 AND Y=%2 AND Zoom=%3 AND Type=%4)").arg(pos.X()).arg(pos.Y()).arg(zoom).arg((int) type));
                    query.next();
                    ar=query.value(0).toByteArray();
                }

                cn.close();
            }
        }
    }
    return ar;
}
// PureImageCache::ExportMapDataToDB("C:/Users/Xapo/Documents/mapcontrol/debug/mapscache/data.qmdb","C:/Users/Xapo/Documents/mapcontrol/debug/mapscache/data2.qmdb");
bool PureImageCache::ExportMapDataToDB(QString sourceFile, QString destFile)
{
    bool ret=true;
    QList<long> add;
    if(!QFileInfo(destFile).exists())
    {
        qDebug()<<"Try to create EmptyDB";
        ret=CreateEmptyDB(destFile);
    }
    if(!ret) return false;
    QSqlDatabase ca = QSqlDatabase::addDatabase("QSQLITE","ca");
    ca.setDatabaseName(sourceFile);

    if(ca.open())
    {
        QSqlDatabase cb = QSqlDatabase::addDatabase("QSQLITE","cb");
        cb.setDatabaseName(destFile);
        if(cb.open())
        {
            QSqlQuery queryb(cb);
            queryb.exec(QString("ATTACH DATABASE \"%1\" AS Source").arg(sourceFile));
            QSqlQuery querya(ca);
            querya.exec("SELECT id, X, Y, Zoom, Type FROM Tiles");
            while(querya.next())
            {
                long id=querya.value(0).toLongLong();
                queryb.exec(QString("SELECT id FROM Tiles WHERE X=%1 AND Y=%2 AND Zoom=%3 AND Type=%4;").arg(querya.value(1).toLongLong()).arg(querya.value(2).toLongLong()).arg(querya.value(3).toLongLong()).arg(querya.value(4).toLongLong()));
                if(!queryb.next())
                {
                    add.append(id);
                }

            }
            long f;
            foreach(f,add)
            {
                queryb.exec(QString("INSERT INTO Tiles(X, Y, Zoom, Type) SELECT X, Y, Zoom, Type FROM Source.Tiles WHERE id=%1").arg(f));
                queryb.exec(QString("INSERT INTO TilesData(id, Tile) Values((SELECT last_insert_rowid()), (SELECT Tile FROM Source.TilesData WHERE id=%1))").arg(f));
            }
            add.clear();
            ca.close();
            cb.close();

        }
        else return false;
    }
    else return false;

    return true;

}

