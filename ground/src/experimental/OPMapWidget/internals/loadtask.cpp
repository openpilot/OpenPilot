#include "loadtask.h"

bool operator==(LoadTask const& lhs,LoadTask const& rhs)
{
    return ((lhs.Pos==rhs.Pos)&&(lhs.Zoom==rhs.Zoom));
}
