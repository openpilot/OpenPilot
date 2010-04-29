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
#include "rtslam/imageManagerPoint.hpp"
//#include "rtslam/parents.hpp"
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
		class ObservationPinHoleAnchoredHomogeneousPoint: public ObservationAbstract,
		    public SpecificChildOf<SensorPinHole> ,
		    public SpecificChildOf<LandmarkAnchoredHomogeneousPoint> ,
		    public ChildOf<ImageManagerPoint> {


				// Define the function linkToParentPinHole.
			ENABLE_LINK_TO_SPECIFIC_PARENT(SensorAbstract,SensorPinHole,PinHole,ObservationAbstract)
				;
				// Define the functions pinHole() and pinHolePtr().
			ENABLE_ACCESS_TO_SPECIFIC_PARENT(SensorPinHole,pinHole)
				;
				// Define the function linkToParentAHP.
			ENABLE_LINK_TO_SPECIFIC_PARENT(LandmarkAbstract,LandmarkAnchoredHomogeneousPoint,AHP,ObservationAbstract)
				;
				// Define the functions ahp() and ahpPtr().
			ENABLE_ACCESS_TO_SPECIFIC_PARENT(LandmarkAnchoredHomogeneousPoint,ahp)
				;

				// Define the function linkToWeakParentDataManager().
				ENABLE_LINK_TO_WEAK_SPECIFIC_PARENT(DataManagerAbstract,ImageManagerPoint,
						ObservationAbstract,ObservationPinHoleAnchoredHomogeneousPoint,DataManager)
				;

				// Define the functions imageManager() and imageManagerPtr().
			ENABLE_ACCESS_TO_PARENT(ImageManagerPoint,imageManager)
				;

			public:

				ObservationPinHoleAnchoredHomogeneousPoint(const sensor_ptr_t & pinholePtr, const landmark_ptr_t & ahpPtr);
				~ObservationPinHoleAnchoredHomogeneousPoint(void) {
					UNREGISTER_FROM_WEAK_SPECIFIC_PARENT(DataManagerAbstract,ObservationAbstract);
					UNREGISTER_FROM_WEAK_SPECIFIC_PARENT(ImageManagerPoint,ObservationPinHoleAnchoredHomogeneousPoint);
				}


				/**
				 * Projection function, with Jacobians and non-observable part.
				 */
				virtual void project_func(const vec7 & sg, const vec & lmk, vec & meas, vec & nobs, mat & EXP_sg, mat & EXP_lmk);
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

				/**
				 * Predict appearance
				 */
				virtual void predictAppearance();

				virtual double getMatchScore(){
					//todo implement getScoreMatchInPercent()
					return 85.0;
				}


				/**
				 * find and match the expected appearence in the raw-data
				 */
				virtual void matchFeature(raw_ptr_t rawPtr);

			public:
				/* Search in the sensor().dataManagerList() the manager of the proper type, to be
				 * linked as the weak-father of this observation.
				 */
				void linkToWeakParentDataManager(void);


		};

	}
}

#endif /* OBSERVATIONPINHOLEANCHOREDHOMOGENEOUS_HPP_ */
