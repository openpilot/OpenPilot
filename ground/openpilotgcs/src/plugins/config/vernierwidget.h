/**
 ******************************************************************************
 *
 * @file       vernierwidget.h
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Configuration Plugin
 * @{
 * @brief A widget which displays a coarse/fine control
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

#ifndef VERNIERWIDGET_H_
#define VERNIERWIDGET_H_

#include <QWidget>
#include "ui_vernier.h"

class VernierWidget : public QWidget
{
    Q_OBJECT

public:
    VernierWidget(QWidget *parent = 0);
   ~VernierWidget();
   void setMin(double value);
   void setMax(double value);
   void setRange(double min, double max);

signals:

private slots:

private:
   double curveMin;
   double curveMax;
   Ui_VernierWidget * m_vernier;

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);


};
#endif /* VERNIERWIDGET_H_ */
