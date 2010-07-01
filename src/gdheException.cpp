/* $Id$ */

#include "gdhe/gdheException.hpp"

#include <sstream>

using std::string;

using namespace jafar::gdhe;

/*
 * class GdheException
 */

GdheException::GdheException(ExceptionId id_, const string& message_, const string& file_, int line_) throw() :
  jafar::kernel::Exception(message_, "gdhe", exceptionIdToString(id_), file_, line_),
  id(id_)
{}

GdheException::~GdheException() throw() {}

GdheException::ExceptionId GdheException::getExceptionId() const throw() {
  return id;
}

string GdheException::exceptionIdToString(ExceptionId id_) throw() {
  switch(id_) {
//   case MY_ERROR:
//     return "MY_ERROR";

  default:
    std::stringstream s;
    s << id_;
    return s.str();
  }
}

