/********************************************************************************
** Form generated from reading UI file 'opmap_mapoverlaywidget.ui'
**
** Created: Fri 2. Jul 13:44:00 2010
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OPMAP_MAPOVERLAYWIDGET_H
#define UI_OPMAP_MAPOVERLAYWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OPMap_MapOverlayWidget
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout_2;
    QToolButton *toolButton;
    QSlider *verticalSlider;
    QToolButton *toolButton_2;
    QSpacerItem *horizontalSpacer_5;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_4;
    QFrame *frame;
    QHBoxLayout *horizontalLayout_3;
    QLabel *labelStatus;
    QSpacerItem *horizontalSpacer_3;
    QProgressBar *progressBarTiles;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QWidget *OPMap_MapOverlayWidget)
    {
        if (OPMap_MapOverlayWidget->objectName().isEmpty())
            OPMap_MapOverlayWidget->setObjectName(QString::fromUtf8("OPMap_MapOverlayWidget"));
        OPMap_MapOverlayWidget->resize(476, 289);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(OPMap_MapOverlayWidget->sizePolicy().hasHeightForWidth());
        OPMap_MapOverlayWidget->setSizePolicy(sizePolicy);
        OPMap_MapOverlayWidget->setAutoFillBackground(false);
        OPMap_MapOverlayWidget->setStyleSheet(QString::fromUtf8("/*background-color: rgba(0, 0, 0, 0);*/\n"
"background-color: transparent;\n"
"\n"
"QFrame {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(110, 110, 110, 255), stop:1 rgba(71, 71, 71, 255));\n"
"color: rgba(255, 255, 255, 70);\n"
"}\n"
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
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x"
                        "2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(120, 120, 120, 255));\n"
"color: rgb(255, 255, 255);\n"
"}\n"
"\n"
"QToolButton:checked {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(120, 120, 120, 255));\n"
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
"color: rgb(255, 255, "
                        "255);\n"
"}\n"
"QComboBox:hover {\n"
"background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(48, 48, 48, 255), stop:1 rgba(120, 120, 120, 255));\n"
"	background-color: rgb(197, 138, 0);\n"
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
"\n"
"QToolTip {\n"
"background-color: white;\n"
"color: black;\n"
"border: 1px solid black;\n"
"padding: 5px;\n"
"border-radius: 5px;\n"
"/* opacity: 170; */ \n"
"}\n"
""));
        verticalLayout = new QVBoxLayout(OPMap_MapOverlayWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, -1, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        frame_2 = new QFrame(OPMap_MapOverlayWidget);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(frame_2->sizePolicy().hasHeightForWidth());
        frame_2->setSizePolicy(sizePolicy1);
        frame_2->setMinimumSize(QSize(27, 170));
        frame_2->setMaximumSize(QSize(27, 170));
        frame_2->setStyleSheet(QString::fromUtf8("background-color: rgba(0, 0, 0, 100);\n"
"border: none;\n"
"border-radius: 12px;"));
        frame_2->setFrameShape(QFrame::NoFrame);
        frame_2->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(frame_2);
        verticalLayout_2->setSpacing(3);
        verticalLayout_2->setContentsMargins(3, 3, 3, 3);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        toolButton = new QToolButton(frame_2);
        toolButton->setObjectName(QString::fromUtf8("toolButton"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(toolButton->sizePolicy().hasHeightForWidth());
        toolButton->setSizePolicy(sizePolicy2);
        toolButton->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/opmap/images/plus.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButton->setIcon(icon);
        toolButton->setAutoRaise(true);

        verticalLayout_2->addWidget(toolButton);

        verticalSlider = new QSlider(frame_2);
        verticalSlider->setObjectName(QString::fromUtf8("verticalSlider"));
        sizePolicy2.setHeightForWidth(verticalSlider->sizePolicy().hasHeightForWidth());
        verticalSlider->setSizePolicy(sizePolicy2);
        verticalSlider->setMinimumSize(QSize(0, 120));
        verticalSlider->setMaximumSize(QSize(16777215, 120));
        verticalSlider->setStyleSheet(QString::fromUtf8("QSlider {\n"
"}\n"
"background-color: rgba(255, 255, 255, 0);\n"
"QSlider::handle:vertical {\n"
"border: 1px solid #5c5c5c;\n"
"background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(128, 128, 128, 255), stop:1 rgba(255, 255, 255, 255));\n"
"width: 24px;\n"
"height: 20px;\n"
"margin: -2px 0;\n"
"border-radius: 3px;\n"
"}\n"
"\n"
""));
        verticalSlider->setMinimum(2);
        verticalSlider->setMaximum(19);
        verticalSlider->setPageStep(1);
        verticalSlider->setTracking(true);
        verticalSlider->setOrientation(Qt::Vertical);
        verticalSlider->setInvertedAppearance(false);
        verticalSlider->setInvertedControls(false);

        verticalLayout_2->addWidget(verticalSlider);

        toolButton_2 = new QToolButton(frame_2);
        toolButton_2->setObjectName(QString::fromUtf8("toolButton_2"));
        sizePolicy2.setHeightForWidth(toolButton_2->sizePolicy().hasHeightForWidth());
        toolButton_2->setSizePolicy(sizePolicy2);
        toolButton_2->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 0);"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/opmap/images/minus.png"), QSize(), QIcon::Normal, QIcon::Off);
        toolButton_2->setIcon(icon1);
        toolButton_2->setAutoRaise(true);

        verticalLayout_2->addWidget(toolButton_2);


        horizontalLayout->addWidget(frame_2);

        horizontalSpacer_5 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_5);


        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_4);

        frame = new QFrame(OPMap_MapOverlayWidget);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setStyleSheet(QString::fromUtf8("background-color: rgba(0, 0, 0, 100);\n"
"border: none;\n"
"border-radius: 8px;\n"
"border-bottom-left-radius: 0px;\n"
"border-bottom-right-radius: 0px; "));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout_3 = new QHBoxLayout(frame);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(-1, 0, -1, 0);
        labelStatus = new QLabel(frame);
        labelStatus->setObjectName(QString::fromUtf8("labelStatus"));
        labelStatus->setStyleSheet(QString::fromUtf8("QLabel { /* all label types */\n"
"color: rgb(255, 255, 255);\n"
"	background-color: rgba(255, 255, 255, 0);\n"
"border: none;\n"
"border-radius: 10px;\n"
"}"));
        labelStatus->setFrameShape(QFrame::NoFrame);
        labelStatus->setAlignment(Qt::AlignCenter);
        labelStatus->setMargin(1);

        horizontalLayout_3->addWidget(labelStatus);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);

        progressBarTiles = new QProgressBar(frame);
        progressBarTiles->setObjectName(QString::fromUtf8("progressBarTiles"));
        sizePolicy1.setHeightForWidth(progressBarTiles->sizePolicy().hasHeightForWidth());
        progressBarTiles->setSizePolicy(sizePolicy1);
        progressBarTiles->setMinimumSize(QSize(100, 0));
        progressBarTiles->setMaximumSize(QSize(100, 14));
        progressBarTiles->setStyleSheet(QString::fromUtf8("QProgressBar {\n"
"border: nonei;\n"
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
        progressBarTiles->setValue(24);
        progressBarTiles->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(progressBarTiles);


        horizontalLayout_2->addWidget(frame);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(OPMap_MapOverlayWidget);

        QMetaObject::connectSlotsByName(OPMap_MapOverlayWidget);
    } // setupUi

    void retranslateUi(QWidget *OPMap_MapOverlayWidget)
    {
        OPMap_MapOverlayWidget->setWindowTitle(QString());
        toolButton->setText(QString());
        toolButton_2->setText(QString());
        labelStatus->setText(QApplication::translate("OPMap_MapOverlayWidget", "labelStatus", 0, QApplication::UnicodeUTF8));
        progressBarTiles->setFormat(QApplication::translate("OPMap_MapOverlayWidget", "%v", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class OPMap_MapOverlayWidget: public Ui_OPMap_MapOverlayWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPMAP_MAPOVERLAYWIDGET_H
