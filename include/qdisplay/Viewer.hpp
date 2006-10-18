#ifndef _IMAGE_VIEWER_H_
#define _IMAGE_VIEWER_H_

#include <QGraphicsView>

namespace jafar {
namespace qdisplay {

class Viewer : public QGraphicsView {

  public:
    Viewer();

    QGraphicsScene* scene() { return m_scene; }
  private:
    QGraphicsScene* m_scene;
};

}
}

#endif
