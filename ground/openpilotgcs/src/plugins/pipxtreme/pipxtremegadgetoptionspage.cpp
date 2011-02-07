/**
 ******************************************************************************
 *
 * @file       pipxtremegadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @{
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

#include "pipxtremegadgetoptionspage.h"
#include "pipxtremegadgetconfiguration.h"
#include "ui_pipxtremegadgetoptionspage.h"

PipXtremeGadgetOptionsPage::PipXtremeGadgetOptionsPage(PipXtremeGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
		options_page(NULL),
		m_config(config)
{
}

//creates options page widget
QWidget *PipXtremeGadgetOptionsPage::createPage(QWidget *parent)
{
//    QWidget *widget = new QWidget;
//    return widget;

	options_page = new Ui::PipXtremeGadgetOptionsPage();
	QWidget *optionsPageWidget = new QWidget;
	options_page->setupUi(optionsPageWidget);




	return optionsPageWidget;
}
/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void PipXtremeGadgetOptionsPage::apply()
{

}

void PipXtremeGadgetOptionsPage::finish()
{
	if (options_page)
	{
		delete options_page;
		options_page = NULL;
	}
}


