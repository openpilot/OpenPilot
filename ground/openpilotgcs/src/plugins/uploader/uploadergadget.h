/**
 ******************************************************************************
 *
 * @file       uploadergadget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup YModemUploader YModem Serial Uploader Plugin
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

#ifndef UPLOADERGADGET_H
#define UPLOADERGADGET_H

#include <coreplugin/iuavgadget.h>
#include "uploadergadgetwidget.h"
#include "uploader_global.h"

class IUAVGadget;
class QWidget;
class QString;
class UploaderGadgetWidget;

using namespace Core;

class UPLOADER_EXPORT UploaderGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    UploaderGadget(QString classId, UploaderGadgetWidget *widget, QWidget *parent = 0);
    ~UploaderGadget();

    QWidget *widget() { return m_widget; }
    void loadConfiguration(IUAVGadgetConfiguration* config);

private:
    UploaderGadgetWidget *m_widget;
};

#endif // UPLOADERGADGET_H

