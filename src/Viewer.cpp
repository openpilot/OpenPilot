#include "qdisplay/Viewer.hpp"

#include <QGraphicsScene>

namespace jafar {
namespace qdisplay {

Viewer::Viewer() : m_scene(new QGraphicsScene())
{
  show();
  setScene(m_scene);
}

}
}

