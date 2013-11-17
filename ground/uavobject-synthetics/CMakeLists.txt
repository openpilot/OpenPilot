include(uavobject.cmake)

add_custom_target (uavobject-synthetics
  DEPENDS ${uavobjgcs_SRCS}
)

add_dependencies(uavobject-synthetics uavobjgenerator)
