/********************************************************************************
** Form generated from reading UI file 'fgoptionspage.ui'
**
** Created: Tue 24. Aug 21:12:33 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FGOPTIONSPAGE_H
#define UI_FGOPTIONSPAGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>
#include "utils/pathchooser.h"

QT_BEGIN_NAMESPACE

class Ui_FGOptionsPage
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QLabel *label_2;
    Utils::PathChooser *executablePathChooser;
    Utils::PathChooser *dataDirectoryPathChooser;
    QCheckBox *fgManualControl;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *FGOptionsPage)
    {
        if (FGOptionsPage->objectName().isEmpty())
            FGOptionsPage->setObjectName(QString::fromUtf8("FGOptionsPage"));
        FGOptionsPage->resize(400, 320);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(FGOptionsPage->sizePolicy().hasHeightForWidth());
        FGOptionsPage->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(FGOptionsPage);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(FGOptionsPage);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_2 = new QLabel(FGOptionsPage);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        executablePathChooser = new Utils::PathChooser(FGOptionsPage);
        executablePathChooser->setObjectName(QString::fromUtf8("executablePathChooser"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(1);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(executablePathChooser->sizePolicy().hasHeightForWidth());
        executablePathChooser->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(executablePathChooser, 0, 1, 1, 1);

        dataDirectoryPathChooser = new Utils::PathChooser(FGOptionsPage);
        dataDirectoryPathChooser->setObjectName(QString::fromUtf8("dataDirectoryPathChooser"));

        gridLayout->addWidget(dataDirectoryPathChooser, 1, 1, 1, 1);

        fgManualControl = new QCheckBox(FGOptionsPage);
        fgManualControl->setObjectName(QString::fromUtf8("fgManualControl"));
        sizePolicy1.setHeightForWidth(fgManualControl->sizePolicy().hasHeightForWidth());
        fgManualControl->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(fgManualControl, 3, 1, 1, 1);

        verticalSpacer = new QSpacerItem(20, 182, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 4, 1, 1, 1);


        retranslateUi(FGOptionsPage);

        QMetaObject::connectSlotsByName(FGOptionsPage);
    } // setupUi

    void retranslateUi(QWidget *FGOptionsPage)
    {
        FGOptionsPage->setWindowTitle(QApplication::translate("FGOptionsPage", "Form", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("FGOptionsPage", "FlightGear executable:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("FGOptionsPage", "FlightGear data directory:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        fgManualControl->setToolTip(QApplication::translate("FGOptionsPage", "Manual aircraft control (can be used when hardware is not available)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        fgManualControl->setText(QApplication::translate("FGOptionsPage", "Manual aircraft control (can be used when hardware is not available)", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class FGOptionsPage: public Ui_FGOptionsPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FGOPTIONSPAGE_H
