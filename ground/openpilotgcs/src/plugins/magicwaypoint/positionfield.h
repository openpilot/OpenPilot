/**
 ******************************************************************************
 *
 * @file       joystickcontrol.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A that mimics a transmitter joystick and updates the MCC
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


#ifndef POSITIONFIELD_H
#define POSITIONFIELD_H

#include <QWidget>
#include <QGraphicsView>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>

namespace Ui {
    class PositionField;
}

class PositionField : public QGraphicsView
{
    Q_OBJECT

public:
    explicit PositionField(QWidget *parent = 0);
    ~PositionField();
    void paint();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

signals:
    void positionClicked(double north, double east);

public slots:
    void updateDesiredIndicator(double north, double east);
    void updateActualIndicator(double north, double east);

private:
    QSvgRenderer *m_renderer;
    QGraphicsSvgItem *m_background;
    QGraphicsSvgItem *m_positiondesired;
    QGraphicsSvgItem *m_positionactual;
};

#endif // POSITIONFIELD_H

/**
  * @}
  * @}
  */
