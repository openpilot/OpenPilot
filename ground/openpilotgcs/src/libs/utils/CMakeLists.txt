include_directories(..)

set(GCS_DATA_BASENAME
    share/openpilotgcs
)

add_definitions(-DGCS_DATA_BASENAME="${GCS_DATA_BASENAME}")

set(gcsutils_SRCS
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
    consoleprocess_unix.cpp
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

qt5_wrap_ui(gcsutils_SRCS
    filewizardpage.ui
    projectintropage.ui
    newclasswidget.ui
    submiteditorwidget.ui
    checkablemessagebox.ui
)

add_library(gcsutils ${gcsutils_SRCS})

qt5_use_modules(gcsutils Widgets Declarative Xml Svg OpenGL)
