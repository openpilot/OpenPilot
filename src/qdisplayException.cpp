/* $Id$ */

#include "qdisplay/qdisplayException.hpp"

#include <sstream>

using std::string;

using namespace jafar::qdisplay;

/*
 * class QdisplayException
 */

QdisplayException::QdisplayException(ExceptionId id_, const string& message_, const string& file_, int line_) throw() :
  jafar::kernel::Exception(message_, "qdisplay", exceptionIdToString(id_), file_, line_),
  id(id_)
{}

QdisplayException::~QdisplayException() throw() {}

QdisplayException::ExceptionId QdisplayException::getExceptionId() const throw() {
  return id;
}

string QdisplayException::exceptionIdToString(ExceptionId id_) throw() {
  switch(id_) {
//   case MY_ERROR:
//     return "MY_ERROR";

  default:
    std::stringstream s;
    s << id_;
    return s.str();
  }
}

