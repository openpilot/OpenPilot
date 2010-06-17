#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QBrush>
#include <QPen>
#include <QString>
#include <QFont>
#include "../core/opmaps.h"
#include "../core/accessmode.h"
#include "../core/cache.h"
class Configuration
{
public:
    Configuration();
    QBrush EmptytileBrush;
    QString EmptyTileText;
    QPen EmptyTileBorders;
    QPen ScalePen;
    QPen SelectionPen;
    QFont MissingDataFont;

    void SetAccessMode(core::AccessMode::Types const& type);
    core::AccessMode::Types AccessMode();

    void SetLanguage(core::LanguageType::Types const& type);
    core::LanguageType::Types Language();

    void SetUseMemoryCache(bool const& value){core::OPMaps::Instance()->setUseMemoryCache(value);}
    bool UseMemoryCache(){return core::OPMaps::Instance()->UseMemoryCache();}

    void SetCacheLocation(QString const& dir){core::Cache::Instance()->setCacheLocation(dir);}
    QString CacheLocation(){return core::Cache::Instance()->CacheLocation();}
};

#endif // CONFIGURATION_H
