/* $Id$ */

#include "correl/correlException.hpp"

#include <sstream>

using std::string;

using namespace jafar::correl;

/*
 * class CorrelException
 */

CorrelException::CorrelException(ExceptionId id_, const string& message_, const string& file_, int line_) throw() :
  jafar::kernel::Exception(message_, "correl", exceptionIdToString(id_), file_, line_),
  id(id_)
{}

CorrelException::~CorrelException() throw() {}

CorrelException::ExceptionId CorrelException::getExceptionId() const throw() {
  return id;
}

string CorrelException::exceptionIdToString(ExceptionId id_) throw() {
  switch(id_) {
//   case MY_ERROR:
//     return "MY_ERROR";

  default:
    std::stringstream s;
    s << id_;
    return s.str();
  }
}

