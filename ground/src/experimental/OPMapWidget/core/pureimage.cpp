#include "pureimage.h"


PureImageProxy::PureImageProxy()
{

}

QPixmap PureImageProxy::FromStream(const QByteArray &array)
{
    return QPixmap::fromImage(QImage::fromData(array));
}
bool PureImageProxy::Save(const QByteArray &array, QPixmap &pic)
{
    pic=QPixmap::fromImage(QImage::fromData(array));
    return true;
}
