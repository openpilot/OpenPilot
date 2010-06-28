/********************************************************************************
** Form generated from reading UI file 'IPconnectionoptionspage.ui'
**
** Created: Sun Jun 27 12:45:24 2010
**      by: Qt User Interface Compiler version 4.6.2
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
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_IPconnectionOptionsPage
{
public:
    QGridLayout *gridLayout;
    QSpacerItem *horizontalSpacer;
    QRadioButton *UseTCP;
    QRadioButton *UseUDP;
    QSpinBox *Port;
    QLabel *label_3;
    QLineEdit *HostName;
    QLabel *label_2;

    void setupUi(QWidget *IPconnectionOptionsPage)
    {
        if (IPconnectionOptionsPage->objectName().isEmpty())
            IPconnectionOptionsPage->setObjectName(QString::fromUtf8("IPconnectionOptionsPage"));
        IPconnectionOptionsPage->resize(388, 300);
        gridLayout = new QGridLayout(IPconnectionOptionsPage);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 0, 3, 1, 1);

        UseTCP = new QRadioButton(IPconnectionOptionsPage);
        UseTCP->setObjectName(QString::fromUtf8("UseTCP"));

        gridLayout->addWidget(UseTCP, 2, 2, 1, 1);

        UseUDP = new QRadioButton(IPconnectionOptionsPage);
        UseUDP->setObjectName(QString::fromUtf8("UseUDP"));

        gridLayout->addWidget(UseUDP, 3, 2, 1, 1);

        Port = new QSpinBox(IPconnectionOptionsPage);
        Port->setObjectName(QString::fromUtf8("Port"));
        Port->setMinimum(1);
        Port->setMaximum(999999);

        gridLayout->addWidget(Port, 4, 2, 1, 1);

        label_3 = new QLabel(IPconnectionOptionsPage);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 4, 1, 1, 1);

        HostName = new QLineEdit(IPconnectionOptionsPage);
        HostName->setObjectName(QString::fromUtf8("HostName"));

        gridLayout->addWidget(HostName, 0, 2, 1, 1);

        label_2 = new QLabel(IPconnectionOptionsPage);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 1, 1, 1);


        retranslateUi(IPconnectionOptionsPage);

        QMetaObject::connectSlotsByName(IPconnectionOptionsPage);
    } // setupUi

    void retranslateUi(QWidget *IPconnectionOptionsPage)
    {
        IPconnectionOptionsPage->setWindowTitle(QApplication::translate("IPconnectionOptionsPage", "Form", 0, QApplication::UnicodeUTF8));
        UseTCP->setText(QApplication::translate("IPconnectionOptionsPage", "TCP connection", 0, QApplication::UnicodeUTF8));
        UseUDP->setText(QApplication::translate("IPconnectionOptionsPage", "UDP connection", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("IPconnectionOptionsPage", "Port", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("IPconnectionOptionsPage", "Host Name/Number", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class IPconnectionOptionsPage: public Ui_IPconnectionOptionsPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCPTELEMETRYOPTIONSPAGE_H
