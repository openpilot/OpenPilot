/**
 ******************************************************************************
 *
 * @file       GCSControlgadgetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
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

#ifndef GCSControlGADGETWIDGET_H_
#define GCSControlGADGETWIDGET_H_

#include <QtGui/QLabel>
#include "manualcontrolcommand.h"

#define UDP_PORT 2323

class Ui_GCSControl;

class GCSControlGadgetWidget : public QLabel
{
    Q_OBJECT

public:
    GCSControlGadgetWidget(QWidget *parent = 0);
    ~GCSControlGadgetWidget();
    void setGCSControl(bool newState);
    bool getGCSControl(void);
    void setUDPControl(bool newState);
    bool getUDPControl(void);

signals:
    void sticksChanged(double leftX, double leftY, double rightX, double rightY);

public slots:
    // signals from parent gadget indicating change from flight
    void updateSticks(double leftX, double leftY, double rightX, double rightY);

    // signals from children widgets indicating a local change
    void leftStickClicked(double X, double Y);
    void rightStickClicked(double X, double Y);

protected slots:
    void toggleControl(int state);
    void toggleArmed(int state);
    void selectFlightMode(int state);
    void mccChanged(UAVObject *);
    void toggleUDPControl(int state);

private:
    Ui_GCSControl *m_gcscontrol;
    UAVObject::Metadata mccInitialData;
    double leftX,leftY,rightX,rightY;
};

#endif /* GCSControlGADGETWIDGET_H_ */
