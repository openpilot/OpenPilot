/**
 ******************************************************************************
 *
 * @file       airspeedgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Airspeed Plugin Gadget options page
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   Airspeed
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

#include "airspeedgadgetoptionspage.h"
#include "airspeedgadgetconfiguration.h"
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

AirspeedGadgetOptionsPage::AirspeedGadgetOptionsPage(AirspeedGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
}

//creates options page widget
QWidget *AirspeedGadgetOptionsPage::createPage(QWidget *parent)
{
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    QVBoxLayout *vl = new QVBoxLayout();
    optionsPageWidget->setLayout(vl);

    //SVG file select layout and widget
    //choose file layout and widget
    QHBoxLayout *FileLayout = new QHBoxLayout;
    QWidget *FileWidget = new QWidget;
    FileWidget->setLayout(FileLayout);
    QWidget *label = new QLabel("Dial SVG:");
    svgSourceFile = new QLineEdit();
    QPushButton* loadfile = new QPushButton("Load File");
    loadfile->setMaximumWidth(80);
    FileLayout->addWidget(label);
    FileLayout->addWidget(svgSourceFile);
    FileLayout->addWidget(loadfile);


    QSpacerItem *spacer = new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding);

    //add partial widget to main widget
    vl->addWidget(FileWidget);
    vl->addSpacerItem(spacer);

    //clears comboboxes, if not every time the user enters options page the lists
    //duplicate
    svgSourceFile->clear();

    //connect signals to slots

    //fires when the user presses file button
    connect(loadfile, SIGNAL(clicked(bool)),
            this,SLOT(setOpenFileName()));

    // Restore the contents from the settings:
    svgSourceFile->setText(m_config->dialFile());

    return optionsPageWidget;
}
/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void AirspeedGadgetOptionsPage::apply()
{
    m_config->setDialFile(svgSourceFile->text());

}

/**

Opens an open file dialog.

*/
void AirspeedGadgetOptionsPage::setOpenFileName()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(this),
                                                    tr("QFileDialog::getOpenFileName()"),
                                                    svgSourceFile->text(),
                                                    tr("All Files (*);;SVG Files (*.svg)"),
                                                    &selectedFilter,
                                                    options);
    if (!fileName.isEmpty()) svgSourceFile->setText(fileName);

}


void AirspeedGadgetOptionsPage::finish()
{

}
