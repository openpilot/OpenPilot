#include "point.h"
#include "size.h"
Point::Point(int dw)
{
    this->x=(short)Point::LOWORD(dw);
    this->y=(short)Point::HIWORD(dw);
    empty=false;
}
Point::Point(Size sz)
{
    this->x=sz.Width();
    this->y=sz.Height();
    empty=false;
}
Point::Point(int x, int y)
{
    this->x=x;
    this->y=y;
    empty=false;
}
Point::Point():x(0),y(0),empty(true)
{}
uint qHash(Point const& point)
{
    return point.x^point.y;
}
bool operator==(Point const &lhs,Point const &rhs)
{
    return (lhs.x==rhs.x && lhs.y==rhs.y);
}
bool operator!=(Point const &lhs,Point const &rhs)
{
    return !(lhs==rhs);
}
int Point::HIWORD(int n)
{
    return (n >> 16) & 0xffff;
}

int Point::LOWORD(int n)
{
    return n & 0xffff;
}
Point Point::Empty=Point();

