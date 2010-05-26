#ifndef OPMAPCONTROL_H
#define OPMAPCONTROL_H

#include "../internals/core.h"
#include <QtGui>

#include <QWidget>
class OPMapControl:public QWidget
{
    Q_OBJECT

public:
    OPMapControl(QWidget *parent=0);
protected:
    void paintEvent ( QPaintEvent* evnt );
    void mousePressEvent ( QMouseEvent* evnt );
    void mouseReleaseEvent ( QMouseEvent* evnt );
    void mouseMoveEvent ( QMouseEvent* evnt );
private:
    Core core;

};

#endif // OPMAPCONTROL_H
