

#include <QApplication>
//#include "map.h"
//#include "../mapwidget/opmapcontrol.h"
//#include "../mapwidget/mapgraphicitem.h"
#include "../mapwidget/opmapwidget.h"

#include <QGLWidget>
int main(int argc, char *argv[])
{
      QApplication app(argc, argv);

//      mapcontrol::OPMapControl map;
//      map.setGeometry(20,20,1022,680);
//      map.show();

//      QGraphicsScene scene;
//      internals::Core *c=new internals::Core;
//      mapcontrol::MapGraphicItem *mapi=new mapcontrol::MapGraphicItem(c);
//      scene.addItem(mapi);
//      QGraphicsView view(&scene);
//      view.setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
//      view.setRenderHints(QPainter::Antialiasing
//                               | QPainter::TextAntialiasing);
//
//      mapi->rotate(10);
//      view.show();
      mapcontrol::OPMapWidget *map=new mapcontrol::OPMapWidget();
      map->show();
      return app.exec();
}


