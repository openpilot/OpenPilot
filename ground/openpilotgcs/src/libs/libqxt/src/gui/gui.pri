INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS  += qxtbasespinbox.h
HEADERS  += qxtcheckcombobox.h
HEADERS  += qxtcheckcombobox_p.h
HEADERS  += qxtconfigdialog.h
HEADERS  += qxtconfigdialog_p.h
HEADERS  += qxtconfigwidget.h
HEADERS  += qxtconfigwidget_p.h
HEADERS  += qxtconfirmationmessage.h
HEADERS  += qxtcountrycombobox.h
HEADERS  += qxtcountrycombobox_p.h
HEADERS  += qxtcountrymodel.h
HEADERS  += qxtcountrymodel_p.h
HEADERS  += qxtcrumbview.h
HEADERS  += qxtcrumbview_p.h
HEADERS  += qxtflowview.h
HEADERS  += qxtflowview_p.h
HEADERS  += qxtgui.h
HEADERS  += qxtgroupbox.h
HEADERS  += qxtheaderview.h
HEADERS  += qxtitemdelegate.h
HEADERS  += qxtitemdelegate_p.h
HEADERS  += qxtitemeditorcreator.h
HEADERS  += qxtitemeditorcreatorbase.h
HEADERS  += qxtlabel.h
HEADERS  += qxtletterboxwidget.h
HEADERS  += qxtletterboxwidget_p.h
HEADERS  += qxtlineedit.h
HEADERS  += qxtlistwidget.h
HEADERS  += qxtlistwidget_p.h
HEADERS  += qxtlistwidgetitem.h
HEADERS  += qxtlanguagecombobox.h
HEADERS  += qxtlanguagecombobox_p.h
HEADERS  += qxtnativeeventfilter.h
HEADERS  += qxtprogresslabel.h
HEADERS  += qxtproxystyle.h
HEADERS  += qxtpushbutton.h
HEADERS  += qxtspanslider.h
HEADERS  += qxtspanslider_p.h
HEADERS  += qxtstandarditemeditorcreator.h
HEADERS  += qxtstars.h
HEADERS  += qxtstringspinbox.h
HEADERS  += qxtstringvalidator.h
HEADERS  += qxtstringvalidator_p.h
HEADERS  += qxttablewidget.h
HEADERS  += qxttablewidget_p.h
HEADERS  += qxttablewidgetitem.h
HEADERS  += qxttabwidget.h
HEADERS  += qxttabwidget_p.h
HEADERS  += qxttooltip.h
HEADERS  += qxttooltip_p.h
HEADERS  += qxttreewidget.h
HEADERS  += qxttreewidget_p.h
HEADERS  += qxttreewidgetitem.h

HEADERS  += qxtscheduleheaderwidget.h
HEADERS  += qxtscheduleitemdelegate.h
HEADERS  += qxtscheduleview.h
HEADERS  += qxtscheduleviewheadermodel_p.h
HEADERS  += qxtscheduleview_p.h
HEADERS  += qxtstyleoptionscheduleviewitem.h

HEADERS  += qxtsortfilterproxymodel.h
HEADERS  += qxtfilterdialog.h
HEADERS  += qxtfilterdialog_p.h
HEADERS  += qxtlookuplineedit.h
HEADERS  += qxtlookuplineedit_p.h

SOURCES  += qxtbasespinbox.cpp
SOURCES  += qxtcheckcombobox.cpp
SOURCES  += qxtconfigdialog.cpp
SOURCES  += qxtconfigwidget.cpp
SOURCES  += qxtconfirmationmessage.cpp
SOURCES  += qxtcountrymodel.cpp
SOURCES  += qxtcountrycombobox.cpp
SOURCES  += qxtcrumbview.cpp
SOURCES  += qxtflowview.cpp
SOURCES  += qxtflowview_p.cpp
SOURCES  += qxtgroupbox.cpp
SOURCES  += qxtheaderview.cpp
SOURCES  += qxtitemdelegate.cpp
SOURCES  += qxtlabel.cpp
SOURCES  += qxtletterboxwidget.cpp
SOURCES  += qxtlineedit.cpp
SOURCES  += qxtlistwidget.cpp
SOURCES  += qxtlistwidgetitem.cpp
SOURCES  += qxtlanguagecombobox.cpp
SOURCES  += qxtprogresslabel.cpp
SOURCES  += qxtproxystyle.cpp
SOURCES  += qxtpushbutton.cpp
SOURCES  += qxtspanslider.cpp
SOURCES  += qxtstars.cpp
SOURCES  += qxtstringspinbox.cpp
SOURCES  += qxtstringvalidator.cpp
SOURCES  += qxttablewidget.cpp
SOURCES  += qxttablewidgetitem.cpp
SOURCES  += qxttabwidget.cpp
SOURCES  += qxttooltip.cpp
SOURCES  += qxttreewidget.cpp
SOURCES  += qxttreewidgetitem.cpp

SOURCES  += qxtscheduleitemdelegate.cpp
SOURCES  += qxtscheduleview.cpp
SOURCES  += qxtscheduleview_p.cpp
SOURCES  += qxtscheduleviewheadermodel_p.cpp
SOURCES  += qxtstyleoptionscheduleviewitem.cpp
SOURCES  += qxtscheduleheaderwidget.cpp

SOURCES  += qxtsortfilterproxymodel.cpp
SOURCES  += qxtfilterdialog.cpp
SOURCES  += qxtlookuplineedit.cpp

!qws:!symbian {
    # QxtApplication, QxtGlobalShortcut, QxtScreen and
    # QxtWindowSystem are disabled for QWS/Symbian pending implementation
    HEADERS  += qxtapplication.h
    HEADERS  += qxtapplication_p.h
    HEADERS  += qxtglobalshortcut.h
    HEADERS  += qxtglobalshortcut_p.h
    HEADERS  += qxtscreen.h
    HEADERS  += qxtscreen_p.h
    HEADERS  += qxtwindowsystem.h

    SOURCES  += qxtapplication.cpp
    SOURCES  += qxtglobalshortcut.cpp
    SOURCES  += qxtscreen.cpp
    SOURCES  += qxtwindowsystem.cpp

    unix:!macx {
        SOURCES += qxtapplication_x11.cpp
        SOURCES += qxtglobalshortcut_x11.cpp
        SOURCES += qxtscreen_x11.cpp
        SOURCES += qxtwindowsystem_x11.cpp
    }
    macx {
        SOURCES += qxtapplication_mac.cpp
        SOURCES += qxtglobalshortcut_mac.cpp
        SOURCES -= qxtscreen.cpp qxtwindowsystem.cpp
        HEADERS -= qxtscreen.h qxtwindowsystem.h
    }
    win32 {
        SOURCES += qxtapplication_win.cpp
        SOURCES += qxtglobalshortcut_win.cpp
        SOURCES += qxtscreen_win.cpp
        SOURCES += qxtwindowsystem_win.cpp
    }
}

RESOURCES += resources.qrc

