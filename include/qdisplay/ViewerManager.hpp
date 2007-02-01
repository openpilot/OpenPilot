#ifndef _VIEWER_MANAGER_HPP_
#define _VIEWER_MANAGER_HPP_

#include <QList>

namespace jafar {
namespace qdisplay {

class Viewer;

/**
 * ViewerManager gives access to all active Viewer.
 * Usefull for memory management and for closing all visible Viewer.
 */
class ViewerManager {
  public:
    /**
     * Instantiate a new viewer manager.
     * @exception jafar::kernel::JafarException is thrown if there is allready an instance
     * of the ViewerManager.
     */
    ViewerManager();
    ~ViewerManager();
    /**
     * @param v viewer to register, it's called in the Viewer constructor
     */
    inline static void registerViewer( jafar::qdisplay::Viewer* v)
    {
      if(instance()) instance()->m_list.append( v);
    }
    /**
     * @param v viewer to unregister, it's called in the Viewer destructor
     */
    inline static void unregisterViewer( jafar::qdisplay::Viewer* v)
    {
      if(instance()) instance()->m_list.removeAll( v);
    }
    /**
     * @return a pointer to the current instance of the ViewerManager.
     * @note this function won't create the viewer manager
     */
    inline static ViewerManager* instance() { return s_instance; }
    /**
     * @return the list of registered viewer
     */
    inline QList<Viewer*> viewers() { return m_list; }
    /**
     * Close all registered viewers.
     */
    static void closeAllViewer();
  private:
    static ViewerManager* s_instance;
    QList<Viewer*> m_list;
};
}
}

#endif
