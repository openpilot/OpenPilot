#ifndef ALLLAYERSOFTYPE_H
#define ALLLAYERSOFTYPE_H

#include "maptype.h"
#include <QList>
#include <QVector>
class AllLayersOfType
{
public:
    AllLayersOfType();
    QVector<MapType::Types> GetAllLayersOfType(const MapType::Types &type);
};

#endif // ALLLAYERSOFTYPE_H
