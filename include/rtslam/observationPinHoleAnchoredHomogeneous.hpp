/**
 * \file observationPinHoleAnchoredHomogeneous.hpp
 *
 * Header file for observations of Anchored Homogeneous Points (AHP) from pin-hole cameras.
 *
 * \date 14/02/2010
 *     \author jsola@laas.fr
 *
 * \ingroup rtslam
 */

#ifndef OBSERVATIONPINHOLEANCHOREDHOMOGENEOUS_HPP_
#define OBSERVATIONPINHOLEANCHOREDHOMOGENEOUS_HPP_

#include "rtslam/observationAbstract.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"

#include "boost/shared_ptr.hpp"

namespace jafar {
	namespace rtslam {

		class ObservationPinHoleAnchoredHomogeneousPoint;
		typedef boost::shared_ptr<ObservationPinHoleAnchoredHomogeneousPoint> obs_ph_ahp_ptr_t;


		/**
		 * Class for Pin-Hole observations of Anchored Homogeneous 3D points.
		 * \author jsola@laas.fr
		 * \ingroup rtslam
		 */
		class ObservationPinHoleAnchoredHomogeneousPoint: public ObservationAbstract {
			public:

				ObservationPinHoleAnchoredHomogeneousPoint(const sensor_ptr_t & pinholePtr, const landmark_ptr_t & ahpPtr);
				void link(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr); ///< Link to sensor and landmark

				pinhole_ptr_t pinHolePtr; ///<  Use this pointer to downcast the SensorAbstract into SensorPinHole type.
				ahp_ptr_t ahpPtr; ///<          Use this pointer to downcast the LandmarkAbstract into LandmarkAnchoredHomogeneousPoint type.

				/**
				 * Projection function, with Jacobians and non-observable part.
				 */
				void project_func(const vec7 & sg, const vec & lmk, vec & meas, vec & nobs, mat & EXP_sg, mat & EXP_lmk);
				/**
				 * Retro-projection function, with Jacobians
				 */
				void backProject_func(const vec7 & sg, const vec & meas, const vec & nobs, vec & lmk, mat & LMK_sg,
				    mat & LMK_meas, mat LMK_nobs);


				/**
				 * Predict visibility.
				 *
				 * Visibility can only be established after project_func() has been called.
				 *
				 * \return true if landmark is predicted visible.
				 */
				virtual bool predictVisibility();



		};

	}
}

#endif /* OBSERVATIONPINHOLEANCHOREDHOMOGENEOUS_HPP_ */
