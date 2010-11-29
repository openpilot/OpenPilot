/********************************************************************************
** Form generated from reading UI file 'hitloptionspage.ui'
**
** Created: Wed Nov 24 20:23:33 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HITLOPTIONSPAGE_H
#define UI_HITLOPTIONSPAGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>
#include "utils/pathchooser.h"

QT_BEGIN_NAMESPACE

class Ui_HITLOptionsPage
{
public:
    QGridLayout *gridLayout;
    QLabel *label_3;
    QComboBox *chooseFlightSimulator;
    QFrame *line;
    QSpacerItem *verticalSpacer;
    QLabel *label_6;
    QLineEdit *hostAddress;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_2;
    QLineEdit *inputPort;
    Utils::PathChooser *executablePath;
    QLabel *label_4;
    Utils::PathChooser *dataPath;
    QLineEdit *longitude;
    QLineEdit *outputPort;
    QCheckBox *manualControl;
    QLabel *label;
    QLineEdit *latitude;
    QLabel *label_5;
    QCheckBox *startSim;
    QLabel *label_9;
    QLineEdit *remoteHostAddress;

    void setupUi(QWidget *HITLOptionsPage)
    {
        if (HITLOptionsPage->objectName().isEmpty())
            HITLOptionsPage->setObjectName(QString::fromUtf8("HITLOptionsPage"));
        HITLOptionsPage->resize(577, 367);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(HITLOptionsPage->sizePolicy().hasHeightForWidth());
        HITLOptionsPage->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(HITLOptionsPage);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_3 = new QLabel(HITLOptionsPage);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 1, 1, 1, 1);

        chooseFlightSimulator = new QComboBox(HITLOptionsPage);
        chooseFlightSimulator->setObjectName(QString::fromUtf8("chooseFlightSimulator"));

        gridLayout->addWidget(chooseFlightSimulator, 1, 2, 1, 3);

        line = new QFrame(HITLOptionsPage);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line, 2, 1, 1, 4);

        verticalSpacer = new QSpacerItem(20, 182, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 12, 2, 1, 1);

        label_6 = new QLabel(HITLOptionsPage);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 3, 1, 1, 1);

        hostAddress = new QLineEdit(HITLOptionsPage);
        hostAddress->setObjectName(QString::fromUtf8("hostAddress"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(hostAddress->sizePolicy().hasHeightForWidth());
        hostAddress->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(hostAddress, 3, 2, 1, 2);

        label_7 = new QLabel(HITLOptionsPage);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 6, 1, 1, 1);

        label_8 = new QLabel(HITLOptionsPage);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout->addWidget(label_8, 6, 3, 1, 1);

        label_2 = new QLabel(HITLOptionsPage);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(label_2, 8, 1, 1, 1);

        inputPort = new QLineEdit(HITLOptionsPage);
        inputPort->setObjectName(QString::fromUtf8("inputPort"));

        gridLayout->addWidget(inputPort, 5, 2, 1, 1);

        executablePath = new Utils::PathChooser(HITLOptionsPage);
        executablePath->setObjectName(QString::fromUtf8("executablePath"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(1);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(executablePath->sizePolicy().hasHeightForWidth());
        executablePath->setSizePolicy(sizePolicy3);

        gridLayout->addWidget(executablePath, 7, 2, 1, 3);

        label_4 = new QLabel(HITLOptionsPage);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 5, 3, 1, 1);

        dataPath = new Utils::PathChooser(HITLOptionsPage);
        dataPath->setObjectName(QString::fromUtf8("dataPath"));

        gridLayout->addWidget(dataPath, 8, 2, 1, 3);

        longitude = new QLineEdit(HITLOptionsPage);
        longitude->setObjectName(QString::fromUtf8("longitude"));

        gridLayout->addWidget(longitude, 6, 4, 1, 1);

        outputPort = new QLineEdit(HITLOptionsPage);
        outputPort->setObjectName(QString::fromUtf8("outputPort"));

        gridLayout->addWidget(outputPort, 5, 4, 1, 1);

        manualControl = new QCheckBox(HITLOptionsPage);
        manualControl->setObjectName(QString::fromUtf8("manualControl"));
        sizePolicy2.setHeightForWidth(manualControl->sizePolicy().hasHeightForWidth());
        manualControl->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(manualControl, 9, 1, 1, 4);

        label = new QLabel(HITLOptionsPage);
        label->setObjectName(QString::fromUtf8("label"));
        sizePolicy2.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(label, 7, 1, 1, 1);

        latitude = new QLineEdit(HITLOptionsPage);
        latitude->setObjectName(QString::fromUtf8("latitude"));

        gridLayout->addWidget(latitude, 6, 2, 1, 1);

        label_5 = new QLabel(HITLOptionsPage);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 5, 1, 1, 1);

        startSim = new QCheckBox(HITLOptionsPage);
        startSim->setObjectName(QString::fromUtf8("startSim"));
        startSim->setEnabled(true);
        sizePolicy2.setHeightForWidth(startSim->sizePolicy().hasHeightForWidth());
        startSim->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(startSim, 10, 1, 1, 1);

        label_9 = new QLabel(HITLOptionsPage);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout->addWidget(label_9, 4, 1, 1, 1);

        remoteHostAddress = new QLineEdit(HITLOptionsPage);
        remoteHostAddress->setObjectName(QString::fromUtf8("remoteHostAddress"));
        sizePolicy1.setHeightForWidth(remoteHostAddress->sizePolicy().hasHeightForWidth());
        remoteHostAddress->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(remoteHostAddress, 4, 2, 1, 1);


        retranslateUi(HITLOptionsPage);

        QMetaObject::connectSlotsByName(HITLOptionsPage);
    } // setupUi

    void retranslateUi(QWidget *HITLOptionsPage)
    {
        HITLOptionsPage->setWindowTitle(QApplication::translate("HITLOptionsPage", "Form", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("HITLOptionsPage", "Choose flight simulator:", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("HITLOptionsPage", "Local interface address (IP):", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        hostAddress->setToolTip(QApplication::translate("HITLOptionsPage", "For communication with sim computer via network. Should be IP address of one of the local interfaces. ", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_7->setText(QApplication::translate("HITLOptionsPage", "Latitude in degrees:", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("HITLOptionsPage", "Longitude in degrees:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("HITLOptionsPage", "Data directory:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        inputPort->setToolTip(QApplication::translate("HITLOptionsPage", "For receiving data from sim", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("HITLOptionsPage", "Output Port:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        outputPort->setToolTip(QApplication::translate("HITLOptionsPage", "For sending data to sim", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        manualControl->setToolTip(QApplication::translate("HITLOptionsPage", "Manual aircraft control (can be used when hardware is not available)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        manualControl->setText(QApplication::translate("HITLOptionsPage", "Manual aircraft control (can be used when hardware is not available)", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("HITLOptionsPage", "Path executable:", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("HITLOptionsPage", "Input Port:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        startSim->setToolTip(QApplication::translate("HITLOptionsPage", "Check this box to start the simulator on the local computer", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        startSim->setText(QApplication::translate("HITLOptionsPage", "Start simulator on local machine", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("HITLOptionsPage", "Remote interface address (IP):", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        remoteHostAddress->setToolTip(QApplication::translate("HITLOptionsPage", "Only required if running simulator on remote machine ", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class HITLOptionsPage: public Ui_HITLOptionsPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HITLOPTIONSPAGE_H
