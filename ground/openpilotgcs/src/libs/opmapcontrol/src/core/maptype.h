/**
 ******************************************************************************
 *
 * @file       maptype.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   OPMapWidget
 * @{
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef MAPTYPE_H
#define MAPTYPE_H
#include <QMetaObject>
#include <QMetaEnum>
#include <QStringList>

namespace core {
class MapType : public QObject {
    Q_OBJECT Q_ENUMS(Types)
public:
    enum Types {
        GoogleMap            = 1,
        GoogleSatellite      = 4,
        GoogleLabels         = 8,
        GoogleTerrain        = 16,
        GoogleHybrid         = 20,

        GoogleMapChina       = 22,
        GoogleSatelliteChina = 24,
        GoogleLabelsChina    = 26,
        GoogleTerrainChina   = 28,
        GoogleHybridChina    = 29,

        OpenStreetMap        = 32,
        OpenStreetOsm        = 33,
        OpenStreetMapSurfer  = 34,
        OpenStreetMapSurferTerrain = 35,

        BingMap              = 444,
        BingSatellite        = 555,
        BingHybrid           = 666,

        ArcGIS_Map           = 777,
        ArcGIS_Satellite     = 788,
        ArcGIS_ShadedRelief  = 799,
        ArcGIS_Terrain       = 811,

        SigPacSpainMap       = 3001,

        GoogleMapKorea       = 4001,
        GoogleSatelliteKorea = 4002,
        GoogleLabelsKorea    = 4003,
        GoogleHybridKorea    = 4005,

        Statkart_Topo2       = 5500
    };
    static QString StrByType(Types const & value)
    {
        QMetaObject metaObject = MapType().staticMetaObject;
        QMetaEnum metaEnum     = metaObject.enumerator(metaObject.indexOfEnumerator("Types"));
        QString s = metaEnum.valueToKey(value);

        return s;
    }
    static Types TypeByStr(QString const & value)
    {
        QMetaObject metaObject = MapType().staticMetaObject;
        QMetaEnum metaEnum     = metaObject.enumerator(metaObject.indexOfEnumerator("Types"));
        Types s = (Types)metaEnum.keyToValue(value.toLatin1());

        return s;
    }
    static QStringList TypesList()
    {
        QStringList ret;
        QMetaObject metaObject = MapType().staticMetaObject;
        QMetaEnum metaEnum     = metaObject.enumerator(metaObject.indexOfEnumerator("Types"));

        for (int x = 0; x < metaEnum.keyCount(); ++x) {
            ret.append(metaEnum.key(x));
        }
        return ret;
    }
};
}
#endif // MAPTYPE_H
