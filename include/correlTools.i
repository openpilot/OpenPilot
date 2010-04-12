/* $Id$ */

/** Tools for module correl.
 *
 * \file correlTools.i
 * \ingroup correl
 */   

%{

#include <sstream>

%}

%inline %{

namespace jafar {
  namespace correl {


    /** Template print function which calls the output operator<< of A
     * and returns the resulting string.
     */
    template<class A>
    std::string print(const A& a_) {
      std::ostringstream os;
      os << a_ << std::endl;
      return os.str();
    };

  } // namespace correl
} // namespace jafar

%}


