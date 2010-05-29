#ifndef POINT_H
#define POINT_H


#include <QString>
//#include "size.h"
struct Size;
struct Point
{
    friend uint qHash(Point const& point);
    friend bool operator==(Point const& lhs,Point const& rhs);
    friend bool operator!=(Point const& lhs,Point const& rhs);
public:

    Point();
    Point(int x,int y);
    Point(Size sz);
    Point(int dw);
    bool IsEmpty(){return empty;}
    int X()const{return this->x;}
    int Y()const{return this->y;}
    void SetX(const int &value){x=value;empty=false;}
    void SetY(const int &value){y=value;empty=false;}
    QString ToString()const{return "{"+QString::number(x)+","+QString::number(y)+"}";}

    static Point Empty;
    void Offset(const int &dx,const int &dy)
          {
             x += dx;
             y += dy;
          }
    void Offset(Point p)
          {
             Offset(p.x, p.y);
          }
    static int HIWORD(int n);
    static int LOWORD(int n);

private:
    int x;
    int y;
    bool empty;
};



#endif // POINT_H
