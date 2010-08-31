/********************************************************************************
** Form generated from reading UI file 'hitloptionspage.ui'
**
** Created: Wed 25. Aug 11:43:47 2010
**      by: Qt User Interface Compiler version 4.6.2
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
    QLabel *label_7;
    QLineEdit *latitude;
    QLabel *label_8;
    QLineEdit *longitude;
    QLabel *label;
    Utils::PathChooser *executablePath;
    QLabel *label_2;
    Utils::PathChooser *dataPath;
    QCheckBox *manualControl;
    QSpacerItem *verticalSpacer;
    QLabel *label_6;
    QLineEdit *hostAddress;
    QLineEdit *outputPort;
    QLabel *label_4;
    QLineEdit *inputPort;
    QLabel *label_5;

    void setupUi(QWidget *HITLOptionsPage)
    {
        if (HITLOptionsPage->objectName().isEmpty())
            HITLOptionsPage->setObjectName(QString::fromUtf8("HITLOptionsPage"));
        HITLOptionsPage->resize(400, 320);
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

        label_7 = new QLabel(HITLOptionsPage);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 5, 1, 1, 1);

        latitude = new QLineEdit(HITLOptionsPage);
        latitude->setObjectName(QString::fromUtf8("latitude"));

        gridLayout->addWidget(latitude, 5, 2, 1, 1);

        label_8 = new QLabel(HITLOptionsPage);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout->addWidget(label_8, 5, 3, 1, 1);

        longitude = new QLineEdit(HITLOptionsPage);
        longitude->setObjectName(QString::fromUtf8("longitude"));

        gridLayout->addWidget(longitude, 5, 4, 1, 1);

        label = new QLabel(HITLOptionsPage);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label, 6, 1, 1, 1);

        executablePath = new Utils::PathChooser(HITLOptionsPage);
        executablePath->setObjectName(QString::fromUtf8("executablePath"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(1);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(executablePath->sizePolicy().hasHeightForWidth());
        executablePath->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(executablePath, 6, 2, 1, 3);

        label_2 = new QLabel(HITLOptionsPage);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_2, 7, 1, 1, 1);

        dataPath = new Utils::PathChooser(HITLOptionsPage);
        dataPath->setObjectName(QString::fromUtf8("dataPath"));

        gridLayout->addWidget(dataPath, 7, 2, 1, 3);

        manualControl = new QCheckBox(HITLOptionsPage);
        manualControl->setObjectName(QString::fromUtf8("manualControl"));
        sizePolicy1.setHeightForWidth(manualControl->sizePolicy().hasHeightForWidth());
        manualControl->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(manualControl, 8, 1, 1, 4);

        verticalSpacer = new QSpacerItem(20, 182, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 11, 2, 1, 1);

        label_6 = new QLabel(HITLOptionsPage);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 3, 1, 1, 1);

        hostAddress = new QLineEdit(HITLOptionsPage);
        hostAddress->setObjectName(QString::fromUtf8("hostAddress"));

        gridLayout->addWidget(hostAddress, 3, 2, 1, 1);

        outputPort = new QLineEdit(HITLOptionsPage);
        outputPort->setObjectName(QString::fromUtf8("outputPort"));

        gridLayout->addWidget(outputPort, 4, 4, 1, 1);

        label_4 = new QLabel(HITLOptionsPage);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 4, 3, 1, 1);

        inputPort = new QLineEdit(HITLOptionsPage);
        inputPort->setObjectName(QString::fromUtf8("inputPort"));

        gridLayout->addWidget(inputPort, 4, 2, 1, 1);

        label_5 = new QLabel(HITLOptionsPage);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 4, 1, 1, 1);


        retranslateUi(HITLOptionsPage);

        QMetaObject::connectSlotsByName(HITLOptionsPage);
    } // setupUi

    void retranslateUi(QWidget *HITLOptionsPage)
    {
        HITLOptionsPage->setWindowTitle(QApplication::translate("HITLOptionsPage", "Form", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("HITLOptionsPage", "Choose flight simulator:", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("HITLOptionsPage", "Latitude in degrees:", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("HITLOptionsPage", "Longitude in degrees:", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("HITLOptionsPage", "Path executable:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("HITLOptionsPage", "Data directory:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        manualControl->setToolTip(QApplication::translate("HITLOptionsPage", "Manual aircraft control (can be used when hardware is not available)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        manualControl->setText(QApplication::translate("HITLOptionsPage", "Manual aircraft control (can be used when hardware is not available)", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("HITLOptionsPage", "Host Address:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("HITLOptionsPage", "Output Port:", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("HITLOptionsPage", "Input Port:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class HITLOptionsPage: public Ui_HITLOptionsPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HITLOPTIONSPAGE_H
