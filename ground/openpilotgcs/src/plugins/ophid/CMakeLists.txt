include_directories(inc)
include_directories(/usr/include/libusb-1.0)

add_definitions(-DOPHID_LIBRARY)

if(WIN32)
    set(platform_specific_SRCS
        src/ophid_usbmon_win.cpp
        hidapi/windows/hid.c
    )
else()
    set(platform_specific_SRCS
        src/ophid_usbmon_linux.cpp
        hidapi/libusb/hid.c
    )
endif()

set(opHID_SRCS
    src/ophid_plugin.cpp
    src/ophid.cpp
    src/ophid_usbsignal.cpp
    src/ophid_hidapi.cpp
    ${platform_specific_SRCS}
    inc/ophid.h
    inc/ophid_usbmon.h
    inc/ophid_usbsignal.h
    inc/ophid_plugin.h
    inc/ophid_hidapi.h
)

add_library(opHID ${opHID_SRCS})

qt5_use_modules(opHID Core Widgets)

target_link_libraries(opHID Core ExtensionSystem)

if(WIN32)
    target_link_libraries(opHID hid setupapi ExtensionSystem)
else()
    target_link_libraries(opHID usb udev usb-1.0 rt pthread)
endif()

add_definitions(-DOPHID_LIBRARY)

configure_file(opHID.pluginspec ${PROJECT_BINARY_DIR}/ground/openpilotgcs/src/plugins/ COPYONLY)


