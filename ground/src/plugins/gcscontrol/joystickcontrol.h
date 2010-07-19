#ifndef JOYSTICKCONTROL_H
#define JOYSTICKCONTROL_H

#include <QWidget>
#include <QGraphicsView>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>
#include "uavobjects/manualcontrolcommand.h"

namespace Ui {
    class JoystickControl;
}

class JoystickControl : public QGraphicsView
{
    Q_OBJECT

public:
    explicit JoystickControl(QWidget *parent = 0);
    ~JoystickControl();
   void paint();
protected:
     void mousePressEvent(QMouseEvent *event);
     void paintEvent(QPaintEvent *event);
     void resizeEvent(QResizeEvent *event);

private:
     ManualControlCommand* getMCC();
     QSvgRenderer *m_renderer;
     QGraphicsSvgItem *m_background;
};

#endif // JOYSTICKCONTROL_H
