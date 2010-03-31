/**
 ******************************************************************************
 *
 * @file       modelviewgadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   modelview
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

#include "modelviewgadgetoptionspage.h"
#include "modelviewgadgetconfiguration.h"


ModelViewGadgetOptionsPage::ModelViewGadgetOptionsPage(ModelViewGadgetConfiguration *config, QObject *parent) :
    IOptionsPage(parent),
    m_config(config)
{
}

QWidget *ModelViewGadgetOptionsPage::createPage(QWidget *parent)
{
    QWidget *widget = new QWidget;
    QVBoxLayout *vl = new QVBoxLayout();
    widget->setLayout(vl);

    QWidget *label = new QLabel("3D Object File:");
    vl->addWidget(label);
    
    m_acFileLabel = new QLabel(m_config->acFilename());
    QWidget* acPushbutton = new QPushButton("Change model");
    vl->addWidget(m_acFileLabel);
    vl->addWidget(acPushbutton);

    QSpacerItem *spacer = new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding);
    vl->addSpacerItem(spacer);

    label = new QLabel("Background image file:");
    vl->addWidget(label);
    
    m_bgFileLabel = new QLabel(m_config->bgFilename());
    QWidget* bgPushbutton = new QPushButton("Change background");
    vl->addWidget(m_bgFileLabel);
    vl->addWidget(bgPushbutton);

    QSpacerItem *spacer2 = new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding);
    vl->addSpacerItem(spacer2);

    connect(acPushbutton, SIGNAL(clicked()), this, SLOT(changeAC()) );
    connect(bgPushbutton, SIGNAL(clicked()), this, SLOT(changeBG()) );

    return widget;
}

void ModelViewGadgetOptionsPage::apply()
{
    //m_config->setAcFilename(m_acFileName->selectedFiles());
    //m_config->setBgFilename(m_bgFileName->selectedFiles());
}

void ModelViewGadgetOptionsPage::finish()
{
}

void ModelViewGadgetOptionsPage::changeAC()
{
    QString ac = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(this), 
	tr("Model 3D File"), "../artwork/", tr("3D File (*.dae)") );
    m_config->setAcFilename(ac);
    m_acFileLabel->setText(ac);
}

void ModelViewGadgetOptionsPage::changeBG()
{
    QString bg = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(this), 
	tr("Background Image File"), "../artwork", tr("Image Files (*.png *.jpg *.bmp)") );
    m_config->setBgFilename(bg);
    m_bgFileLabel->setText(bg);
}
