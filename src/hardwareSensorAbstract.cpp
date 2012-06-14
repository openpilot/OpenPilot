/**
 * \file hardwareSensorAbstract.cpp
 * \date 15/03/2011
 * \author croussil
 * \ingroup rtslam
 */


#include "rtslam/hardwareSensorAbstract.hpp"

namespace jafar {
namespace rtslam {
namespace hardware {

  //enum Quantity { qPos, qOriQuat, qOriEuler, qVel, qAbsVel, qAngVel, qAbsAngVel, qAcc, qAbsAcc, qBundleobs, qNQuantity };
  const int HardwareSensorProprioAbstract::QuantityDataSizes[qNQuantity] = { 3, 4, 3, 3, 3, 3, 3, 3, 3, 6 };
  const int HardwareSensorProprioAbstract::QuantityObsSizes [qNQuantity] = { 3, 4, 3, 3, 3, 3, 3, 3, 3, 3 };


}}}
