/**
 ******************************************************************************
 *
 * @file       lineardialgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Airspeed Plugin Gadget options page
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   lineardial
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

#include "lineardialgadgetoptionspage.h"
#include "lineardialgadgetconfiguration.h"
#include "ui_lineardialgadgetoptionspage.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

LineardialGadgetOptionsPage::LineardialGadgetOptionsPage(LineardialGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
}

//creates options page widget (uses the UI file)
QWidget *LineardialGadgetOptionsPage::createPage(QWidget *parent)
{

    options_page = new Ui::LineardialGadgetOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    options_page->setupUi(optionsPageWidget);

    // Restore the contents from the settings:
    options_page->svgSourceFile->setText(m_config->getDialFile());
    options_page->minValue->setValue(m_config->getMin());
    options_page->maxValue->setValue(m_config->getMax());
    options_page->greenMin->setValue(m_config->getGreenMin());
    options_page->greenMax->setValue(m_config->getGreenMax());
    options_page->yellowMin->setValue(m_config->getYellowMin());
    options_page->yellowMax->setValue(m_config->getYellowMax());
    options_page->redMin->setValue(m_config->getRedMin());
    options_page->redMax->setValue(m_config->getRedMax());

    connect(options_page->loadFile, SIGNAL(clicked()), this, SLOT(on_loadFile_clicked()));

    return optionsPageWidget;
}
/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void LineardialGadgetOptionsPage::apply()
{
    m_config->setDialFile(options_page->svgSourceFile->text());
    m_config->setRange(options_page->minValue->value(),options_page->maxValue->value());
    m_config->setGreenRange(options_page->greenMin->value(),options_page->greenMax->value());
    m_config->setYellowRange(options_page->yellowMin->value(),options_page->yellowMax->value());
    m_config->setRedRange(options_page->redMin->value(),options_page->redMax->value());
}

/**

Opens an open file dialog.

*/
void LineardialGadgetOptionsPage::on_loadFile_clicked()
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


void LineardialGadgetOptionsPage::finish()
{

}
