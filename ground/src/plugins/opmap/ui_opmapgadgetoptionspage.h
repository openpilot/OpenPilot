/********************************************************************************
** Form generated from reading UI file 'opmapgadgetoptionspage.ui'
**
** Created: Fri 2. Jul 13:44:00 2010
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OPMAPGADGETOPTIONSPAGE_H
#define UI_OPMAPGADGETOPTIONSPAGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OPMapGadgetOptionsPage
{
public:
    QGridLayout *gridLayout;
    QGridLayout *gridLayout_3;
    QLabel *label_4;
    QDoubleSpinBox *longitudeSpinBox;
    QCheckBox *checkBoxUseOpenGL;
    QDoubleSpinBox *latitudeSpinBox;
    QLabel *label_3;
    QSpinBox *zoomSpinBox;
    QLabel *label_2;
    QComboBox *providerComboBox;
    QLabel *label;
    QCheckBox *checkBoxShowTileGridLines;
    QSpacerItem *horizontalSpacer_4;
    QSpacerItem *verticalSpacer;
    QFrame *line;
    QLabel *label_7;
    QFrame *line_2;
    QGridLayout *gridLayout_5;
    QPushButton *pushButtonCacheDefaults;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_3;
    QLabel *label_6;
    QComboBox *accessModeComboBox;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_5;
    QLineEdit *lineEditCacheLocation;
    QPushButton *pushButtonCacheLocation;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QCheckBox *checkBoxUseMemoryCache;

    void setupUi(QWidget *OPMapGadgetOptionsPage)
    {
        if (OPMapGadgetOptionsPage->objectName().isEmpty())
            OPMapGadgetOptionsPage->setObjectName(QString::fromUtf8("OPMapGadgetOptionsPage"));
        OPMapGadgetOptionsPage->resize(521, 373);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(OPMapGadgetOptionsPage->sizePolicy().hasHeightForWidth());
        OPMapGadgetOptionsPage->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(OPMapGadgetOptionsPage);
        gridLayout->setContentsMargins(5, 5, 5, 5);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        label_4 = new QLabel(OPMapGadgetOptionsPage);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(label_4, 4, 2, 1, 1);

        longitudeSpinBox = new QDoubleSpinBox(OPMapGadgetOptionsPage);
        longitudeSpinBox->setObjectName(QString::fromUtf8("longitudeSpinBox"));
        longitudeSpinBox->setMinimumSize(QSize(120, 0));
        longitudeSpinBox->setFrame(true);
        longitudeSpinBox->setDecimals(8);
        longitudeSpinBox->setMinimum(-180);
        longitudeSpinBox->setMaximum(180);

        gridLayout_3->addWidget(longitudeSpinBox, 4, 3, 1, 1);

        checkBoxUseOpenGL = new QCheckBox(OPMapGadgetOptionsPage);
        checkBoxUseOpenGL->setObjectName(QString::fromUtf8("checkBoxUseOpenGL"));
        checkBoxUseOpenGL->setEnabled(true);
        checkBoxUseOpenGL->setLayoutDirection(Qt::RightToLeft);

        gridLayout_3->addWidget(checkBoxUseOpenGL, 5, 3, 1, 1);

        latitudeSpinBox = new QDoubleSpinBox(OPMapGadgetOptionsPage);
        latitudeSpinBox->setObjectName(QString::fromUtf8("latitudeSpinBox"));
        latitudeSpinBox->setMinimumSize(QSize(120, 0));
        latitudeSpinBox->setFrame(true);
        latitudeSpinBox->setDecimals(8);
        latitudeSpinBox->setMinimum(-90);
        latitudeSpinBox->setMaximum(90);

        gridLayout_3->addWidget(latitudeSpinBox, 2, 3, 1, 1);

        label_3 = new QLabel(OPMapGadgetOptionsPage);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(label_3, 2, 2, 1, 1);

        zoomSpinBox = new QSpinBox(OPMapGadgetOptionsPage);
        zoomSpinBox->setObjectName(QString::fromUtf8("zoomSpinBox"));
        zoomSpinBox->setMaximumSize(QSize(50, 16777215));
        zoomSpinBox->setFrame(true);
        zoomSpinBox->setMinimum(2);
        zoomSpinBox->setMaximum(18);

        gridLayout_3->addWidget(zoomSpinBox, 1, 3, 1, 1);

        label_2 = new QLabel(OPMapGadgetOptionsPage);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(label_2, 1, 2, 1, 1);

        providerComboBox = new QComboBox(OPMapGadgetOptionsPage);
        providerComboBox->setObjectName(QString::fromUtf8("providerComboBox"));
        providerComboBox->setCursor(QCursor(Qt::OpenHandCursor));
        providerComboBox->setMaxVisibleItems(15);
        providerComboBox->setFrame(true);

        gridLayout_3->addWidget(providerComboBox, 0, 3, 1, 1);

        label = new QLabel(OPMapGadgetOptionsPage);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(label, 0, 2, 1, 1);

        checkBoxShowTileGridLines = new QCheckBox(OPMapGadgetOptionsPage);
        checkBoxShowTileGridLines->setObjectName(QString::fromUtf8("checkBoxShowTileGridLines"));
        checkBoxShowTileGridLines->setEnabled(true);
        checkBoxShowTileGridLines->setMouseTracking(true);
        checkBoxShowTileGridLines->setLayoutDirection(Qt::RightToLeft);

        gridLayout_3->addWidget(checkBoxShowTileGridLines, 6, 3, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_4, 2, 0, 1, 1);


        gridLayout->addLayout(gridLayout_3, 1, 0, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 10, 0, 1, 1);

        line = new QFrame(OPMapGadgetOptionsPage);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line, 2, 0, 1, 1);

        label_7 = new QLabel(OPMapGadgetOptionsPage);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_7, 3, 0, 1, 1);

        line_2 = new QFrame(OPMapGadgetOptionsPage);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_2, 9, 0, 1, 1);

        gridLayout_5 = new QGridLayout();
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        pushButtonCacheDefaults = new QPushButton(OPMapGadgetOptionsPage);
        pushButtonCacheDefaults->setObjectName(QString::fromUtf8("pushButtonCacheDefaults"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pushButtonCacheDefaults->sizePolicy().hasHeightForWidth());
        pushButtonCacheDefaults->setSizePolicy(sizePolicy1);
        pushButtonCacheDefaults->setCursor(QCursor(Qt::OpenHandCursor));
        pushButtonCacheDefaults->setCheckable(false);
        pushButtonCacheDefaults->setChecked(false);
        pushButtonCacheDefaults->setFlat(false);

        gridLayout_5->addWidget(pushButtonCacheDefaults, 5, 2, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_2, 5, 0, 1, 1);


        gridLayout->addLayout(gridLayout_5, 7, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        label_6 = new QLabel(OPMapGadgetOptionsPage);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_6);

        accessModeComboBox = new QComboBox(OPMapGadgetOptionsPage);
        accessModeComboBox->setObjectName(QString::fromUtf8("accessModeComboBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(accessModeComboBox->sizePolicy().hasHeightForWidth());
        accessModeComboBox->setSizePolicy(sizePolicy2);
        accessModeComboBox->setMinimumSize(QSize(160, 0));
        accessModeComboBox->setCursor(QCursor(Qt::OpenHandCursor));
        accessModeComboBox->setFrame(true);

        horizontalLayout->addWidget(accessModeComboBox);


        gridLayout->addLayout(horizontalLayout, 4, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_5 = new QLabel(OPMapGadgetOptionsPage);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_3->addWidget(label_5);

        lineEditCacheLocation = new QLineEdit(OPMapGadgetOptionsPage);
        lineEditCacheLocation->setObjectName(QString::fromUtf8("lineEditCacheLocation"));
        lineEditCacheLocation->setAutoFillBackground(false);
        lineEditCacheLocation->setMaxLength(512);

        horizontalLayout_3->addWidget(lineEditCacheLocation);

        pushButtonCacheLocation = new QPushButton(OPMapGadgetOptionsPage);
        pushButtonCacheLocation->setObjectName(QString::fromUtf8("pushButtonCacheLocation"));
        pushButtonCacheLocation->setCursor(QCursor(Qt::OpenHandCursor));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/core/images/dir.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButtonCacheLocation->setIcon(icon);
        pushButtonCacheLocation->setFlat(true);

        horizontalLayout_3->addWidget(pushButtonCacheLocation);


        gridLayout->addLayout(horizontalLayout_3, 6, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        checkBoxUseMemoryCache = new QCheckBox(OPMapGadgetOptionsPage);
        checkBoxUseMemoryCache->setObjectName(QString::fromUtf8("checkBoxUseMemoryCache"));
        checkBoxUseMemoryCache->setEnabled(true);
        checkBoxUseMemoryCache->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_2->addWidget(checkBoxUseMemoryCache);


        gridLayout->addLayout(horizontalLayout_2, 5, 0, 1, 1);


        retranslateUi(OPMapGadgetOptionsPage);

        QMetaObject::connectSlotsByName(OPMapGadgetOptionsPage);
    } // setupUi

    void retranslateUi(QWidget *OPMapGadgetOptionsPage)
    {
        OPMapGadgetOptionsPage->setWindowTitle(QApplication::translate("OPMapGadgetOptionsPage", "Form", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("OPMapGadgetOptionsPage", "Default longitude ", 0, QApplication::UnicodeUTF8));
        checkBoxUseOpenGL->setText(QApplication::translate("OPMapGadgetOptionsPage", "Use OpenGL", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("OPMapGadgetOptionsPage", "Default latitude ", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("OPMapGadgetOptionsPage", "Default zoom ", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("OPMapGadgetOptionsPage", "Map type ", 0, QApplication::UnicodeUTF8));
        checkBoxShowTileGridLines->setText(QApplication::translate("OPMapGadgetOptionsPage", "Show Tile Grid Lines", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("OPMapGadgetOptionsPage", "Server and Cache", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        pushButtonCacheDefaults->setToolTip(QApplication::translate("OPMapGadgetOptionsPage", "Restore default server and cache settings", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        pushButtonCacheDefaults->setText(QApplication::translate("OPMapGadgetOptionsPage", " Default ", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("OPMapGadgetOptionsPage", "Access mode ", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("OPMapGadgetOptionsPage", "Cache location ", 0, QApplication::UnicodeUTF8));
        pushButtonCacheLocation->setText(QString());
        checkBoxUseMemoryCache->setText(QApplication::translate("OPMapGadgetOptionsPage", "Use Memory Cache", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class OPMapGadgetOptionsPage: public Ui_OPMapGadgetOptionsPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPMAPGADGETOPTIONSPAGE_H
