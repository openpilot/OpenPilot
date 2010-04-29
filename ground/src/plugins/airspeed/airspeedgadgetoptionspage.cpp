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
#include <QtGui/QComboBox>
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
    QWidget *widget = new QWidget;
    //main layout
    QVBoxLayout *vl = new QVBoxLayout();
    widget->setLayout(vl);

    //port layout and widget
    QHBoxLayout *portLayout = new QHBoxLayout();
    QWidget *x = new QWidget;
    x->setLayout(portLayout);
    QWidget *label = new QLabel("Dial SVG:");
    m_portCB = new QComboBox(parent);
    m_portCB->setMinimumSize(200,22);
    portLayout->addWidget(label);
    portLayout->addWidget(m_portCB);


    QSpacerItem *spacer = new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding);

    //add partial widget to main widget
    vl->addWidget(x);
    vl->addSpacerItem(spacer);

    //clears comboboxes, if not every time the user enters options page the lists
    //duplicate
    m_portCB->clear();

    return widget;
}
/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void AirspeedGadgetOptionsPage::apply()
{
    m_config->setDialFile(m_portCB->currentText());

}

void AirspeedGadgetOptionsPage::finish()
{

}
