include(${PROJECT_SOURCE_DIR}/cmake/IncludeQt5.cmake)

set(uavobjgenerator_SRCS
    main.cpp
    uavobjectparser.cpp
    generators/generator_io.cpp
    generators/java/uavobjectgeneratorjava.cpp
    generators/flight/uavobjectgeneratorflight.cpp
    generators/gcs/uavobjectgeneratorgcs.cpp
    generators/matlab/uavobjectgeneratormatlab.cpp
    generators/python/uavobjectgeneratorpython.cpp
    generators/wireshark/uavobjectgeneratorwireshark.cpp
    generators/generator_common.cpp
)

add_executable(uavobjgenerator ${uavobjgenerator_SRCS})

qt5_use_modules(uavobjgenerator Core Xml)