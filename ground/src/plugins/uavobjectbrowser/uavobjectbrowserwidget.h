/**
 ******************************************************************************
 *
 * @file       uavobjectbrowserwidget.h
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

#ifndef UAVOBJECTBROWSERWIDGET_H_
#define UAVOBJECTBROWSERWIDGET_H_

#include <QtGui/QWidget>
#include <QtGui/QTreeView>
#include "uavobjects/settingspersistence.h"

class QPushButton;
class Ui_UAVObjectBrowser;


class UAVObjectBrowserWidget : public QWidget
{
    Q_OBJECT

public:
    UAVObjectBrowserWidget(QWidget *parent = 0);
   ~UAVObjectBrowserWidget();

private slots:
   void sendUpdate();
   void requestUpdate();
   void showMetaData(bool show);
   void saveSettings();
   void readSettings();


private:
   QPushButton *m_requestUpdate;
   QPushButton *m_sendUpdate;
   Ui_UAVObjectBrowser *m_browser;
   QAbstractItemModel *m_model;

   void updateSettings(SettingsPersistence::OperationEnum op);
};

#endif /* UAVOBJECTBROWSERWIDGET_H_ */
