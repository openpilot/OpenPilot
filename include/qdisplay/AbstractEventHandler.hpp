#ifndef _ABSTRACT_EVENT_HANDLER_HPP_
#define _ABSTRACT_EVENT_HANDLER_HPP_


namespace jafar {
namespace qdisplay {

  /**
   * This the base class for Qt events handler that you want to catch in your scripts.
   * You should never instantiate it directly.
   * It needs to be inherited and reimplemented in your favorite scripting language.
   */
  class AbstractEventHandler {
    public:
      virtual ~AbstractEventHandler() { }
      virtual void mouseReleaseEvent(int button, double x, double y) { };
  };
}
}


#endif
