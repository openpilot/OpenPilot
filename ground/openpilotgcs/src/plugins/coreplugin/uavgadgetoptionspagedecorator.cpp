/**
 ******************************************************************************
 *
 * @file       uavgadgetoptionspagedecorator.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup CorePlugin Core Plugin
 * @{
 * @brief The Core GCS plugin
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

#include "uavgadgetoptionspagedecorator.h"
#include "ui_uavgadgetoptionspage.h"
#include "uavgadgetinstancemanager.h"
#include "coreimpl.h"

#include <QtCore/QDebug>

using namespace Core;

UAVGadgetOptionsPageDecorator::UAVGadgetOptionsPageDecorator(IOptionsPage *page, IUAVGadgetConfiguration *config,
                                                             bool isSingleConfigurationGadget, QObject *parent) :
    Core::IOptionsPage(parent),
    m_optionsPage(page),
    m_config(config),
    m_isSingleConfigurationGadget(isSingleConfigurationGadget),
    m_id(config->name()),
    m_category(config->classId())
{
    m_optionsPage->setParent(this);
    m_instanceManager = ICore::instance()->uavGadgetInstanceManager();
    m_categoryTr = m_instanceManager->gadgetName(m_category);
}

QWidget *UAVGadgetOptionsPageDecorator::createPage(QWidget *parent)
{
    m_page = new Ui_TopOptionsPage();
    QWidget *w = new QWidget(parent);
    m_page->setupUi(w);
    if (m_config->locked()) {
        m_page->deleteButton->hide();
        m_page->lockCheckBox->hide();
        m_page->nameLineEdit->setDisabled(true);
    }
    if (!m_instanceManager->canDeleteConfiguration(m_config))
        m_page->deleteButton->setDisabled(true);
    m_page->lockCheckBox->hide(); //
    m_page->nameLineEdit->setText(m_id);

    QWidget *wi = m_optionsPage->createPage(w);
    wi->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_page->verticalLayout_4->addWidget(wi);

    // For some gadgets it might not make sense to have multiple configurations
    if (m_isSingleConfigurationGadget) {
        m_page->configurationBox->hide();
    }

    connect(m_page->cloneButton, SIGNAL(clicked()), this, SLOT(cloneConfiguration()));
    connect(m_page->deleteButton, SIGNAL(clicked()), this, SLOT(deleteConfiguration()));
    connect(m_page->nameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)));

    return w;
}

void UAVGadgetOptionsPageDecorator::apply()
{
    m_id = m_config->provisionalName();
    m_optionsPage->apply();
    m_instanceManager->applyChanges(m_config);
}

void UAVGadgetOptionsPageDecorator::finish()
{
    m_optionsPage->finish();
}

void UAVGadgetOptionsPageDecorator::cloneConfiguration()
{
    m_instanceManager->cloneConfiguration(m_config);
}

void UAVGadgetOptionsPageDecorator::deleteConfiguration()
{
    m_instanceManager->deleteConfiguration(m_config);
}

void UAVGadgetOptionsPageDecorator::textEdited(QString name)
{
    m_config->setProvisionalName(name);
    m_instanceManager->configurationNameEdited(name);
}

