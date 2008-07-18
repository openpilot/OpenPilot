/* $Id$ */

/** Tools for module qdisplay.
 *
 * \file qdisplayTools.i
 * \ingroup qdisplay
 */   

%{

#include <sstream>
#include <QApplication>

%}

%inline %{

namespace jafar {
  namespace qdisplay {


    /** Template print function which calls the output operator<< of A
     * and returns the resulting string.
     */
/*    template<class A>
    std::string print(const A& a_) {
      std::ostringstream os;
      os << a_ << std::endl;
      return os.str();
    };*/
    void processEvents() {
      QApplication::processEvents();
    };

  } // namespace qdisplay
} // namespace jafar

%}


