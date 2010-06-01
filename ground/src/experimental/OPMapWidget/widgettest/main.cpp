

#include <QApplication>
//#include "map.h"
#include "../mapwidget/opmapcontrol.h"
int main(int argc, char *argv[])
{
      QApplication app(argc, argv);
  //    map * mw = new map();

    //  mw->resize(400,590);
//      mw->setWindowTitle("Map");
//      mw->adjustSize();
//      mw->show();1022 680
      OPMapControl map;
      map.setGeometry(20,20,1022,680);
      map.show();
      return app.exec();
}


