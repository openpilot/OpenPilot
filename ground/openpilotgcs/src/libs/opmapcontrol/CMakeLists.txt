include_directories(..)

set(opmapcontrol_SRCS
    src/internals/core.cpp
    src/internals/rectangle.cpp
    src/internals/tile.cpp
    src/internals/tilematrix.cpp
    src/internals/pureprojection.cpp
    src/internals/rectlatlng.cpp
    src/internals/sizelatlng.cpp
    src/internals/pointlatlng.cpp
    src/internals/loadtask.cpp
    src/internals/mousewheelzoomtype.cpp
    src/internals/projections/lks94projection.cpp
    src/internals/projections/mercatorprojection.cpp
    src/internals/projections/mercatorprojectionyandex.cpp
    src/internals/projections/platecarreeprojection.cpp
    src/internals/projections/platecarreeprojectionpergo.cpp
    src/core/opmaps.cpp
    src/core/pureimagecache.cpp
    src/core/pureimage.cpp
    src/core/rawtile.cpp
    src/core/memorycache.cpp
    src/core/cache.cpp
    src/core/languagetype.cpp
    src/core/providerstrings.cpp
    src/core/cacheitemqueue.cpp
    src/core/tilecachequeue.cpp
    src/core/alllayersoftype.cpp
    src/core/urlfactory.cpp
    src/core/placemark.cpp
    src/core/point.cpp
    src/core/size.cpp
    src/core/kibertilecache.cpp
    src/core/diagnostics.cpp
    src/mapwidget/mapgraphicitem.cpp
    src/mapwidget/opmapwidget.cpp
    src/mapwidget/configuration.cpp
    src/mapwidget/waypointitem.cpp
    src/mapwidget/uavitem.cpp
    src/mapwidget/gpsitem.cpp
    src/mapwidget/trailitem.cpp
    src/mapwidget/homeitem.cpp
    src/mapwidget/mapripform.cpp
    src/mapwidget/mapripper.cpp
    src/mapwidget/traillineitem.cpp
    src/mapwidget/waypointline.cpp
    src/mapwidget/waypointcircle.cpp
)

set(opmapcontrol_ITFS
    src/core/geodecoderstatus.h
    src/core/accessmode.h
    src/core/maptype.h
    src/mapwidget/uavtrailtype.h
)

qt5_wrap_ui(opmapcontrol_SRCS
    src/mapwidget/mapripform.ui
)

qt5_add_resources(opmapcontrol_SRCS
    src/mapwidget/mapresources.qrc
) 

add_library(opmapcontrol ${opmapcontrol_SRCS} ${opmapcontrol_ITFS})

qt5_use_modules(opmapcontrol Core Widgets Network Sql OpenGL Svg)

target_link_libraries(opmapcontrol utils)
