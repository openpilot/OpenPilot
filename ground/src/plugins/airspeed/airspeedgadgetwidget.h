/**
 ******************************************************************************
 *
 * @file       airspeedgadgetwidget.h
 * @author     David "Buzz" Carlson Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   airspeed 
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

#ifndef AIRSPEEDGADGETWIDGET_H_
#define AIRSPEEDGADGETWIDGET_H_

#include <QGraphicsView>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>

#include <QFile>
// Used for test purposes
#include <QTimer>

class AirspeedGadgetWidget : public QGraphicsView
{
    Q_OBJECT

public:
    AirspeedGadgetWidget(QWidget *parent = 0);
   ~AirspeedGadgetWidget();
   void setDialFile(QString dfn);
   void paint();
   void setActual(int speed);
   void setDesired(int speed);

protected:
   void paintEvent(QPaintEvent *event);
   void resizeEvent(QResizeEvent *event);

private:

// Test functions
private slots:
   void testRotate();
// End test functions

private:
   QSvgRenderer *m_renderer;
   QGraphicsSvgItem *m_background;
   QGraphicsSvgItem *m_foreground;
   QGraphicsSvgItem *m_desired;
   QGraphicsSvgItem *m_actual;

   // Test variables
   int testSpeed;
   QTimer m_testTimer;
   // End test variables
};
#endif /* AIRSPEEDGADGETWIDGET_H_ */
