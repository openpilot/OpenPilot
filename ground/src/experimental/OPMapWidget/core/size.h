#ifndef SIZE_H
#define SIZE_H

#include "point.h"
#include <QString>
#include <QHash>
struct Size
{

    Size();
    Size(Point pt){width=pt.X(); height=pt.Y();};
    Size(int Width,int Height){width=Width; height=Height;};
    friend uint qHash(Size const& size);
  //  friend bool operator==(Size const& lhs,Size const& rhs);
    Size operator-(const Size &sz1){return Size(width-sz1.width,height-sz1.height);}
    Size operator+(const Size &sz1){return Size(sz1.width+width,sz1.height+height);}

    int GetHashCode(){return width^height;}
    uint qHash(Size const& rect){return width^height;}
    QString ToString(){return "With="+QString::number(width)+" ,Height="+QString::number(height);}
    int Width()const {return width;}
    int Height()const {return height;}
    void SetWidth(int const& value){width=value;}
    void SetHeight(int const& value){height=value;}
private:
    int width;
    int height;
    Point p;
};
//bool operator==(Size const& lhs,Size const& rhs){return (lhs.width==rhs.width && lhs.height==rhs.height);}

#endif // SIZE_H
