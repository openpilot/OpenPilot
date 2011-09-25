/**
 ******************************************************************************
 *
 * @file       debuggadgetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup DebugGadgetPlugin Debug Gadget Plugin
 * @{
 * @brief A place holder gadget plugin 
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

#ifndef DEBUGGADGETWIDGET_H_
#define DEBUGGADGETWIDGET_H_

#include <QtGui/QLabel>
#include "ui_debug.h"
class DebugGadgetWidget : public QLabel
{
    Q_OBJECT

public:
    DebugGadgetWidget(QWidget *parent = 0);
    ~DebugGadgetWidget();

private:
    Ui_Form *m_config;
private slots:
        void saveLog();
        void dbgMsgError( const QString & level, const QList<QVariant> & msgs );
        void dbgMsg( const QString & level, const QList<QVariant> & msgs );
};

#endif /* DEBUGGADGETWIDGET_H_ */
