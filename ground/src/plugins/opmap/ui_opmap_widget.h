/********************************************************************************
** Form generated from reading UI file 'opmap_widget.ui'
**
** Created: Fri 2. Jul 21:37:47 2010
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OPMAP_WIDGET_H
#define UI_OPMAP_WIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QSplitter>
#include <QtGui/QToolButton>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OPMap_Widget
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *layoutWidgetToolBar;
    QFrame *frameToolBar;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *layoutWidget_8;
    QToolButton *toolButtonWaypointsTreeViewShowHide;
    QSpacerItem *horizontalSpacer;
    QComboBox *comboBoxFindPlace;
    QToolButton *toolButtonFindPlace;
    QSpacerItem *horizontalSpacer_5;
    QToolButton *toolButtonMapHome;
    QToolButton *toolButtonMapUAV;
    QSpacerItem *horizontalSpacer_6;
    QToolButton *toolButtonAddWaypoint;
    QToolButton *toolButtonWaypointEditor;
    QSpacerItem *horizontalSpacer_4;
    QToolButton *toolButtonZoomM;
    QToolButton *toolButtonZoomP;
    QSlider *horizontalSliderZoom;
    QSpacerItem *horizontalSpacer_2;
    QToolButton *toolButtonFlightControlsShowHide;
    QHBoxLayout *horizontalLayout_3;
    QSplitter *splitter;
    QTreeView *treeViewWaypoints;
    QWidget *mapWidget;
    QFrame *frameFlightControls;
    QVBoxLayout *verticalLayout_4;
    QToolButton *toolButtonHoldPosition;
    QToolButton *toolButtonHome;
    QToolButton *toolButtonPrevWaypoint;
    QToolButton *toolButtonNextWaypoint;
    QToolButton *toolButtonGo;
    QHBoxLayout *layoutWidgetStatusBar_2;
    QFrame *frameStatusBar;
    QHBoxLayout *horizontalLayout;
    QHBoxLayout *layoutWidgetStatusBar;
    QSpacerItem *horizontalSpacer_7;
    QLabel *labelUAVPos;
    QLabel *labelMapPos;
    QLabel *labelMousePos;
    QSpacerItem *horizontalSpacer_3;
    QProgressBar *progressBarMap;

    void setupUi(QWidget *OPMap_Widget)
    {
        if (OPMap_Widget->objectName().isEmpty())
            OPMap_Widget->setObjectName(QString::fromUtf8("OPMap_Widget"));
        OPMap_Widget->resize(704, 422);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(OPMap_Widget->sizePolicy().hasHeightForWidth());
        OPMap_Widget->setSizePolicy(sizePolicy);
        OPMap_Widget->setMouseTracking(false);
        OPMap_Widget->setStyleSheet(QString::fromUtf8(""));
        verticalLayout = new QVBoxLayout(OPMap_Widget);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        layoutWidgetToolBar = new QHBoxLayout();
        layoutWidgetToolBar->setSpacing(0);
        layoutWidgetToolBar->setObjectName(QString::fromUtf8("layoutWidgetToolBar"));
        layoutWidgetToolBar->setContentsMargins(0, -1, 0, -1);
        frameToolBar = new QFrame(OPMap_Widget);
        frameToolBar->setObjectName(QString::fromUtf8("frameToolBar"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(frameToolBar->sizePolicy().hasHeightForWidth());
        frameToolBar->setSizePolicy(sizePolicy1);
        frameToolBar->setMinimumSize(QSize(0, 0));
        frameToolBar->setAutoFillBackground(false);
        frameToolBar->setStyleSheet(QString::fromUtf8("QFrame {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(110, 110, 110, 255), stop:1 rgba(71, 71, 71, 255));\n"
"color: rgba(255, 255, 255, 70);\n"
"}\n"
"\n"
"QLabel { /* all label types */\n"
"/* background-color: rgba(255, 255, 255, 0); */\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(64, 64, 64, 255), stop:1 rgba(128, 128, 128, 255));\n"
"color: rgb(255, 255, 255);\n"
"border: 0px; \n"
"border-radius: 3px;\n"
"}\n"
"\n"
"QToolButton {	/* all types of tool button */ \n"
"background-color: rgba(255, 255, 255, 0);\n"
"color: rgb(255, 255, 255);\n"
"/*border-style: none;*/\n"
"border: 0px; \n"
"border-radius: 3px;\n"
"}\n"
"QToolButton:hover {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(255, 255, 255, 200), stop:1 rgba(180, 180, 180, 200));\n"
"color: rgb(255, 255, 255);\n"
"border: 0px; \n"
"border-radius: 3px;\n"
"}\n"
"QToolButton:pressed {\n"
"background-color: qlineargradient"
                        "(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(120, 120, 120, 255));\n"
"color: rgb(255, 255, 255);\n"
"border: 0px; \n"
"border-radius: 3px;\n"
"}\n"
"\n"
"QToolButton:checked {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(120, 120, 120, 255));\n"
"color: rgb(255, 255, 255);\n"
"border: 0px; \n"
"border-radius: 3px;\n"
"}\n"
"\n"
"QSlider::groove:horizontal {\n"
"border: none;\n"
"height: 4px;\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(80, 80, 80, 255));\n"
"margin: 2px 0;\n"
"}\n"
"QSlider::handle:horizontal {\n"
"border: 1px solid #5c5c5c;\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(255, 255, 255, 255), stop:1 rgba(128, 128, 128, 255));\n"
"width: 24px;\n"
"margin: -2px 0;\n"
"border-radius: 3px;\n"
"}\n"
"\n"
"QComboBox {\n"
"background-color: qlineargradient(spread:pad, x1:0."
                        "5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(120, 120, 120, 255));\n"
"color: rgb(255, 255, 255);\n"
"border: 1px solid black; \n"
"border-radius: 0px;\n"
"}\n"
"QComboBox::down-arrow:on {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(120, 120, 120, 255));\n"
"top: 1px; left: 1px;	/* move the arrow when the popup is open */\n"
"}\n"
"QComboBox::down-arrow {\n"
"     image: url(:/opmap/images/combobox_down_arrow.png);\n"
"}\n"
"QComboBox:drop-down {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(110, 110, 110, 255), stop:1 rgba(71, 71, 71, 255));\n"
"/*     subcontrol-origin: padding;\n"
"     subcontrol-position: top right;\n"
"     border-left-style: 1px solid white;\n"
"     border-top-right-radius: 1px;\n"
"     border-bottom-right-radius: 1px; */\n"
"}\n"
"QComboBox QAbstractItemView { /* the drop down list */\n"
"background-color: rgb(255, 255, 255);\n"
"color: rgb(0, 0,"
                        " 0);\n"
"border: 1px solid black;\n"
"selection-background-color: rgb(160, 160, 255);\n"
"border-radius: 2px;\n"
"}\n"
""));
        frameToolBar->setFrameShape(QFrame::NoFrame);
        frameToolBar->setFrameShadow(QFrame::Plain);
        horizontalLayout_2 = new QHBoxLayout(frameToolBar);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 3, 0, 3);
        layoutWidget_8 = new QHBoxLayout();
        layoutWidget_8->setSpacing(5);
        layoutWidget_8->setObjectName(QString::fromUtf8("layoutWidget_8"));
        layoutWidget_8->setContentsMargins(3, -1, 3, -1);
        toolButtonWaypointsTreeViewShowHide = new QToolButton(frameToolBar);
        toolButtonWaypointsTreeViewShowHide->setObjectName(QString::fromUtf8("toolButtonWaypointsTreeViewShowHide"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(toolButtonWaypointsTreeViewShowHide->sizePolicy().hasHeightForWidth());
        toolButtonWaypointsTreeViewShowHide->setSizePolicy(sizePolicy2);
        toolButtonWaypointsTreeViewShowHide->setStyleSheet(QString::fromUtf8(""));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/core/images/prev.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonWaypointsTreeViewShowHide->setIcon(icon);
        toolButtonWaypointsTreeViewShowHide->setIconSize(QSize(20, 20));
        toolButtonWaypointsTreeViewShowHide->setAutoRaise(false);

        layoutWidget_8->addWidget(toolButtonWaypointsTreeViewShowHide);

        horizontalSpacer = new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        layoutWidget_8->addItem(horizontalSpacer);

        comboBoxFindPlace = new QComboBox(frameToolBar);
        comboBoxFindPlace->setObjectName(QString::fromUtf8("comboBoxFindPlace"));
        sizePolicy2.setHeightForWidth(comboBoxFindPlace->sizePolicy().hasHeightForWidth());
        comboBoxFindPlace->setSizePolicy(sizePolicy2);
        comboBoxFindPlace->setMinimumSize(QSize(120, 0));
        comboBoxFindPlace->setStyleSheet(QString::fromUtf8(""));
        comboBoxFindPlace->setEditable(true);
        comboBoxFindPlace->setMaxVisibleItems(20);
        comboBoxFindPlace->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        comboBoxFindPlace->setFrame(false);

        layoutWidget_8->addWidget(comboBoxFindPlace);

        toolButtonFindPlace = new QToolButton(frameToolBar);
        toolButtonFindPlace->setObjectName(QString::fromUtf8("toolButtonFindPlace"));
        toolButtonFindPlace->setStyleSheet(QString::fromUtf8(""));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/core/images/find.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonFindPlace->setIcon(icon1);
        toolButtonFindPlace->setIconSize(QSize(20, 20));
        toolButtonFindPlace->setAutoRaise(false);
        toolButtonFindPlace->setArrowType(Qt::NoArrow);

        layoutWidget_8->addWidget(toolButtonFindPlace);

        horizontalSpacer_5 = new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        layoutWidget_8->addItem(horizontalSpacer_5);

        toolButtonMapHome = new QToolButton(frameToolBar);
        toolButtonMapHome->setObjectName(QString::fromUtf8("toolButtonMapHome"));
        toolButtonMapHome->setStyleSheet(QString::fromUtf8(""));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/opmap/images/gcs.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonMapHome->setIcon(icon2);
        toolButtonMapHome->setIconSize(QSize(20, 20));
        toolButtonMapHome->setAutoRepeat(true);
        toolButtonMapHome->setAutoRaise(false);

        layoutWidget_8->addWidget(toolButtonMapHome);

        toolButtonMapUAV = new QToolButton(frameToolBar);
        toolButtonMapUAV->setObjectName(QString::fromUtf8("toolButtonMapUAV"));
        toolButtonMapUAV->setStyleSheet(QString::fromUtf8(""));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/opmap/images/uav.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonMapUAV->setIcon(icon3);
        toolButtonMapUAV->setIconSize(QSize(20, 20));
        toolButtonMapUAV->setCheckable(true);
        toolButtonMapUAV->setAutoRepeat(true);
        toolButtonMapUAV->setAutoRaise(false);

        layoutWidget_8->addWidget(toolButtonMapUAV);

        horizontalSpacer_6 = new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        layoutWidget_8->addItem(horizontalSpacer_6);

        toolButtonAddWaypoint = new QToolButton(frameToolBar);
        toolButtonAddWaypoint->setObjectName(QString::fromUtf8("toolButtonAddWaypoint"));
        toolButtonAddWaypoint->setStyleSheet(QString::fromUtf8(""));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/opmap/images/waypoint.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonAddWaypoint->setIcon(icon4);
        toolButtonAddWaypoint->setIconSize(QSize(20, 20));
        toolButtonAddWaypoint->setAutoRepeat(true);
        toolButtonAddWaypoint->setAutoRaise(false);

        layoutWidget_8->addWidget(toolButtonAddWaypoint);

        toolButtonWaypointEditor = new QToolButton(frameToolBar);
        toolButtonWaypointEditor->setObjectName(QString::fromUtf8("toolButtonWaypointEditor"));
        toolButtonWaypointEditor->setStyleSheet(QString::fromUtf8(""));
        toolButtonWaypointEditor->setIcon(icon4);
        toolButtonWaypointEditor->setIconSize(QSize(20, 20));
        toolButtonWaypointEditor->setAutoRepeat(true);
        toolButtonWaypointEditor->setAutoRaise(false);

        layoutWidget_8->addWidget(toolButtonWaypointEditor);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        layoutWidget_8->addItem(horizontalSpacer_4);

        toolButtonZoomM = new QToolButton(frameToolBar);
        toolButtonZoomM->setObjectName(QString::fromUtf8("toolButtonZoomM"));
        toolButtonZoomM->setStyleSheet(QString::fromUtf8(""));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/opmap/images/minus.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonZoomM->setIcon(icon5);
        toolButtonZoomM->setIconSize(QSize(20, 20));
        toolButtonZoomM->setAutoRaise(false);

        layoutWidget_8->addWidget(toolButtonZoomM);

        toolButtonZoomP = new QToolButton(frameToolBar);
        toolButtonZoomP->setObjectName(QString::fromUtf8("toolButtonZoomP"));
        toolButtonZoomP->setStyleSheet(QString::fromUtf8(""));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/opmap/images/plus.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonZoomP->setIcon(icon6);
        toolButtonZoomP->setIconSize(QSize(20, 20));
        toolButtonZoomP->setAutoRaise(false);

        layoutWidget_8->addWidget(toolButtonZoomP);

        horizontalSliderZoom = new QSlider(frameToolBar);
        horizontalSliderZoom->setObjectName(QString::fromUtf8("horizontalSliderZoom"));
        sizePolicy2.setHeightForWidth(horizontalSliderZoom->sizePolicy().hasHeightForWidth());
        horizontalSliderZoom->setSizePolicy(sizePolicy2);
        horizontalSliderZoom->setMinimumSize(QSize(130, 0));
        horizontalSliderZoom->setMaximumSize(QSize(130, 16777215));
        horizontalSliderZoom->setCursor(QCursor(Qt::OpenHandCursor));
        horizontalSliderZoom->setAutoFillBackground(false);
        horizontalSliderZoom->setStyleSheet(QString::fromUtf8(""));
        horizontalSliderZoom->setMinimum(2);
        horizontalSliderZoom->setMaximum(19);
        horizontalSliderZoom->setPageStep(1);
        horizontalSliderZoom->setOrientation(Qt::Horizontal);
        horizontalSliderZoom->setInvertedControls(false);
        horizontalSliderZoom->setTickPosition(QSlider::NoTicks);
        horizontalSliderZoom->setTickInterval(2);

        layoutWidget_8->addWidget(horizontalSliderZoom);

        horizontalSpacer_2 = new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        layoutWidget_8->addItem(horizontalSpacer_2);

        toolButtonFlightControlsShowHide = new QToolButton(frameToolBar);
        toolButtonFlightControlsShowHide->setObjectName(QString::fromUtf8("toolButtonFlightControlsShowHide"));
        sizePolicy2.setHeightForWidth(toolButtonFlightControlsShowHide->sizePolicy().hasHeightForWidth());
        toolButtonFlightControlsShowHide->setSizePolicy(sizePolicy2);
        toolButtonFlightControlsShowHide->setStyleSheet(QString::fromUtf8(""));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/core/images/next.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonFlightControlsShowHide->setIcon(icon7);
        toolButtonFlightControlsShowHide->setIconSize(QSize(20, 20));
        toolButtonFlightControlsShowHide->setAutoRaise(false);

        layoutWidget_8->addWidget(toolButtonFlightControlsShowHide);


        horizontalLayout_2->addLayout(layoutWidget_8);


        layoutWidgetToolBar->addWidget(frameToolBar);


        verticalLayout->addLayout(layoutWidgetToolBar);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        splitter = new QSplitter(OPMap_Widget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setStyleSheet(QString::fromUtf8("QSplitter::handle {\n"
"/*	image: url(opmap/images/splitter.png); */\n"
"/*background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(255, 255, 255, 48), stop:1 rgba(0, 0, 0, 48));*/\n"
"	background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(71, 71, 71, 255), stop:0.514124 rgba(200, 200, 200, 255), stop:1 rgba(110, 110, 110, 255));\n"
"}\n"
"\n"
"QSplitter::handle:horizontal {\n"
"	height: 5px;\n"
"}\n"
"\n"
"QSplitter::handle:vertical {\n"
"	width: 5px;\n"
"}"));
        splitter->setFrameShape(QFrame::NoFrame);
        splitter->setFrameShadow(QFrame::Sunken);
        splitter->setLineWidth(1);
        splitter->setOrientation(Qt::Horizontal);
        splitter->setOpaqueResize(true);
        splitter->setHandleWidth(5);
        splitter->setChildrenCollapsible(false);
        treeViewWaypoints = new QTreeView(splitter);
        treeViewWaypoints->setObjectName(QString::fromUtf8("treeViewWaypoints"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(treeViewWaypoints->sizePolicy().hasHeightForWidth());
        treeViewWaypoints->setSizePolicy(sizePolicy3);
        treeViewWaypoints->setMinimumSize(QSize(100, 0));
        treeViewWaypoints->setMaximumSize(QSize(300, 16777215));
        treeViewWaypoints->setContextMenuPolicy(Qt::NoContextMenu);
        treeViewWaypoints->setStyleSheet(QString::fromUtf8("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(71, 71, 71, 255), stop:1 rgba(110, 110, 110, 255));\n"
"color: rgb(255, 255, 255);\n"
"/*\n"
"QTreeView::item {\n"
"border: 1px solid #d9d9d9;\n"
"border-top-color: transparent;\n"
"border-bottom-color: transparent;\n"
"} \n"
"*/\n"
"QTreeView::item:hover {\n"
"background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #e7effd, stop: 1 #cbdaf1);\n"
"border: 1px solid #bfcde4;\n"
"} \n"
"\n"
"QTreeView::branch:has-children:!has-siblings:closed, QTreeView::branch:closed:has-children:has-siblings {\n"
" border-image: none;\n"
" image: url(branch-closed.png);\n"
"}\n"
"QTreeView::branch:open:has-children:!has-siblings, QTreeView::branch:open:has-children:has-siblings {\n"
" border-image: none;\n"
" image: url(branch-open.png);\n"
"}"));
        treeViewWaypoints->setFrameShape(QFrame::NoFrame);
        treeViewWaypoints->setFrameShadow(QFrame::Plain);
        treeViewWaypoints->setTabKeyNavigation(true);
        treeViewWaypoints->setSelectionBehavior(QAbstractItemView::SelectItems);
        treeViewWaypoints->setAnimated(true);
        treeViewWaypoints->setHeaderHidden(true);
        splitter->addWidget(treeViewWaypoints);
        mapWidget = new QWidget(splitter);
        mapWidget->setObjectName(QString::fromUtf8("mapWidget"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(mapWidget->sizePolicy().hasHeightForWidth());
        mapWidget->setSizePolicy(sizePolicy4);
        mapWidget->setMinimumSize(QSize(100, 0));
        mapWidget->setMouseTracking(false);
        mapWidget->setAcceptDrops(true);
        mapWidget->setStyleSheet(QString::fromUtf8("\n"
"/* background-color: black; */\n"
"border: 1px solid black;\n"
"\n"
"QLabel { /* all label types */\n"
"/* background-color: rgba(255, 255, 255, 0); */\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(64, 64, 64, 255), stop:1 rgba(128, 128, 128, 255));\n"
"color: rgb(255, 255, 255);\n"
"}\n"
"\n"
"QToolButton {	/* all types of tool button */ \n"
"background-color: rgba(255, 255, 255, 0);\n"
"color: rgb(255, 255, 255);\n"
"/*border-style: none;*/\n"
"border: 0px; \n"
"}\n"
"QToolButton:hover {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(255, 160, 0, 255), stop:1 rgba(160, 100, 0, 255));\n"
"color: rgb(255, 255, 255);\n"
"}\n"
"QToolButton:pressed {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(120, 120, 120, 255));\n"
"color: rgb(255, 255, 255);\n"
"}\n"
"\n"
"QToolButton:checked {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:"
                        "0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(120, 120, 120, 255));\n"
"color: rgb(255, 255, 255);\n"
"}\n"
"\n"
"QSlider::groove:horizontal {\n"
"border: none;\n"
"height: 4px;\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(80, 80, 80, 255));\n"
"margin: 2px 0;\n"
"}\n"
"QSlider::handle:horizontal {\n"
"border: 1px solid #5c5c5c;\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(255, 255, 255, 255), stop:1 rgba(128, 128, 128, 255));\n"
"width: 24px;\n"
"margin: -2px 0;\n"
"border-radius: 3px;\n"
"}\n"
"\n"
"QComboBox {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(120, 120, 120, 255));\n"
"color: rgb(255, 255, 255);\n"
"}\n"
"QComboBox:hover {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(120, 120, 120, 255));\n"
"	background-color: rgb(197, 138,"
                        " 0);\n"
"}\n"
"QComboBox::down-arrow {\n"
"     image: url(:/opmap/images/combobox_down_arrow.png);\n"
"}\n"
"QComboBox:drop-down {\n"
"     subcontrol-origin: padding;\n"
"     subcontrol-position: top right;\n"
"     border-left-style: none;\n"
"     border-top-right-radius: 1px;\n"
"     border-bottom-right-radius: 1px;\n"
"}\n"
""));
        splitter->addWidget(mapWidget);

        horizontalLayout_3->addWidget(splitter);

        frameFlightControls = new QFrame(OPMap_Widget);
        frameFlightControls->setObjectName(QString::fromUtf8("frameFlightControls"));
        QSizePolicy sizePolicy5(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(frameFlightControls->sizePolicy().hasHeightForWidth());
        frameFlightControls->setSizePolicy(sizePolicy5);
        frameFlightControls->setMinimumSize(QSize(80, 50));
        frameFlightControls->setMaximumSize(QSize(80, 16777215));
        frameFlightControls->setMouseTracking(false);
        frameFlightControls->setContextMenuPolicy(Qt::NoContextMenu);
        frameFlightControls->setStyleSheet(QString::fromUtf8("QWidget { /* all types of qwidget */\n"
"background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(71, 71, 71, 255), stop:1 rgba(110, 110, 110, 255));\n"
"color: rgb(255, 255, 255);\n"
"}\n"
"\n"
"QToolButton {	/* all types of tool button */ \n"
"background-color: rgba(255, 255, 255, 0);\n"
"color: rgb(255, 255, 255);\n"
"/*border-style: none;*/\n"
"border: 0px; \n"
"}\n"
"QToolButton:hover {\n"
"background-color: qlineargradient(spread:pad, x1:1, y1:0.5, x2:0, y2:0.5, stop:0 rgba(71, 71, 71, 0), stop:0.5 rgba(150, 150, 150, 255), stop:1 rgba(71, 71, 71, 0));\n"
"color: rgb(255, 255, 255);\n"
"}\n"
"QToolButton:pressed {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(180, 180, 180, 255));\n"
"color: rgb(255, 255, 255);\n"
"}\n"
""));
        frameFlightControls->setFrameShape(QFrame::NoFrame);
        verticalLayout_4 = new QVBoxLayout(frameFlightControls);
        verticalLayout_4->setSpacing(1);
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        toolButtonHoldPosition = new QToolButton(frameFlightControls);
        toolButtonHoldPosition->setObjectName(QString::fromUtf8("toolButtonHoldPosition"));
        sizePolicy2.setHeightForWidth(toolButtonHoldPosition->sizePolicy().hasHeightForWidth());
        toolButtonHoldPosition->setSizePolicy(sizePolicy2);
        toolButtonHoldPosition->setMinimumSize(QSize(48, 0));
        toolButtonHoldPosition->setStyleSheet(QString::fromUtf8(""));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/opmap/images/hold.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonHoldPosition->setIcon(icon8);
        toolButtonHoldPosition->setIconSize(QSize(80, 40));
        toolButtonHoldPosition->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        toolButtonHoldPosition->setAutoRaise(false);

        verticalLayout_4->addWidget(toolButtonHoldPosition);

        toolButtonHome = new QToolButton(frameFlightControls);
        toolButtonHome->setObjectName(QString::fromUtf8("toolButtonHome"));
        toolButtonHome->setEnabled(true);
        sizePolicy2.setHeightForWidth(toolButtonHome->sizePolicy().hasHeightForWidth());
        toolButtonHome->setSizePolicy(sizePolicy2);
        toolButtonHome->setMinimumSize(QSize(48, 0));
        toolButtonHome->setAutoFillBackground(false);
        toolButtonHome->setStyleSheet(QString::fromUtf8(""));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/opmap/images/home.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonHome->setIcon(icon9);
        toolButtonHome->setIconSize(QSize(80, 40));
        toolButtonHome->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        toolButtonHome->setAutoRaise(false);

        verticalLayout_4->addWidget(toolButtonHome);

        toolButtonPrevWaypoint = new QToolButton(frameFlightControls);
        toolButtonPrevWaypoint->setObjectName(QString::fromUtf8("toolButtonPrevWaypoint"));
        sizePolicy2.setHeightForWidth(toolButtonPrevWaypoint->sizePolicy().hasHeightForWidth());
        toolButtonPrevWaypoint->setSizePolicy(sizePolicy2);
        toolButtonPrevWaypoint->setMinimumSize(QSize(48, 0));
        toolButtonPrevWaypoint->setStyleSheet(QString::fromUtf8(""));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/opmap/images/prev_waypoint.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonPrevWaypoint->setIcon(icon10);
        toolButtonPrevWaypoint->setIconSize(QSize(80, 40));
        toolButtonPrevWaypoint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        toolButtonPrevWaypoint->setAutoRaise(false);

        verticalLayout_4->addWidget(toolButtonPrevWaypoint);

        toolButtonNextWaypoint = new QToolButton(frameFlightControls);
        toolButtonNextWaypoint->setObjectName(QString::fromUtf8("toolButtonNextWaypoint"));
        sizePolicy2.setHeightForWidth(toolButtonNextWaypoint->sizePolicy().hasHeightForWidth());
        toolButtonNextWaypoint->setSizePolicy(sizePolicy2);
        toolButtonNextWaypoint->setMinimumSize(QSize(48, 0));
        toolButtonNextWaypoint->setStyleSheet(QString::fromUtf8(""));
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/opmap/images/next_waypoint.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonNextWaypoint->setIcon(icon11);
        toolButtonNextWaypoint->setIconSize(QSize(80, 40));
        toolButtonNextWaypoint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        toolButtonNextWaypoint->setAutoRaise(false);

        verticalLayout_4->addWidget(toolButtonNextWaypoint);

        toolButtonGo = new QToolButton(frameFlightControls);
        toolButtonGo->setObjectName(QString::fromUtf8("toolButtonGo"));
        sizePolicy2.setHeightForWidth(toolButtonGo->sizePolicy().hasHeightForWidth());
        toolButtonGo->setSizePolicy(sizePolicy2);
        toolButtonGo->setMinimumSize(QSize(48, 0));
        toolButtonGo->setStyleSheet(QString::fromUtf8(""));
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/opmap/images/go.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonGo->setIcon(icon12);
        toolButtonGo->setIconSize(QSize(80, 40));
        toolButtonGo->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        toolButtonGo->setAutoRaise(false);

        verticalLayout_4->addWidget(toolButtonGo);


        horizontalLayout_3->addWidget(frameFlightControls);


        verticalLayout->addLayout(horizontalLayout_3);

        layoutWidgetStatusBar_2 = new QHBoxLayout();
        layoutWidgetStatusBar_2->setSpacing(0);
        layoutWidgetStatusBar_2->setObjectName(QString::fromUtf8("layoutWidgetStatusBar_2"));
        layoutWidgetStatusBar_2->setContentsMargins(0, -1, 0, -1);
        frameStatusBar = new QFrame(OPMap_Widget);
        frameStatusBar->setObjectName(QString::fromUtf8("frameStatusBar"));
        sizePolicy1.setHeightForWidth(frameStatusBar->sizePolicy().hasHeightForWidth());
        frameStatusBar->setSizePolicy(sizePolicy1);
        frameStatusBar->setMinimumSize(QSize(0, 0));
        frameStatusBar->setContextMenuPolicy(Qt::NoContextMenu);
        frameStatusBar->setStyleSheet(QString::fromUtf8("QFrame{\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(110, 110, 110, 255), stop:1 rgba(71, 71, 71, 255));\n"
"color: rgba(255, 255, 255, 70);\n"
"}\n"
"\n"
"QLabel { /* all label types */\n"
"/* background-color: rgba(255, 255, 255, 0); */\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 128), stop:1 rgba(128, 128, 128, 128));\n"
"color: rgb(255, 255, 255);\n"
"/*border: 1px solid black;*/\n"
"border: none;\n"
"border-radius: 3px;\n"
"}\n"
""));
        frameStatusBar->setFrameShape(QFrame::NoFrame);
        frameStatusBar->setFrameShadow(QFrame::Plain);
        horizontalLayout = new QHBoxLayout(frameStatusBar);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 3, 0, 3);
        layoutWidgetStatusBar = new QHBoxLayout();
        layoutWidgetStatusBar->setSpacing(5);
        layoutWidgetStatusBar->setObjectName(QString::fromUtf8("layoutWidgetStatusBar"));
        layoutWidgetStatusBar->setContentsMargins(3, -1, 8, -1);
        horizontalSpacer_7 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        layoutWidgetStatusBar->addItem(horizontalSpacer_7);

        labelUAVPos = new QLabel(frameStatusBar);
        labelUAVPos->setObjectName(QString::fromUtf8("labelUAVPos"));
        sizePolicy.setHeightForWidth(labelUAVPos->sizePolicy().hasHeightForWidth());
        labelUAVPos->setSizePolicy(sizePolicy);
        QFont font;
        font.setPointSize(8);
        font.setBold(false);
        font.setItalic(false);
        font.setWeight(50);
        font.setKerning(true);
        labelUAVPos->setFont(font);
        labelUAVPos->setStyleSheet(QString::fromUtf8(""));
        labelUAVPos->setFrameShape(QFrame::NoFrame);
        labelUAVPos->setFrameShadow(QFrame::Raised);
        labelUAVPos->setAlignment(Qt::AlignCenter);
        labelUAVPos->setMargin(5);

        layoutWidgetStatusBar->addWidget(labelUAVPos);

        labelMapPos = new QLabel(frameStatusBar);
        labelMapPos->setObjectName(QString::fromUtf8("labelMapPos"));
        sizePolicy.setHeightForWidth(labelMapPos->sizePolicy().hasHeightForWidth());
        labelMapPos->setSizePolicy(sizePolicy);
        labelMapPos->setFont(font);
        labelMapPos->setStyleSheet(QString::fromUtf8(""));
        labelMapPos->setFrameShape(QFrame::NoFrame);
        labelMapPos->setAlignment(Qt::AlignCenter);
        labelMapPos->setMargin(5);

        layoutWidgetStatusBar->addWidget(labelMapPos);

        labelMousePos = new QLabel(frameStatusBar);
        labelMousePos->setObjectName(QString::fromUtf8("labelMousePos"));
        sizePolicy.setHeightForWidth(labelMousePos->sizePolicy().hasHeightForWidth());
        labelMousePos->setSizePolicy(sizePolicy);
        labelMousePos->setFont(font);
        labelMousePos->setStyleSheet(QString::fromUtf8(""));
        labelMousePos->setFrameShape(QFrame::NoFrame);
        labelMousePos->setFrameShadow(QFrame::Plain);
        labelMousePos->setAlignment(Qt::AlignCenter);
        labelMousePos->setMargin(5);

        layoutWidgetStatusBar->addWidget(labelMousePos);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        layoutWidgetStatusBar->addItem(horizontalSpacer_3);

        progressBarMap = new QProgressBar(frameStatusBar);
        progressBarMap->setObjectName(QString::fromUtf8("progressBarMap"));
        QSizePolicy sizePolicy6(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(progressBarMap->sizePolicy().hasHeightForWidth());
        progressBarMap->setSizePolicy(sizePolicy6);
        progressBarMap->setMinimumSize(QSize(40, 12));
        progressBarMap->setMaximumSize(QSize(100, 20));
        progressBarMap->setAutoFillBackground(false);
        progressBarMap->setStyleSheet(QString::fromUtf8("QProgressBar {\n"
"border: none;\n"
"border-radius: 5px;\n"
"padding: 3px;\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(120, 120, 120, 255));\n"
"color: rgb(255, 255, 255);\n"
"}\n"
"QProgressBar::chunk {\n"
"background-color: rgb(85, 85, 255);\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(160, 160, 255, 255), stop:1 rgba(80, 80, 160, 255));\n"
"border: none;\n"
"border-radius: 3px;\n"
"}"));
        progressBarMap->setValue(50);
        progressBarMap->setAlignment(Qt::AlignCenter);
        progressBarMap->setTextVisible(true);
        progressBarMap->setInvertedAppearance(false);

        layoutWidgetStatusBar->addWidget(progressBarMap);


        horizontalLayout->addLayout(layoutWidgetStatusBar);


        layoutWidgetStatusBar_2->addWidget(frameStatusBar);


        verticalLayout->addLayout(layoutWidgetStatusBar_2);


        retranslateUi(OPMap_Widget);

        comboBoxFindPlace->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(OPMap_Widget);
    } // setupUi

    void retranslateUi(QWidget *OPMap_Widget)
    {
        OPMap_Widget->setWindowTitle(QApplication::translate("OPMap_Widget", "Form", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        toolButtonWaypointsTreeViewShowHide->setToolTip(QApplication::translate("OPMap_Widget", "Show/Hide Waypoint Treeview", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonWaypointsTreeViewShowHide->setText(QString());
        comboBoxFindPlace->clear();
        comboBoxFindPlace->insertItems(0, QStringList()
         << QApplication::translate("OPMap_Widget", "london", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("OPMap_Widget", "new york", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("OPMap_Widget", "paris", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        comboBoxFindPlace->setToolTip(QApplication::translate("OPMap_Widget", "Enter a place here that you want to find, then press either return or the find button to the right.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        toolButtonFindPlace->setToolTip(QApplication::translate("OPMap_Widget", "Find place", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonFindPlace->setText(QString());
#ifndef QT_NO_TOOLTIP
        toolButtonMapHome->setToolTip(QApplication::translate("OPMap_Widget", "Center map over home position", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonMapHome->setText(QString());
#ifndef QT_NO_TOOLTIP
        toolButtonMapUAV->setToolTip(QApplication::translate("OPMap_Widget", "Center map over UAV position", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonMapUAV->setText(QString());
#ifndef QT_NO_TOOLTIP
        toolButtonAddWaypoint->setToolTip(QApplication::translate("OPMap_Widget", "Add a waypoint", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonAddWaypoint->setText(QString());
#ifndef QT_NO_TOOLTIP
        toolButtonWaypointEditor->setToolTip(QApplication::translate("OPMap_Widget", "Opens the waypoint editor", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonWaypointEditor->setText(QString());
#ifndef QT_NO_TOOLTIP
        toolButtonZoomM->setToolTip(QApplication::translate("OPMap_Widget", "Zoom out", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonZoomM->setText(QString());
#ifndef QT_NO_TOOLTIP
        toolButtonZoomP->setToolTip(QApplication::translate("OPMap_Widget", "Zoom in", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonZoomP->setText(QString());
#ifndef QT_NO_TOOLTIP
        horizontalSliderZoom->setToolTip(QApplication::translate("OPMap_Widget", "Zoom level", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        toolButtonFlightControlsShowHide->setToolTip(QApplication::translate("OPMap_Widget", "Show/Hide Flight Controls", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonFlightControlsShowHide->setText(QString());
#ifndef QT_NO_TOOLTIP
        toolButtonHoldPosition->setToolTip(QApplication::translate("OPMap_Widget", "UAV hold position", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonHoldPosition->setText(QApplication::translate("OPMap_Widget", "Hold", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        toolButtonHome->setToolTip(QApplication::translate("OPMap_Widget", "UAV go home", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonHome->setText(QApplication::translate("OPMap_Widget", "Home", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        toolButtonPrevWaypoint->setToolTip(QApplication::translate("OPMap_Widget", "Move UAV to previous waypoint", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonPrevWaypoint->setText(QApplication::translate("OPMap_Widget", "Previous WP", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        toolButtonNextWaypoint->setToolTip(QApplication::translate("OPMap_Widget", "Move UAV to next waypoint", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonNextWaypoint->setText(QApplication::translate("OPMap_Widget", "Next WP", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        toolButtonGo->setToolTip(QApplication::translate("OPMap_Widget", "UAV go!", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        toolButtonGo->setText(QApplication::translate("OPMap_Widget", "Go", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        labelUAVPos->setToolTip(QApplication::translate("OPMap_Widget", "UAV position", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        labelUAVPos->setText(QApplication::translate("OPMap_Widget", "labelUAVPos", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        labelMapPos->setToolTip(QApplication::translate("OPMap_Widget", "Map position", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        labelMapPos->setText(QApplication::translate("OPMap_Widget", "labelMapPos", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        labelMousePos->setToolTip(QApplication::translate("OPMap_Widget", "Mouse position", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        labelMousePos->setText(QApplication::translate("OPMap_Widget", "labelMousePos", 0, QApplication::UnicodeUTF8));
        progressBarMap->setFormat(QApplication::translate("OPMap_Widget", "%v", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class OPMap_Widget: public Ui_OPMap_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPMAP_WIDGET_H
