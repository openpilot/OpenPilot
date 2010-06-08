/**
 ******************************************************************************
 *
 * @file       scopegadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Scope Plugin Gadget options page
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   Scope
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

#include "scopegadgetoptionspage.h"
#include "scopegadgetconfiguration.h"
#include "ui_scopegadgetoptionspage.h"

#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavdataobject.h"

ScopeGadgetOptionsPage::ScopeGadgetOptionsPage(ScopeGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
    //nothing to do here...
}

//creates options page widget (uses the UI file)
QWidget* ScopeGadgetOptionsPage::createPage(QWidget *parent)
{
    options_page = new Ui::ScopeGadgetOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    options_page->setupUi(optionsPageWidget);

    //TODO: Rest of the init stuff here


    return optionsPageWidget;
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void ScopeGadgetOptionsPage::apply()
{

    //Apply configuration changes
    // m_config->setDialFile(options_page->svgSourceFile->text());

}

void ScopeGadgetOptionsPage::finish()
{

}
