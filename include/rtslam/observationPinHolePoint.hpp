/**
 * \file observationPinHolePoint.hpp
 *
 * ## Add brief description here ##
 *
 * \author jsola@laas.fr
 * \date 28/04/2010
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#ifndef OBSERVATIONPINHOLEPOINT_HPP_
#define OBSERVATIONPINHOLEPOINT_HPP_

#include <boost/shared_ptr.hpp>

#include "rtslam/observationAbstract.hpp"
#include "rtslam/rtSlam.hpp"
#include "rtslam/activeSearch.hpp"
#include "rtslam/featureAbstract.hpp"
#include "rtslam/appearanceImage.hpp"

namespace jafar {
	namespace rtslam {

		class ObservationPinHolePoint;
		typedef boost::shared_ptr<ObservationPinHolePoint> obs_ph_p_ptr_t;

		class ObservationPinHolePoint: public ObservationAbstract {

			public:
				static bool detectInRoi(const raw_ptr_t & rawPtr, const ROI & roi, feature_ptr_t & featPtr){

					// todo implement detectInRoi() - the code below is just to ensure compilation

					// 1. generate random pixel
					vec2 pix;
					pix(0) = roi.x + rand()%roi.width; pix(1) = roi.y + rand()%roi.height;

					// Assign pixel and covariance
					featPtr->state.x(pix);
					identity_mat I(2);
					featPtr->state.P(I);

					// initialize appearance
					appearenceimage_ptr_t appPtr(new AppearenceImage);
					featPtr->appearancePtr = appPtr;

					// return true if successful
					return true;
				}

		};

	}
}

#endif /* OBSERVATIONPINHOLEPOINT_HPP_ */
