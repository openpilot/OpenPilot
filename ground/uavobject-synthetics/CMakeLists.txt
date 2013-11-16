add_custom_command(
  OUTPUT "${PROJECT_BINARY_DIR}/uavobject-synthetics/flight/gcstelemetrystats.h"
  COMMAND mkdir -p ${PROJECT_BINARY_DIR}/uavobject-synthetics
  COMMAND cd ${PROJECT_BINARY_DIR}/uavobject-synthetics && ${PROJECT_BINARY_DIR}/ground/uavobjgenerator/uavobjgenerator ${PROJECT_SOURCE_DIR}/shared/uavobjectdefinition ${PROJECT_SOURCE_DIR} # ${PROJECT_BINARY_DIR}/uavobject-synthetics
)

add_custom_target(uavobject-synthetics
  DEPENDS "${PROJECT_BINARY_DIR}/uavobject-synthetics/flight/gcstelemetrystats.h"
)

add_dependencies(uavobject-synthetics uavobjgenerator)
