set(Aggregation_SRCS
    Aggregate.cpp
)

add_library(Aggregation ${Aggregation_SRCS})

qt5_use_modules(Aggregation Core)

add_definitions(-DAGGREGATION_LIBRARY)

set_target_properties(Aggregation PROPERTIES 
    VERSION ${GCSLIBS_VERSION_STRING} 
    SOVERSION ${GCSLIBS_VERSION_MAJOR}
)     
