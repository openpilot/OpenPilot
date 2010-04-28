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

namespace jafar {
	namespace rtslam {

		class ObservationPinHolePoint;
		typedef boost::shared_ptr<ObservationPinHolePoint> obs_ph_p_ptr_t;

		class ObservationPinHolePoint: public ObservationAbstract {

			public:
				static bool detectInRoi(const raw_ptr_t & rawPtr, const ROI & roi, feature_ptr_t & featPtr){
					// todo implement detectInRoi()
				}

		};

	}
}

#endif /* OBSERVATIONPINHOLEPOINT_HPP_ */
