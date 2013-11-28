FIND_PATH(HID_INCLUDE_DIR hidsdi.h
    $ENV{HIDDIR}
    ${CMAKE_BINARY_DIR}/tools/hid
    ${CMAKE_BINARY_DIR}/tools/qt-5.1.1/Tools/mingw48_32/i686-w64-mingw32/include  
)

FIND_LIBRARY(HID_LIB 
    NAMES hid
    PATHS
    $ENV{HIDDIR}
    ${CMAKE_BINARY_DIR}/tools/hid
    ${CMAKE_BINARY_DIR}/tools/qt-5.1.1/Tools/mingw48_32/lib    
)
