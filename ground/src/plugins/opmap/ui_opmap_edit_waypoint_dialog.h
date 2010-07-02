/********************************************************************************
** Form generated from reading UI file 'opmap_edit_waypoint_dialog.ui'
**
** Created: Fri 2. Jul 13:52:21 2010
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OPMAP_EDIT_WAYPOINT_DIALOG_H
#define UI_OPMAP_EDIT_WAYPOINT_DIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_opmap_edit_waypoint_dialog
{
public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label_7;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_6;
    QLabel *label_8;
    QLineEdit *lineEditDescription;
    QCheckBox *checkBoxLocked;
    QSpinBox *spinBoxNumber;
    QDoubleSpinBox *doubleSpinBoxLatitude;
    QDoubleSpinBox *doubleSpinBoxLongitude;
    QDoubleSpinBox *doubleSpinBoxAltitude;
    QLabel *label_4;
    QLabel *label_5;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButtonOK;
    QPushButton *pushButtonApply;
    QPushButton *pushButtonRevert;
    QPushButton *pushButtonCancel;

    void setupUi(QDialog *opmap_edit_waypoint_dialog)
    {
        if (opmap_edit_waypoint_dialog->objectName().isEmpty())
            opmap_edit_waypoint_dialog->setObjectName(QString::fromUtf8("opmap_edit_waypoint_dialog"));
        opmap_edit_waypoint_dialog->setWindowModality(Qt::ApplicationModal);
        opmap_edit_waypoint_dialog->resize(500, 187);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(opmap_edit_waypoint_dialog->sizePolicy().hasHeightForWidth());
        opmap_edit_waypoint_dialog->setSizePolicy(sizePolicy);
        opmap_edit_waypoint_dialog->setMinimumSize(QSize(500, 187));
        opmap_edit_waypoint_dialog->setMaximumSize(QSize(500, 187));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/core/images/openpilot_logo_128.png"), QSize(), QIcon::Normal, QIcon::Off);
        opmap_edit_waypoint_dialog->setWindowIcon(icon);
        opmap_edit_waypoint_dialog->setModal(true);
        verticalLayout = new QVBoxLayout(opmap_edit_waypoint_dialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_7 = new QLabel(opmap_edit_waypoint_dialog);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy1);
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_7, 0, 0, 1, 1);

        label = new QLabel(opmap_edit_waypoint_dialog);
        label->setObjectName(QString::fromUtf8("label"));
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label, 1, 0, 1, 1);

        label_2 = new QLabel(opmap_edit_waypoint_dialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        label_3 = new QLabel(opmap_edit_waypoint_dialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        sizePolicy1.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy1);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_3, 3, 0, 1, 1);

        label_6 = new QLabel(opmap_edit_waypoint_dialog);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 3, 3, 1, 1);

        label_8 = new QLabel(opmap_edit_waypoint_dialog);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        sizePolicy1.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy1);
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_8, 4, 0, 1, 1);

        lineEditDescription = new QLineEdit(opmap_edit_waypoint_dialog);
        lineEditDescription->setObjectName(QString::fromUtf8("lineEditDescription"));

        gridLayout->addWidget(lineEditDescription, 4, 2, 1, 4);

        checkBoxLocked = new QCheckBox(opmap_edit_waypoint_dialog);
        checkBoxLocked->setObjectName(QString::fromUtf8("checkBoxLocked"));
        checkBoxLocked->setLayoutDirection(Qt::RightToLeft);

        gridLayout->addWidget(checkBoxLocked, 0, 5, 1, 1);

        spinBoxNumber = new QSpinBox(opmap_edit_waypoint_dialog);
        spinBoxNumber->setObjectName(QString::fromUtf8("spinBoxNumber"));
        spinBoxNumber->setMaximum(200);

        gridLayout->addWidget(spinBoxNumber, 0, 2, 1, 1);

        doubleSpinBoxLatitude = new QDoubleSpinBox(opmap_edit_waypoint_dialog);
        doubleSpinBoxLatitude->setObjectName(QString::fromUtf8("doubleSpinBoxLatitude"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(doubleSpinBoxLatitude->sizePolicy().hasHeightForWidth());
        doubleSpinBoxLatitude->setSizePolicy(sizePolicy2);
        doubleSpinBoxLatitude->setDecimals(7);
        doubleSpinBoxLatitude->setMinimum(-90);
        doubleSpinBoxLatitude->setMaximum(90);

        gridLayout->addWidget(doubleSpinBoxLatitude, 1, 2, 1, 1);

        doubleSpinBoxLongitude = new QDoubleSpinBox(opmap_edit_waypoint_dialog);
        doubleSpinBoxLongitude->setObjectName(QString::fromUtf8("doubleSpinBoxLongitude"));
        doubleSpinBoxLongitude->setDecimals(7);
        doubleSpinBoxLongitude->setMinimum(-180);
        doubleSpinBoxLongitude->setMaximum(180);

        gridLayout->addWidget(doubleSpinBoxLongitude, 2, 2, 1, 1);

        doubleSpinBoxAltitude = new QDoubleSpinBox(opmap_edit_waypoint_dialog);
        doubleSpinBoxAltitude->setObjectName(QString::fromUtf8("doubleSpinBoxAltitude"));
        doubleSpinBoxAltitude->setMinimum(-5000);
        doubleSpinBoxAltitude->setMaximum(5000);

        gridLayout->addWidget(doubleSpinBoxAltitude, 3, 2, 1, 1);

        label_4 = new QLabel(opmap_edit_waypoint_dialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 1, 3, 1, 1);

        label_5 = new QLabel(opmap_edit_waypoint_dialog);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 2, 3, 1, 1);


        verticalLayout->addLayout(gridLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButtonOK = new QPushButton(opmap_edit_waypoint_dialog);
        pushButtonOK->setObjectName(QString::fromUtf8("pushButtonOK"));

        horizontalLayout->addWidget(pushButtonOK);

        pushButtonApply = new QPushButton(opmap_edit_waypoint_dialog);
        pushButtonApply->setObjectName(QString::fromUtf8("pushButtonApply"));

        horizontalLayout->addWidget(pushButtonApply);

        pushButtonRevert = new QPushButton(opmap_edit_waypoint_dialog);
        pushButtonRevert->setObjectName(QString::fromUtf8("pushButtonRevert"));

        horizontalLayout->addWidget(pushButtonRevert);

        pushButtonCancel = new QPushButton(opmap_edit_waypoint_dialog);
        pushButtonCancel->setObjectName(QString::fromUtf8("pushButtonCancel"));

        horizontalLayout->addWidget(pushButtonCancel);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(opmap_edit_waypoint_dialog);

        QMetaObject::connectSlotsByName(opmap_edit_waypoint_dialog);
    } // setupUi

    void retranslateUi(QDialog *opmap_edit_waypoint_dialog)
    {
        opmap_edit_waypoint_dialog->setWindowTitle(QApplication::translate("opmap_edit_waypoint_dialog", "OpenPilot GCS Edit Waypoint", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("opmap_edit_waypoint_dialog", "Number ", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("opmap_edit_waypoint_dialog", "Latitude ", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("opmap_edit_waypoint_dialog", "Longitude ", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("opmap_edit_waypoint_dialog", "Altitude ", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("opmap_edit_waypoint_dialog", "meters", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("opmap_edit_waypoint_dialog", "Description ", 0, QApplication::UnicodeUTF8));
        checkBoxLocked->setText(QApplication::translate("opmap_edit_waypoint_dialog", "Locked", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("opmap_edit_waypoint_dialog", "degrees", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("opmap_edit_waypoint_dialog", "degrees", 0, QApplication::UnicodeUTF8));
        pushButtonOK->setText(QApplication::translate("opmap_edit_waypoint_dialog", "OK", 0, QApplication::UnicodeUTF8));
        pushButtonApply->setText(QApplication::translate("opmap_edit_waypoint_dialog", "Apply", 0, QApplication::UnicodeUTF8));
        pushButtonRevert->setText(QApplication::translate("opmap_edit_waypoint_dialog", "Revert", 0, QApplication::UnicodeUTF8));
        pushButtonCancel->setText(QApplication::translate("opmap_edit_waypoint_dialog", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class opmap_edit_waypoint_dialog: public Ui_opmap_edit_waypoint_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPMAP_EDIT_WAYPOINT_DIALOG_H
