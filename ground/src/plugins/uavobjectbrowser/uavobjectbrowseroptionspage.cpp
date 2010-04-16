/**
 ******************************************************************************
 *
 * @file       uavobjectbrowseroptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   uavobjectbrowser
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

#include "uavobjectbrowseroptionspage.h"
#include "uavobjectbrowserconfiguration.h"
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QPushButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QColorDialog>


UAVObjectBrowserOptionsPage::UAVObjectBrowserOptionsPage(UAVObjectBrowserConfiguration *config, QObject *parent) :
    IOptionsPage(parent),
    m_config(config)
{
}

QWidget *UAVObjectBrowserOptionsPage::createPage(QWidget *parent)
{
    QWidget *widget = new QWidget;
    QVBoxLayout *vl = new QVBoxLayout();
    widget->setLayout(vl);

    QHBoxLayout *recentColorLayout = new QHBoxLayout();
    QWidget *ru = new QWidget;
    ru->setLayout(recentColorLayout);
    QWidget *label = new QLabel(tr("Recently updated highlight color:"));
    QHBoxLayout *rubLayout = new QHBoxLayout();
    QWidget *rub = new QWidget;
    rub->setLayout(rubLayout);
    m_ruLabel = new QLabel("     ");
    m_ruLabel->setMinimumWidth(40);
    m_ruButton = new QPushButton(tr("Choose"));
    rubLayout->addWidget(m_ruLabel);
    rubLayout->addWidget(m_ruButton);
    recentColorLayout->addWidget(label);
    recentColorLayout->addWidget(rub);

    QHBoxLayout *manualColorLayout = new QHBoxLayout();
    QWidget *mc = new QWidget;
    mc->setLayout(manualColorLayout);
    label = new QLabel(tr("Manually changed color:"));
    QHBoxLayout *mcbLayout = new QHBoxLayout();
    QWidget *mcb = new QWidget;
    mcb->setLayout(mcbLayout);
    m_mcLabel = new QLabel("     ");
    m_mcLabel->setMinimumWidth(40);
    m_mcButton = new QPushButton(tr("Choose"));
    mcbLayout->addWidget(m_mcLabel);
    mcbLayout->addWidget(m_mcButton);
    manualColorLayout->addWidget(label);
    manualColorLayout->addWidget(mcb);

    QHBoxLayout *timeoutLayout = new QHBoxLayout();
    QWidget *x = new QWidget;
    x->setLayout(timeoutLayout);
    label = new QLabel("Recently updated highlight timeout (ms):");
    m_timeoutSpin = new QSpinBox();
    m_timeoutSpin->setSingleStep(100);
    m_timeoutSpin->setMaximum(1000000000);
    timeoutLayout->addWidget(label);
    timeoutLayout->addWidget(m_timeoutSpin);

    QSpacerItem *spacer = new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding);

    vl->addWidget(ru);
    vl->addWidget(mc);
    vl->addWidget(x);
    vl->addSpacerItem(spacer);

    m_ruColor = m_config->recentlyUpdatedColor();
    QString s = QString("background-color: %1").arg(m_ruColor.name());
    m_ruLabel->setStyleSheet(s);
    m_mcColor = m_config->manuallyChangedColor();
    s = QString("background-color: %1").arg(m_mcColor.name());
    m_mcLabel->setStyleSheet(s);
    m_timeoutSpin->setValue(m_config->recentlyUpdatedTimeout());

    connect(m_ruButton, SIGNAL(clicked()), this, SLOT(ruButtonClicked()));
    connect(m_mcButton, SIGNAL(clicked()), this, SLOT(mcButtonClicked()));

    return widget;

}

void UAVObjectBrowserOptionsPage::apply()
{
    m_config->setRecentlyUpdatedColor(m_ruColor);
    m_config->setManuallyChangedColor(m_mcColor);
    m_config->setRecentlyUpdatedTimeout(m_timeoutSpin->value());
}

void UAVObjectBrowserOptionsPage::finish()
{
    disconnect(m_ruButton, SIGNAL(clicked()), this, SLOT(ruButtonClicked()));
    disconnect(m_mcButton, SIGNAL(clicked()), this, SLOT(mcButtonClicked()));
    delete m_ruButton;
    delete m_mcButton;
    delete m_ruLabel;
    delete m_mcLabel;
    delete m_timeoutSpin;
}

void UAVObjectBrowserOptionsPage::ruButtonClicked()
{
    QColor c = QColorDialog::getColor(m_ruColor);
    m_ruColor = c.isValid() ? c : m_ruColor;
    QString s = QString("background-color: %1").arg(m_ruColor.name());
    m_ruLabel->setStyleSheet(s);
}

void UAVObjectBrowserOptionsPage::mcButtonClicked()
{
    QColor c = QColorDialog::getColor(m_mcColor);
    m_mcColor = c.isValid() ? c : m_mcColor;
    QString s = QString("background-color: %1").arg(m_mcColor.name());
    m_mcLabel->setStyleSheet(s);
}

