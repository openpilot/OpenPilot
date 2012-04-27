/**
 ******************************************************************************
 *
 * @file       configtxpidwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to configure TxPID module
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
#ifndef CONFIGTXPIDWIDGET_H
#define CONFIGTXPIDWIDGET_H

#include "ui_txpid.h"
#include "configtaskwidget.h"

class ConfigTxPIDWidget : public ConfigTaskWidget
{
    Q_OBJECT

public:
    ConfigTxPIDWidget(QWidget *parent = 0);
    ~ConfigTxPIDWidget();
private:
    Ui_TxPIDWidget *m_txpid;

private slots:
    void refreshValues();
    void applySettings();
    void saveSettings();
};

#endif // CONFIGTXPIDWIDGET_H
