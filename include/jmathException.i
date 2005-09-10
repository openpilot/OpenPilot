/* $Id$ */

/** swig/tcl interface file for module jmath exceptions.
 *
 * \file jmathException.i
 */      

%{
  #include <iostream>
  #include <exception>

  #include "kernel/jafarException.hpp"

  // uncomment if you define a catch clause for your own exceptions 
  // #include "jmath/jmathException.hpp"

%}

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

