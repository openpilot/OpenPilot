/* $Id$ */

/** Tools for module rtslam.
 *
 * \file rtslamTools.i
 * \ingroup rtslam
 */   

%{

#include <sstream>

%}

%inline %{

namespace jafar {
  namespace rtslam {


    /** Template print function which calls the output operator<< of A
     * and returns the resulting string.
     */
    template<class A>
    std::string print(const A& a_) {
      std::ostringstream os;
      os << a_ << std::endl;
      return os.str();
    };

  } // namespace rtslam
} // namespace jafar

%}


