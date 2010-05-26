#include <QApplication>
#include "map.h"

int main(int argc, char *argv[])
{
      QApplication app(argc, argv);
      map * mw = new map();

      mw->resize(400,590);
      mw->setWindowTitle("Map");
      mw->show();
      return app.exec();
}


