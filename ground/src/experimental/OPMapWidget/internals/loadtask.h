#ifndef LOADTASK_H
#define LOADTASK_H

#include <QString>
#include "../core/point.h"

struct LoadTask
  {
     friend bool operator==(LoadTask const& lhs,LoadTask const& rhs);
  public:
    Point Pos;
    int Zoom;


    LoadTask(Point pos, int zoom)
     {
        Pos = pos;
        Zoom = zoom;
     }
    LoadTask()
    {
        Pos=Point(-1,-1);
        Zoom=-1;
    }
    bool HasValue()
    {
        return !(Zoom==-1);
    }

    QString ToString()const
     {
        return QString::number(Zoom) + " - " + Pos.ToString();
     }
  };
#endif // LOADTASK_H
