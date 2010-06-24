/********************************************************************************
** Form generated from reading UI file 'TCPtelemetryoptionspage.ui'
**
** Created: Wed 23. Jun 00:36:03 2010
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TCPTELEMETRYOPTIONSPAGE_H
#define UI_TCPTELEMETRYOPTIONSPAGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TCPtelemetryOptionsPage
{
public:
    QGridLayout *gridLayout;
    QSpacerItem *horizontalSpacer;
    QSpinBox *Port;
    QLabel *label_3;
    QLineEdit *HostName;
    QLabel *label_2;

    void setupUi(QWidget *TCPtelemetryOptionsPage)
    {
        if (TCPtelemetryOptionsPage->objectName().isEmpty())
            TCPtelemetryOptionsPage->setObjectName(QString::fromUtf8("TCPtelemetryOptionsPage"));
        TCPtelemetryOptionsPage->resize(388, 300);
        gridLayout = new QGridLayout(TCPtelemetryOptionsPage);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 0, 3, 1, 1);

        Port = new QSpinBox(TCPtelemetryOptionsPage);
        Port->setObjectName(QString::fromUtf8("Port"));
        Port->setMinimum(1);
        Port->setMaximum(999999);

        gridLayout->addWidget(Port, 4, 2, 1, 1);

        label_3 = new QLabel(TCPtelemetryOptionsPage);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 4, 1, 1, 1);

        HostName = new QLineEdit(TCPtelemetryOptionsPage);
        HostName->setObjectName(QString::fromUtf8("HostName"));

        gridLayout->addWidget(HostName, 0, 2, 1, 1);

        label_2 = new QLabel(TCPtelemetryOptionsPage);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 1, 1, 1);


        retranslateUi(TCPtelemetryOptionsPage);

        QMetaObject::connectSlotsByName(TCPtelemetryOptionsPage);
    } // setupUi

    void retranslateUi(QWidget *TCPtelemetryOptionsPage)
    {
        TCPtelemetryOptionsPage->setWindowTitle(QApplication::translate("TCPtelemetryOptionsPage", "Form", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("TCPtelemetryOptionsPage", "Port", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("TCPtelemetryOptionsPage", "Host Name/Number", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TCPtelemetryOptionsPage: public Ui_TCPtelemetryOptionsPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCPTELEMETRYOPTIONSPAGE_H
