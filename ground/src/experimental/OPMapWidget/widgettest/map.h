#ifndef MAP_H
#define MAP_H
#include "../mapwidget/opmapcontrol.h"
#include <QtGui>
class map:public QMainWindow
{Q_OBJECT
public:
    map(QWidget* parent = 0);
private:
    mapcontrol::OPMapControl* mc;
};

#endif // MAP_H
