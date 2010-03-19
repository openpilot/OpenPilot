/*
 * mapgadgetwidget.h
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */

#ifndef MAPGADGETWIDGET_H_
#define MAPGADGETWIDGET_H_

#include "qmapcontrol/qmapcontrol.h"
#include <QtGui/QWidget>

using namespace qmapcontrol;

class MapGadgetWidget : public QWidget
{
    Q_OBJECT

public:
    MapGadgetWidget(QWidget *parent = 0);
   ~MapGadgetWidget();

protected:
   void resizeEvent(QResizeEvent *event);

private:
   MapControl *mc;
   MapAdapter *mapadapter;
   Layer *mainlayer;

   void addZoomButtons();
};

#endif /* MAPGADGETWIDGET_H_ */
