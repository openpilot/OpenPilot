/**
 ******************************************************************************
 *
 * @file       lineardialgadgetwidget.h
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   lineardialplugin
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

#ifndef LINEARDIALGADGETWIDGET_H_
#define LINEARDIALGADGETWIDGET_H_

#include "lineardialgadgetconfiguration.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"
#include <QGraphicsView>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>

#include <QFile>
#include <QTimer>

class LineardialGadgetWidget : public QGraphicsView
{
    Q_OBJECT

public:
    LineardialGadgetWidget(QWidget *parent = 0);
   ~LineardialGadgetWidget();
   void setDialFile(QString dfn);
   void paint();
   void setRange(double min, double max) { minValue=min; maxValue=max;}
   void setGreenRange(double min, double max) {greenMin=min; greenMax=max;}
   void setYellowRange(double min, double max) {yellowMin=min; yellowMax=max;}
   void setRedRange(double min, double max) {redMin=min; redMax=max;}
   void connectInput(QString obj, QString field);
   void setIndex(double val);
   void setDialFont(QString fontProps);

public slots:
    void updateIndex(UAVObject *object1);


protected:
   void paintEvent(QPaintEvent *event);
   void resizeEvent(QResizeEvent *event);

private:

private slots:
   // Test function
   void testRotate();
   void moveIndex();

private:
   QSvgRenderer *m_renderer;
   QGraphicsSvgItem *background;
   QGraphicsSvgItem *foreground;
   QGraphicsSvgItem *index;
   QGraphicsSvgItem *green;
   QGraphicsSvgItem *yellow;
   QGraphicsSvgItem *red;

   QGraphicsTextItem *fieldName;
   QGraphicsTextItem *fieldValue;

                   // Simple flag to skip rendering if the
   bool fgenabled; // layer does not exist.
   bool verticalDial; // True if the dials scales vertically.

   qreal startX; // Where we should draw the bargraph
   qreal startY; // green/yellow/red zones.
   qreal bargraphSize;
   qreal indexHeight;
   qreal indexWidth;

   double testVal;

   double minValue;
   double maxValue;
   double greenMin;
   double greenMax;
   double yellowMin;
   double yellowMax;
   double redMin;
   double redMax;

   // The Value and target variables
   // are expressed in degrees
   double indexTarget;
   double indexValue;

   // Rotation timer
   QTimer dialTimer;

   // Name of the fields to read when an update is received:
   UAVDataObject* obj1;
   QString field1;


   // Test variables
   int testSpeed;
   QTimer m_testTimer;
   // End test variables
};
#endif /* LINEARDIALGADGETWIDGET_H_ */
