#ifndef PUREIMAGE_H
#define PUREIMAGE_H

#include <QPixmap>
#include <QByteArray>

class PureImageProxy
{
public:
    PureImageProxy();
    static QPixmap FromStream(const QByteArray &array);
    static bool Save(const QByteArray &array,QPixmap &pic);
};

#endif // PUREIMAGE_H
