/* $Id$ */

/** swig/tcl interface file for module correl exceptions.
 *
 * \file correlException.i
 * \ingroup correl
 */      

%{
  #include <iostream>
  #include <exception>

  #include "kernel/jafarException.hpp"
  #include "correl/correlException.hpp"
%}



#if defined(SWIGTCL)
/* Tcl exceptions handler.
 *
 * You can customize this handler and add catch blocks to handle your
 * own exceptions. Be aware that order in catch clauses is meaningfull
 * because of JafarException heritage tree.
 */
%exception {
  try {
    $action // Gets substituted by actual function call
  }
  catch (jafar::kernel::Exception& e) {
    std::cout << e;
    SWIG_fail;
  }
  catch (std::exception& e) {
    std::cout << "** std::exception: **" << std::endl;
    std::cout << e.what() << std::endl;
    SWIG_fail;
  }
  catch(...) {
    std::cout << "** unknown exception **" << std::endl;
    SWIG_fail;
  }
}
#elif defined(SWIGRUBY)

%import "kernel/jafarException.hpp"
%include "correl/correlException.hpp"

JAFAR_EXCEPTION_SUPPORT
JAFAR_RUBY_EXCEPTION(Correl, CorrelException);
%exception {
    try {
        $action
    }
    JAFAR_CATCH_MODULE_EXCEPTION(correl, CorrelException)
    JAFAR_CATCH_GENERIC
}

#endif


