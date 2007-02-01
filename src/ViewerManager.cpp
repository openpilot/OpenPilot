
#include "qdisplay/ViewerManager.hpp"

#include <kernel/jafarMacro.hpp>

#include "qdisplay/Viewer.hpp"

namespace jafar {
namespace qdisplay {

ViewerManager* ViewerManager::s_instance = 0;

ViewerManager::ViewerManager()
{
  if(ViewerManager::s_instance != 0)
  {
    JFR_RUN_TIME("There is allready an instance of the Viewer Manager.");
  }
  ViewerManager::s_instance = this;
}

ViewerManager::~ViewerManager()
{
  ViewerManager::s_instance = 0;
}

void ViewerManager::closeAllViewer()
{
  if(instance())
  {
    for(QList<jafar::qdisplay::Viewer*>::iterator it = instance()->m_list.begin();
        it != instance()->m_list.end(); it++)
    {
      (*it)->close();
    }
  }
}

}
}
