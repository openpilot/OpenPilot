/*
 * scopegadgetwidget.h
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */

#ifndef SCOPEGADGETWIDGET_H_
#define SCOPEGADGETWIDGET_H_

#include "qwt/src/qwt.h"
#include "qwt/src/qwt_plot.h"
//#include <QtGui/QWidget>
//class QWidget;
class QwtPlot;

class ScopeGadgetWidget : public QwtPlot
{
    Q_OBJECT

public:
    ScopeGadgetWidget(QWidget *parent = 0);
   ~ScopeGadgetWidget();

private:
};

#endif /* SCOPEGADGETWIDGET_H_ */
