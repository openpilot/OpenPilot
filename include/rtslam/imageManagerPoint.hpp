/*
 * imageManagerPoint.hpp
 *
 *  Created on: Apr 23, 2010
 *      Author: nmansard
 */

#ifndef IMAGEMANAGERPOINT_HPP_
#define IMAGEMANAGERPOINT_HPP_

#include "rtslam/parents.hpp"
#include "rtslam/dataManagerAbstract.hpp"

namespace jafar {
	namespace rtslam {

		class ObservationPinHoleAnchoredHomogeneousPoint;

		class ImageManagerPoint: public DataManagerAbstract,
		    public WeakParentOf<ObservationPinHoleAnchoredHomogeneousPoint> {
			ENABLE_ACCESS_TO_WEAK_CHILDREN(ObservationPinHoleAnchoredHomogeneousPoint,AHP,ahp)
				;

			virtual ~ ImageManagerPoint( void ){}

		};

	}
}// namespace jafar::rtslam

#endif /* IMAGEMANAGERPOINT_HPP_ */
