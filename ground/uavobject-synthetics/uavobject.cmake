file(GLOB uavobjgcs_XMLS ${PROJECT_SOURCE_DIR}/shared/uavobjectdefinition/*.xml)

foreach(uavobjxml ${uavobjgcs_XMLS})
    string(REPLACE "${PROJECT_SOURCE_DIR}/shared/uavobjectdefinition/" "${PROJECT_BINARY_DIR}/uavobject-synthetics/gcs/" uavobjtmp ${uavobjxml})
    string(REPLACE ".xml" ".h" uavobjhdr ${uavobjtmp})
    string(REPLACE ".xml" ".cpp" uavobjsrc ${uavobjtmp})
    set(uavobjgcs_HDRS ${uavobjgcs_HDRS} ${uavobjhdr})
    set(uavobjgcs_SRCS ${uavobjgcs_SRCS} ${uavobjsrc})
endforeach(uavobjxml)

set(uavobjgcs_SRCS ${uavobjgcs_SRCS} ${PROJECT_BINARY_DIR}/uavobject-synthetics/gcs/uavobjectsinit.cpp) 

add_custom_command (
    OUTPUT ${uavobjgcs_SRCS}
    COMMAND mkdir -p ${PROJECT_BINARY_DIR}/uavobject-synthetics
    COMMAND cd ${PROJECT_BINARY_DIR}/uavobject-synthetics && ${PROJECT_BINARY_DIR}/ground/uavobjgenerator/uavobjgenerator ${PROJECT_SOURCE_DIR}/shared/uavobjectdefinition ${PROJECT_SOURCE_DIR} # ${PROJECT_BINARY_DIR}/uavobject-synthetics
)
