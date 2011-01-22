/**
 ******************************************************************************
 *
 * @file       pipxtremegadget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @{
 * @brief The YModem protocol serial uploader plugin
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

#ifndef PIPXTREMEGADGET_H
#define PIPXTREMEGADGET_H

#include <coreplugin/iuavgadget.h>
#include "pipxtremegadgetwidget.h"

class IUAVGadget;
class QWidget;
class QString;
class PipXtremeGadgetWidget;

using namespace Core;

class PipXtremeGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    PipXtremeGadget(QString classId, PipXtremeGadgetWidget *widget, QWidget *parent = 0);
    ~PipXtremeGadget();

    QWidget *widget() { return m_widget; }
    void loadConfiguration(IUAVGadgetConfiguration* config);

private:
    PipXtremeGadgetWidget *m_widget;
};

#endif

