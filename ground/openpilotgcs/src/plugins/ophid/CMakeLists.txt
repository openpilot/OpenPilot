include_directories(inc)
include_directories(/usr/include/libusb-1.0)

add_definitions(-DOPHID_LIBRARY)

set(opHID_SRCS
    src/ophid_plugin.cpp
    src/ophid.cpp
    src/ophid_usbsignal.cpp
    src/ophid_hidapi.cpp
    src/ophid_usbmon_linux.cpp
    hidapi/libusb/hid.c
    inc/ophid.h
    inc/ophid_usbmon.h
    inc/ophid_usbsignal.h
    inc/ophid_plugin.h
    inc/ophid_hidapi.h
)

add_library(opHID ${opHID_SRCS})

qt5_use_modules(opHID Core Widgets)

target_link_libraries(opHID usb udev usb-1.0 rt pthread)

configure_file(opHID.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)


