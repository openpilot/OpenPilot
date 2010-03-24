/**
 ******************************************************************************
 *
 * @file       uavgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   coreplugin
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

#include "uavgadgetoptionspage.h"
#include "ui_uavgadgetoptionspage.h"
#include "uavgadgetinstancemanager.h"
#include "coreimpl.h"

#include <QtCore/QDebug>


using namespace Core;

UAVGadgetOptionsPage::UAVGadgetOptionsPage(IUAVGadgetConfiguration *config, QObject *parent) :
    Core::IOptionsPage(parent),
    m_config(config),
    m_id(config->name()),
    m_category(config->classId())
{
    UAVGadgetInstanceManager *im = ICore::instance()->uavGadgetInstanceManager();
    m_categoryTr = im->uavGadgetName(m_category);
}

QWidget *UAVGadgetOptionsPage::createPage(QWidget *parent)
{
    m_page = new Ui_TopOptionsPage();
    QWidget *w = new QWidget(parent);
    m_page->setupUi(w);
    if (m_config->locked()) {
        m_page->deleteButton->hide();
        m_page->lockCheckBox->hide();
        m_page->nameLineEdit->setDisabled(true);
    }
    m_page->lockCheckBox->hide(); //
    m_page->nameLineEdit->setText(m_id);
    QWidget *wi = widget();
    m_page->verticalLayout_4->addWidget(wi);

    w->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    return w;
}


