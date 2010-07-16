/**
 ******************************************************************************
 *
 * @file       consolegadget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConsolePlugin Console Plugin
 * @{
 * @brief The Console Gadget impliments a console view 
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

#ifndef CONSOLEGADGET_H_
#define CONSOLEGADGET_H_

#include <coreplugin/iuavgadget.h>

namespace Core {
class IUAVGadget;
}
class ConsoleGadgetWidget;
class TextEditLoggerEngine;

using namespace Core;

class ConsoleGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    ConsoleGadget(QString classId, ConsoleGadgetWidget *widget, QWidget *parent = 0);
    ~ConsoleGadget();

    QList<int> context() const { return m_context; }
    QWidget *widget() { return m_widget; }
    QString contextHelpId() const { return QString(); }

private:
    QWidget *m_widget;
    QList<int> m_context;
    TextEditLoggerEngine *m_logger;
};


#endif // CONSOLEGADGET_H_
