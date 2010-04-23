/*
 * imageManagerPoint.hpp
 *
 *  Created on: Apr 23, 2010
 *      Author: nmansard
 */

#ifndef IMAGEMANAGERPOINT_HPP_
#define IMAGEMANAGERPOINT_HPP_

#include "rtslam/parents.hpp"

class ObservationPinHoleAnchoredHomogeneousPoint;

class ImageManagerPoint
: public WeakParentOf<ObservationPinHoleAnchoredHomogeneousPoint>
{
	ENABLE_ACCESS_TO_CHILDREN(ObservationPinHoleAnchoredHomogeneousPoint,AHP,ahp);

};

#endif /* IMAGEMANAGERPOINT_HPP_ */
