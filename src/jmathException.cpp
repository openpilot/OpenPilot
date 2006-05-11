/* $Id$ */

#include "jmath/jmathException.hpp"

#include <sstream>

using std::string;

using namespace jafar::jmath;

/*
 * class JmathException
 */

JmathException::JmathException(ExceptionId id_, const string& message_, const string& file_, int line_) throw() :
  jafar::kernel::Exception(message_, "jmath", exceptionIdToString(id_), file_, line_),
  id(id_)
{}

JmathException::~JmathException() throw() {}

JmathException::ExceptionId JmathException::getExceptionId() const throw() {
  return id;
}

string JmathException::exceptionIdToString(ExceptionId id_) throw() {
  switch(id_) {
  case LAPACK_ERROR:
    return "LAPACK_ERROR";
  default:
    std::stringstream s;
    s << id_;
    return s.str();
  }
}

