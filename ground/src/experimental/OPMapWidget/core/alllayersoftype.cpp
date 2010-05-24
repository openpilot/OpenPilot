#include "alllayersoftype.h"



AllLayersOfType::AllLayersOfType()
{

}
QVector<MapType::Types> AllLayersOfType::GetAllLayersOfType(const MapType::Types &type)
{
    QVector<MapType::Types> types;
    {
        switch(type)
        {
        case MapType::GoogleHybrid:
            {

                types.append(MapType::GoogleSatellite);
                types.append(MapType::GoogleLabels);
            }
            break;

        case MapType::GoogleHybridChina:
            {
                types.append(MapType::GoogleSatelliteChina);
                types.append(MapType::GoogleLabelsChina);
            }
            break;

        case MapType::GoogleHybridKorea:
            {
                types.append(MapType::GoogleSatelliteKorea);
                types.append(MapType::GoogleLabelsKorea);
            }
            break;

        case MapType::YahooHybrid:
            {
                types.append(MapType::YahooSatellite);
                types.append(MapType::YahooLabels);
            }
            break;

        case MapType::ArcGIS_MapsLT_Map_Hybrid:
            {
                types.append(MapType::ArcGIS_MapsLT_OrtoFoto);
                types.append(MapType::ArcGIS_MapsLT_Map_Labels);
            }
            break;

        default:
            {
                types.append(type);
            }
            break;
        }
    }

    return types;

}
