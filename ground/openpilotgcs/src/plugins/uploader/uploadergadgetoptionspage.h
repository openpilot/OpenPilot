/**
 ******************************************************************************
 *
 * @file       uploadergadgetoptionspage.h
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

#ifndef UPLOADERGADGETOPTIONSPAGE_H
#define UPLOADERGADGETOPTIONSPAGE_H
#include <qextserialport/src/qextserialenumerator.h>
#include "coreplugin/dialogs/ioptionspage.h"
#include "QString"
#include <QStringList>
#include <QDebug>
#include "uploader_global.h"

namespace Core {
class IUAVGadgetConfiguration;
}
class UploaderGadgetConfiguration;
class QTextEdit;
class QComboBox;
class QSpinBox;

using namespace Core;

class UPLOADER_EXPORT UploaderGadgetOptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit UploaderGadgetOptionsPage(UploaderGadgetConfiguration *config, QObject *parent = 0);

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

private:
    UploaderGadgetConfiguration *m_config;
    QComboBox *m_portCB;
    QComboBox *m_speedCB;
    QComboBox *m_databitsCB;
    QComboBox *m_flowCB;
    QComboBox *m_parityCB;
    QComboBox *m_stopbitsCB;
    QSpinBox *m_timeoutSpin;
    QStringList BaudRateTypeString;
    QStringList BaudRateTypeStringALL;
    QStringList DataBitsTypeStringALL;
    QStringList ParityTypeStringALL;
    QStringList StopBitsTypeStringALL;
    QStringList DataBitsTypeString;
    QStringList ParityTypeString;
    QStringList StopBitsTypeString;
    QStringList FlowTypeString;
};

#endif // UPLOADERGADGETOPTIONSPAGE_H
