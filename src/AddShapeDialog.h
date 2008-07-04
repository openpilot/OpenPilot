/********************************************************************************
** Form generated from reading ui file 'AddShapeDialog.ui'
**
** Created: Thu Jun 19 17:22:39 2008
**      by: Qt User Interface Compiler version 4.3.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_ADDSHAPEDIALOG_H
#define UI_ADDSHAPEDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>

class Ui_AddShapeDialog
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QDoubleSpinBox *x;
    QLabel *label_5;
    QDoubleSpinBox *width;
    QSpacerItem *spacerItem;
    QLabel *label_2;
    QDoubleSpinBox *y;
    QLabel *label_4;
    QDoubleSpinBox *height;
    QLabel *label_3;
    QComboBox *shapeType;
    QSpacerItem *spacerItem1;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *AddShapeDialog)
    {
    if (AddShapeDialog->objectName().isEmpty())
        AddShapeDialog->setObjectName(QString::fromUtf8("AddShapeDialog"));
    AddShapeDialog->resize(274, 178);
    gridLayout = new QGridLayout(AddShapeDialog);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    label = new QLabel(AddShapeDialog);
    label->setObjectName(QString::fromUtf8("label"));

    gridLayout->addWidget(label, 0, 0, 1, 1);

    x = new QDoubleSpinBox(AddShapeDialog);
    x->setObjectName(QString::fromUtf8("x"));
    x->setDecimals(3);
    x->setMaximum(999.9);

    gridLayout->addWidget(x, 0, 1, 1, 1);

    label_5 = new QLabel(AddShapeDialog);
    label_5->setObjectName(QString::fromUtf8("label_5"));

    gridLayout->addWidget(label_5, 0, 2, 1, 1);

    width = new QDoubleSpinBox(AddShapeDialog);
    width->setObjectName(QString::fromUtf8("width"));
    width->setValue(1);

    gridLayout->addWidget(width, 0, 3, 1, 1);

    spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    gridLayout->addItem(spacerItem, 0, 4, 1, 1);

    label_2 = new QLabel(AddShapeDialog);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    gridLayout->addWidget(label_2, 1, 0, 1, 1);

    y = new QDoubleSpinBox(AddShapeDialog);
    y->setObjectName(QString::fromUtf8("y"));
    y->setDecimals(3);
    y->setMaximum(999.9);

    gridLayout->addWidget(y, 1, 1, 1, 1);

    label_4 = new QLabel(AddShapeDialog);
    label_4->setObjectName(QString::fromUtf8("label_4"));

    gridLayout->addWidget(label_4, 1, 2, 1, 1);

    height = new QDoubleSpinBox(AddShapeDialog);
    height->setObjectName(QString::fromUtf8("height"));
    height->setValue(1);

    gridLayout->addWidget(height, 1, 3, 1, 1);

    label_3 = new QLabel(AddShapeDialog);
    label_3->setObjectName(QString::fromUtf8("label_3"));

    gridLayout->addWidget(label_3, 2, 0, 1, 1);

    shapeType = new QComboBox(AddShapeDialog);
    shapeType->setObjectName(QString::fromUtf8("shapeType"));

    gridLayout->addWidget(shapeType, 2, 1, 1, 3);

    spacerItem1 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    gridLayout->addItem(spacerItem1, 3, 2, 1, 1);

    buttonBox = new QDialogButtonBox(AddShapeDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);

    gridLayout->addWidget(buttonBox, 4, 0, 1, 4);


    retranslateUi(AddShapeDialog);
    QObject::connect(buttonBox, SIGNAL(accepted()), AddShapeDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), AddShapeDialog, SLOT(reject()));

    shapeType->setCurrentIndex(1);


    QMetaObject::connectSlotsByName(AddShapeDialog);
    } // setupUi

    void retranslateUi(QDialog *AddShapeDialog)
    {
    AddShapeDialog->setWindowTitle(QApplication::translate("AddShapeDialog", "Dialog", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("AddShapeDialog", "X:", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("AddShapeDialog", "Width:", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("AddShapeDialog", "Y:", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("AddShapeDialog", "Height:", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("AddShapeDialog", "Shape:", 0, QApplication::UnicodeUTF8));
    shapeType->clear();
    shapeType->insertItems(0, QStringList()
     << QApplication::translate("AddShapeDialog", "Rectangle", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("AddShapeDialog", "Cross", 0, QApplication::UnicodeUTF8)
     << QApplication::translate("AddShapeDialog", "Ellipse", 0, QApplication::UnicodeUTF8)
    );
    Q_UNUSED(AddShapeDialog);
    } // retranslateUi

};

namespace Ui {
    class AddShapeDialog: public Ui_AddShapeDialog {};
} // namespace Ui

#endif // UI_ADDSHAPEDIALOG_H
