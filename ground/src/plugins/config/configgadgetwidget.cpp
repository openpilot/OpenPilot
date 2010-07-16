/**
 ******************************************************************************
 *
 * @file       configgadgetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware
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
#include "configgadgetwidget.h"
#include "ui_configgadget.h"


#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

ConfigGadgetWidget::ConfigGadgetWidget(QWidget *parent) : QWidget(parent)
{
    m_config = new Ui_ConfigGadget();
    m_config->setupUi(this);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
//    this->
//    setToolTip(tr("Choose a gadget to display in this view.\n") +
//            tr("You can also split this view in two.\n\n") +
//            tr("Maybe you first have to choose Show Toolbars in the Window menu."));

}

ConfigGadgetWidget::~ConfigGadgetWidget()
{
   // Do nothing
}

void ConfigGadgetWidget::resizeEvent(QResizeEvent *event)
{

    QWidget::resizeEvent(event);
}
