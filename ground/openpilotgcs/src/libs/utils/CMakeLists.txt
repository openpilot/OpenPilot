include_directories(..)

set(GCS_DATA_BASENAME
    share/openpilotgcs
)

add_definitions(-DQTCREATOR_UTILS_LIB)
add_definitions(-DGCS_DATA_BASENAME="${GCS_DATA_BASENAME}")

if(WIN32)
    set(platform_specific_SRCS
        abstractprocess_win.cpp
        consoleprocess_win.cpp
        winutils.cpp
    )
else()
    set(platform_specific_SRCS
        consoleprocess_unix.cpp
    )
endif()
    
set(utils_SRCS
    reloadpromptutils.cpp
    settingsutils.cpp
    filesearch.cpp
    pathchooser.cpp
    pathlisteditor.cpp
    filewizardpage.cpp
    filewizarddialog.cpp
    projectintropage.cpp
    basevalidatinglineedit.cpp
    filenamevalidatinglineedit.cpp
    projectnamevalidatinglineedit.cpp
    codegeneration.cpp
    newclasswidget.cpp
    classnamevalidatinglineedit.cpp
    linecolumnlabel.cpp
    fancylineedit.cpp
    qtcolorbutton.cpp
    savedaction.cpp
    submiteditorwidget.cpp
    synchronousprocess.cpp
    submitfieldwidget.cpp
    consoleprocess.cpp
    ${platform_specific_SRCS}
    uncommentselection.cpp
    parameteraction.cpp
    treewidgetcolumnstretcher.cpp
    checkablemessagebox.cpp
    styledbar.cpp
    stylehelper.cpp
    welcomemodetreewidget.cpp
    iwelcomepage.cpp
    fancymainwindow.cpp
    detailsbutton.cpp
    detailswidget.cpp
    coordinateconversions.cpp
    pathutils.cpp
    worldmagmodel.cpp
    homelocationutil.cpp
    mytabbedstackwidget.cpp
    mytabwidget.cpp
    cachedsvgitem.cpp
    svgimageprovider.cpp
    hostosinfo.cpp
    xmlconfig.cpp
)

qt5_wrap_ui(utils_SRCS
    filewizardpage.ui
    projectintropage.ui
    newclasswidget.ui
    submiteditorwidget.ui
    checkablemessagebox.ui
)

add_library(utils ${utils_SRCS})

if(WIN32)
    target_link_libraries(utils opengl32)
endif()

qt5_use_modules(utils Core Widgets Declarative Xml Svg OpenGL Network)
