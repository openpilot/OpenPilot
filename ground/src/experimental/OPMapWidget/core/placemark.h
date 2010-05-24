#ifndef PLACEMARK_H
#define PLACEMARK_H

#include <QString>

class Placemark
{
public:
    Placemark(const QString &address)
    {
       this->address = address;
    }
    QString Address(){return address;}
    int Accuracy(){return accuracy;}
    void SetAddress(const QString &adr){address=adr;}
    void SetAccuracy(const int &value){accuracy=value;}
private:

    QString address;
    int accuracy;
protected:


};
#endif // PLACEMARK_H
