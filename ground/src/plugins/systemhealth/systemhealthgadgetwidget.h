/**
 ******************************************************************************
 *
 * @file       systemhealthgadgetwidget.h
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   systemhealth
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

#ifndef SYSTEMHEALTHGADGETWIDGET_H_
#define SYSTEMHEALTHGADGETWIDGET_H_

#include "systemhealthgadgetconfiguration.h"
#include "uavobjects/uavobject.h"
#include <QGraphicsView>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>

#include <QFile>
#include <QTimer>

class SystemHealthGadgetWidget : public QGraphicsView
{
    Q_OBJECT

public:
    SystemHealthGadgetWidget(QWidget *parent = 0);
   ~SystemHealthGadgetWidget();
   void setSystemFile(QString dfn);
   void setIndicator(QString indicator);
   void paint();

protected:
   void paintEvent(QPaintEvent *event);
   void resizeEvent(QResizeEvent *event);

private slots:
   // Test function
   void test();
   void updateAlarms(UAVObject *systemAlarm); // Called by the systemalarms UAVObject

private:
   QSvgRenderer *m_renderer;
   QGraphicsSvgItem *background;
   QGraphicsSvgItem *foreground;

                   // Simple flag to skip rendering if the
   bool fgenabled; // layer does not exist.


   // Test variables
   double testValue;
   QTimer m_testTimer;
   // End test variables
};
#endif /* SYSTEMHEALTHGADGETWIDGET_H_ */
