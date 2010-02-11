/* $Id$ */

#include "rtslam/rtslamException.hpp"

#include <sstream>

using std::string;

using namespace jafar::rtslam;

/*
 * class RtslamException
 */

RtslamException::RtslamException(ExceptionId id_, const string& message_, const string& file_, int line_) throw() :
  jafar::kernel::Exception(message_, "rtslam", exceptionIdToString(id_), file_, line_),
  id(id_)
{}

RtslamException::~RtslamException() throw() {}

RtslamException::ExceptionId RtslamException::getExceptionId() const throw() {
  return id;
}

string RtslamException::exceptionIdToString(ExceptionId id_) throw() {
  switch(id_) {
//   case MY_ERROR:
//     return "MY_ERROR";

  default:
    std::stringstream s;
    s << id_;
    return s.str();
  }
}

