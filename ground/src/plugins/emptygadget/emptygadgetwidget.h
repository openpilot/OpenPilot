/*
 * emptygadgetwidget.h
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */

#ifndef EMPTYGADGETWIDGET_H_
#define EMPTYGADGETWIDGET_H_

#include <QtGui/QWidget>
#include <QtGui/QLabel>
class QWidget;
class QLabel;

class EmptyGadgetWidget : public QLabel
{
    Q_OBJECT

public:
    EmptyGadgetWidget(QWidget *parent = 0);
   ~EmptyGadgetWidget();

private:
};

#endif /* EMPTYGADGETWIDGET_H_ */
