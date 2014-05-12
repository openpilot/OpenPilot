/**
 ******************************************************************************
 *
 * @file       outputchannelform.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2011.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Servo output configuration form for the config output gadget
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
#ifndef OUTPUTCHANNELFORM_H
#define OUTPUTCHANNELFORM_H

#include "channelform.h"
#include "configoutputwidget.h"
#include "ui_outputchannelform.h"

#include <QWidget>

class OutputChannelForm : public ChannelForm {
    Q_OBJECT

public:
    explicit OutputChannelForm(const int index, QWidget *parent = NULL);
    ~OutputChannelForm();

    friend class ConfigOutputWidget;

    virtual QString name();
    virtual void setName(const QString &name);

public slots:
    int min() const;
    void setMin(int minimum);
    int max() const;
    void setMax(int maximum);
    int neutral() const;
    void setNeutral(int value);
    void setRange(int minimum, int maximum);
    void enableChannelTest(bool state);

signals:
    void channelChanged(int index, int value);

private:
    Ui::outputChannelForm ui;
    bool m_inChannelTest;

private slots:
    void linkToggled(bool state);
    void reverseChannel(bool state);
    void sendChannelTest(int value);
    void setChannelRange();
};

#endif // OUTPUTCHANNELFORM_H
