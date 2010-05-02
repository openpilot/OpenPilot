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
#include "ui_airspeedgadgetoptionspage.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

AirspeedGadgetOptionsPage::AirspeedGadgetOptionsPage(AirspeedGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
}

//creates options page widget (uses the UI file)
QWidget *AirspeedGadgetOptionsPage::createPage(QWidget *parent)
{

    options_page = new Ui::AirspeedGadgetOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    options_page->setupUi(optionsPageWidget);

    // Restore the contents from the settings:
    options_page->svgSourceFile->setText(m_config->dialFile());
    options_page->backgroundID->setText(m_config->dialBackground());
    options_page->foregroundID->setText(m_config->dialForeground());
    options_page->needle1ID->setText(m_config->dialNeedle1());
    options_page->needle2ID->setText(m_config->dialNeedle2());

    connect(options_page->loadFile, SIGNAL(clicked()), this, SLOT(on_loadFile_clicked()));


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
    m_config->setDialFile(options_page->svgSourceFile->text());

}

/**

Opens an open file dialog.

*/
void AirspeedGadgetOptionsPage::on_loadFile_clicked()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(this),
                                                    tr("QFileDialog::getOpenFileName()"),
                                                    options_page->svgSourceFile->text(),
                                                    tr("All Files (*);;SVG Files (*.svg)"),
                                                    &selectedFilter,
                                                    options);
    if (!fileName.isEmpty()) options_page->svgSourceFile->setText(fileName);

}


void AirspeedGadgetOptionsPage::finish()
{

}
