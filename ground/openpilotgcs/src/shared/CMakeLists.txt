include_directories(qtlockedfile)

if(WIN32)
    set(platform_specific_SRCS
        qtlockedfile/qtlockedfile_win.cpp
    )
else()
    set(platform_specific_SRCS
        qtlockedfile/qtlockedfile_unix.cpp
    )
endif()

set(sharedtools_SRCS
    ${platform_specific_SRCS}
    qtlockedfile/qtlockedfile.cpp
    qtsingleapplication/qtsingleapplication.cpp
    qtsingleapplication/qtlocalpeer.cpp
)

add_library(sharedtools ${sharedtools_SRCS})

qt5_use_modules(sharedtools Widgets Network)
