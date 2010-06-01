/**
******************************************************************************
*
* @file       ui_mainwindow.h
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
*             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
* @brief      
* @see        The GNU Public License (GPL) Version 3
* @defgroup   DocumentationHelper
* @{
* 
*****************************************************************************/
/* 
* This program is free software; you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by 
* the Free Software Foundation; either version 3 of the License, or 
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful, but 
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
* for more details.
* 
* You should have received a copy of the GNU General Public License along 
* with this program; if not, write to the Free Software Foundation, Inc., 
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Sun 30. May 20:43:26 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTextEdit>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QPushButton *goBT;
    QLabel *cfileL;
    QLabel *cpathL;
    QPushButton *cpathBT;
    QLabel *label_2;
    QCheckBox *nameCB;
    QCheckBox *bockifCB;
    QTextEdit *textEdit;
    QCheckBox *licenseCB;
    QCheckBox *confirmCB;
    QTextEdit *output;
    QLabel *label;
    QLineEdit *defgroup;
    QLineEdit *namespa;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(600, 455);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        goBT = new QPushButton(centralWidget);
        goBT->setObjectName(QString::fromUtf8("goBT"));
        goBT->setGeometry(QRect(110, 10, 75, 23));
        cfileL = new QLabel(centralWidget);
        cfileL->setObjectName(QString::fromUtf8("cfileL"));
        cfileL->setGeometry(QRect(20, 380, 561, 16));
        QFont font;
        font.setPointSize(10);
        cfileL->setFont(font);
        cpathL = new QLabel(centralWidget);
        cpathL->setObjectName(QString::fromUtf8("cpathL"));
        cpathL->setGeometry(QRect(20, 360, 561, 16));
        cpathL->setFont(font);
        cpathBT = new QPushButton(centralWidget);
        cpathBT->setObjectName(QString::fromUtf8("cpathBT"));
        cpathBT->setGeometry(QRect(20, 10, 75, 23));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(25, 177, 131, 16));
        nameCB = new QCheckBox(centralWidget);
        nameCB->setObjectName(QString::fromUtf8("nameCB"));
        nameCB->setGeometry(QRect(20, 80, 111, 17));
        bockifCB = new QCheckBox(centralWidget);
        bockifCB->setObjectName(QString::fromUtf8("bockifCB"));
        bockifCB->setGeometry(QRect(20, 140, 211, 17));
        textEdit = new QTextEdit(centralWidget);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        textEdit->setGeometry(QRect(20, 190, 201, 161));
        licenseCB = new QCheckBox(centralWidget);
        licenseCB->setObjectName(QString::fromUtf8("licenseCB"));
        licenseCB->setGeometry(QRect(20, 40, 111, 17));
        confirmCB = new QCheckBox(centralWidget);
        confirmCB->setObjectName(QString::fromUtf8("confirmCB"));
        confirmCB->setGeometry(QRect(20, 120, 141, 17));
        output = new QTextEdit(centralWidget);
        output->setObjectName(QString::fromUtf8("output"));
        output->setGeometry(QRect(245, 30, 341, 321));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(250, 10, 46, 13));
        defgroup = new QLineEdit(centralWidget);
        defgroup->setObjectName(QString::fromUtf8("defgroup"));
        defgroup->setEnabled(false);
        defgroup->setGeometry(QRect(20, 57, 113, 20));
        namespa = new QLineEdit(centralWidget);
        namespa->setObjectName(QString::fromUtf8("namespa"));
        namespa->setEnabled(false);
        namespa->setGeometry(QRect(20, 97, 113, 20));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 21));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);
        QObject::connect(licenseCB, SIGNAL(toggled(bool)), defgroup, SLOT(setEnabled(bool)));
        QObject::connect(nameCB, SIGNAL(toggled(bool)), namespa, SLOT(setEnabled(bool)));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        goBT->setText(QApplication::translate("MainWindow", "Go", 0, QApplication::UnicodeUTF8));
        cfileL->setText(QApplication::translate("MainWindow", "Current File:", 0, QApplication::UnicodeUTF8));
        cpathL->setText(QApplication::translate("MainWindow", "Current Path:", 0, QApplication::UnicodeUTF8));
        cpathBT->setText(QApplication::translate("MainWindow", "Choose Path", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "Created #IF statements", 0, QApplication::UnicodeUTF8));
        nameCB->setText(QApplication::translate("MainWindow", "Create NameSpace", 0, QApplication::UnicodeUTF8));
        bockifCB->setText(QApplication::translate("MainWindow", "Create Block #if in qDebug Statements", 0, QApplication::UnicodeUTF8));
        licenseCB->setText(QApplication::translate("MainWindow", "Write License Info", 0, QApplication::UnicodeUTF8));
        confirmCB->setText(QApplication::translate("MainWindow", "Confirm Before Change", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Output", 0, QApplication::UnicodeUTF8));
        defgroup->setText(QApplication::translate("MainWindow", "DefGroup", 0, QApplication::UnicodeUTF8));
        namespa->setText(QApplication::translate("MainWindow", "namespace", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
