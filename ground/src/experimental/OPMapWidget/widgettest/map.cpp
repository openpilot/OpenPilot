#include "map.h"
map::map(QWidget* parent)
{
    mc= new mapcontrol::OPMapControl();
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(mc);

    QWidget* w = new QWidget();
    w->setLayout(layout);
    setCentralWidget(w);
}
