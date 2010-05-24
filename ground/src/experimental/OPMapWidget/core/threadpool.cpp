#include "threadpool.h"

threadpool::threadpool()
{
//    m.lock();
    threadpool::count=0;
    mm.lock();
    count2=0;
    mm.unlock();
//    qDebug()<<"Thread constructor";
//    m.unlock();
}
void threadpool::run()
{
    mm.lock();
    ++count2;
    int countt=count2;
    mm.unlock();
    QByteArray tmp;
    QImage im;
    qDebug()<<"Thread start";
    tmp=GMaps::Instance()->GetImageFrom(MapType::GoogleMap,Point(1,0),2);
   // im.load("C:/Users/Xapo/Pictures/x.jpg");
    //tmp=QPixmap::fromImage(im);
   // qDebug()<<"WWWWWWWW="<<im.width();
    mm.lock();
    pix.append(tmp);
   // ++threadpool::count;
    mm.unlock();
}
QByteArray threadpool::GetPic(int i)
{
    m.lock();
    QByteArray p=pix.last();
    m.unlock();
    return p;


}
