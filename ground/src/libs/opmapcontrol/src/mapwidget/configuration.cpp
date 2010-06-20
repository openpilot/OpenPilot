#include "configuration.h"

Configuration::Configuration()
{
    EmptytileBrush = Qt::cyan;
    MissingDataFont =QFont ("Times",10,QFont::Bold);
    EmptyTileText = "We are sorry, but we don't\nhave imagery at this zoom\nlevel for this region.";
    EmptyTileBorders = QPen(Qt::white);
    ScalePen = QPen(Qt::blue);
    SelectionPen = QPen(Qt::blue);
    DragButton = Qt::RightButton;
}
void Configuration::SetAccessMode(core::AccessMode::Types const& type)
{
    core::OPMaps::Instance()->setAccessMode(type);
}
core::AccessMode::Types Configuration::AccessMode()
{
    return core::OPMaps::Instance()->GetAccessMode();
}
void Configuration::SetLanguage(core::LanguageType::Types const& type)
{
    core::OPMaps::Instance()->setLanguage(type);
}
core::LanguageType::Types Configuration::Language()
{
    return core::OPMaps::Instance()->GetLanguage();
}
