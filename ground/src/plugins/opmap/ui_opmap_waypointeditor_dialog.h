/********************************************************************************
** Form generated from reading UI file 'opmap_waypointeditor_dialog.ui'
**
** Created: Fri 2. Jul 13:44:00 2010
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OPMAP_WAYPOINTEDITOR_DIALOG_H
#define UI_OPMAP_WAYPOINTEDITOR_DIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGraphicsView>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QSplitter>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_opmap_waypointeditor_dialog
{
public:
    QGridLayout *gridLayout_3;
    QVBoxLayout *verticalLayout;
    QSplitter *splitter;
    QGroupBox *groupBoxWaypoints;
    QGridLayout *gridLayout_2;
    QTableWidget *tableWidgetWaypoints;
    QGroupBox *groupBoxHeightAndTimeline;
    QGridLayout *gridLayout;
    QGraphicsView *graphicsViewWaypointHeightAndTimeline;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *opmap_waypointeditor_dialog)
    {
        if (opmap_waypointeditor_dialog->objectName().isEmpty())
            opmap_waypointeditor_dialog->setObjectName(QString::fromUtf8("opmap_waypointeditor_dialog"));
        opmap_waypointeditor_dialog->resize(561, 511);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(opmap_waypointeditor_dialog->sizePolicy().hasHeightForWidth());
        opmap_waypointeditor_dialog->setSizePolicy(sizePolicy);
        opmap_waypointeditor_dialog->setMinimumSize(QSize(300, 0));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/core/images/openpilot_logo_128.png"), QSize(), QIcon::Normal, QIcon::Off);
        opmap_waypointeditor_dialog->setWindowIcon(icon);
        opmap_waypointeditor_dialog->setSizeGripEnabled(true);
        gridLayout_3 = new QGridLayout(opmap_waypointeditor_dialog);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        splitter = new QSplitter(opmap_waypointeditor_dialog);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setStyleSheet(QString::fromUtf8("QSplitter::handle {\n"
"/*	image: url(images/splitter.png); */\n"
"	background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(255, 255, 255, 80), stop:1 rgba(0, 0, 0, 80));\n"
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
        splitter->setOrientation(Qt::Vertical);
        splitter->setOpaqueResize(true);
        splitter->setHandleWidth(5);
        splitter->setChildrenCollapsible(false);
        groupBoxWaypoints = new QGroupBox(splitter);
        groupBoxWaypoints->setObjectName(QString::fromUtf8("groupBoxWaypoints"));
        groupBoxWaypoints->setMinimumSize(QSize(0, 130));
        groupBoxWaypoints->setFlat(true);
        gridLayout_2 = new QGridLayout(groupBoxWaypoints);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, -1, 0, -1);
        tableWidgetWaypoints = new QTableWidget(groupBoxWaypoints);
        if (tableWidgetWaypoints->columnCount() < 7)
            tableWidgetWaypoints->setColumnCount(7);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidgetWaypoints->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidgetWaypoints->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidgetWaypoints->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidgetWaypoints->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableWidgetWaypoints->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tableWidgetWaypoints->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tableWidgetWaypoints->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        tableWidgetWaypoints->setObjectName(QString::fromUtf8("tableWidgetWaypoints"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tableWidgetWaypoints->sizePolicy().hasHeightForWidth());
        tableWidgetWaypoints->setSizePolicy(sizePolicy1);
        tableWidgetWaypoints->setMinimumSize(QSize(0, 50));
        tableWidgetWaypoints->setDragEnabled(true);
        tableWidgetWaypoints->setShowGrid(false);
        tableWidgetWaypoints->setWordWrap(false);

        gridLayout_2->addWidget(tableWidgetWaypoints, 0, 0, 1, 1);

        splitter->addWidget(groupBoxWaypoints);
        groupBoxHeightAndTimeline = new QGroupBox(splitter);
        groupBoxHeightAndTimeline->setObjectName(QString::fromUtf8("groupBoxHeightAndTimeline"));
        groupBoxHeightAndTimeline->setFlat(true);
        gridLayout = new QGridLayout(groupBoxHeightAndTimeline);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, -1, 0, -1);
        graphicsViewWaypointHeightAndTimeline = new QGraphicsView(groupBoxHeightAndTimeline);
        graphicsViewWaypointHeightAndTimeline->setObjectName(QString::fromUtf8("graphicsViewWaypointHeightAndTimeline"));
        sizePolicy1.setHeightForWidth(graphicsViewWaypointHeightAndTimeline->sizePolicy().hasHeightForWidth());
        graphicsViewWaypointHeightAndTimeline->setSizePolicy(sizePolicy1);
        graphicsViewWaypointHeightAndTimeline->setMinimumSize(QSize(0, 50));
        graphicsViewWaypointHeightAndTimeline->setStyleSheet(QString::fromUtf8("background-color: rgb(191, 191, 191);"));
        graphicsViewWaypointHeightAndTimeline->setFrameShadow(QFrame::Sunken);
        graphicsViewWaypointHeightAndTimeline->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        graphicsViewWaypointHeightAndTimeline->setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing|QPainter::TextAntialiasing);

        gridLayout->addWidget(graphicsViewWaypointHeightAndTimeline, 0, 0, 1, 1);

        splitter->addWidget(groupBoxHeightAndTimeline);

        verticalLayout->addWidget(splitter);

        buttonBox = new QDialogButtonBox(opmap_waypointeditor_dialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        gridLayout_3->addLayout(verticalLayout, 0, 0, 1, 1);


        retranslateUi(opmap_waypointeditor_dialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), opmap_waypointeditor_dialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), opmap_waypointeditor_dialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(opmap_waypointeditor_dialog);
    } // setupUi

    void retranslateUi(QDialog *opmap_waypointeditor_dialog)
    {
        opmap_waypointeditor_dialog->setWindowTitle(QApplication::translate("opmap_waypointeditor_dialog", "OpenPilot GCS Waypoint Editor", 0, QApplication::UnicodeUTF8));
        groupBoxWaypoints->setTitle(QApplication::translate("opmap_waypointeditor_dialog", "Waypoints", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tableWidgetWaypoints->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("opmap_waypointeditor_dialog", "Num", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidgetWaypoints->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("opmap_waypointeditor_dialog", "Locked", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidgetWaypoints->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("opmap_waypointeditor_dialog", "Latitude", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidgetWaypoints->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("opmap_waypointeditor_dialog", "Longitude", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem4 = tableWidgetWaypoints->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("opmap_waypointeditor_dialog", "Altitude", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem5 = tableWidgetWaypoints->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QApplication::translate("opmap_waypointeditor_dialog", "Time", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem6 = tableWidgetWaypoints->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QApplication::translate("opmap_waypointeditor_dialog", "Hold Time", 0, QApplication::UnicodeUTF8));
        groupBoxHeightAndTimeline->setTitle(QApplication::translate("opmap_waypointeditor_dialog", "Height and Timeline", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class opmap_waypointeditor_dialog: public Ui_opmap_waypointeditor_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPMAP_WAYPOINTEDITOR_DIALOG_H
