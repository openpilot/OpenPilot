/********************************************************************************
** Form generated from reading UI file 'configgadgetwidget.ui'
**
** Created: Tue 18. May 19:44:42 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONFIGGADGETWIDGET_H
#define UI_CONFIGGADGETWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTreeView>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ConfigGadgetWidget
{
public:
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *sendButtun;
    QTreeView *treeView;
    QFrame *frame;

    void setupUi(QWidget *ConfigGadgetWidget)
    {
        if (ConfigGadgetWidget->objectName().isEmpty())
            ConfigGadgetWidget->setObjectName(QString::fromUtf8("ConfigGadgetWidget"));
        ConfigGadgetWidget->resize(500, 400);
        horizontalLayoutWidget = new QWidget(ConfigGadgetWidget);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(10, 350, 481, 41));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetMinimumSize);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        sendButtun = new QPushButton(horizontalLayoutWidget);
        sendButtun->setObjectName(QString::fromUtf8("sendButtun"));

        horizontalLayout->addWidget(sendButtun);

        treeView = new QTreeView(ConfigGadgetWidget);
        treeView->setObjectName(QString::fromUtf8("treeView"));
        treeView->setGeometry(QRect(10, 10, 221, 331));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(treeView->sizePolicy().hasHeightForWidth());
        treeView->setSizePolicy(sizePolicy);
        frame = new QFrame(ConfigGadgetWidget);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setGeometry(QRect(240, 10, 251, 331));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy1);
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);

        retranslateUi(ConfigGadgetWidget);

        QMetaObject::connectSlotsByName(ConfigGadgetWidget);
    } // setupUi

    void retranslateUi(QWidget *ConfigGadgetWidget)
    {
        ConfigGadgetWidget->setWindowTitle(QApplication::translate("ConfigGadgetWidget", "Form", 0, QApplication::UnicodeUTF8));
        sendButtun->setText(QApplication::translate("ConfigGadgetWidget", "Send", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ConfigGadgetWidget: public Ui_ConfigGadgetWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONFIGGADGETWIDGET_H
