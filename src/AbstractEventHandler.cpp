#include "qdisplay/AbstractEventHandler.hpp"

#include "kernel/jafarMacro.hpp"

using namespace jafar::qdisplay;

AbstractEventHandler::~AbstractEventHandler()
{
}

void AbstractEventHandler::mouseReleaseEvent(int button, double x, double y)
{
  JFR_DEBUG("button = " << button << " x = " << x << " y = " << y);
}

