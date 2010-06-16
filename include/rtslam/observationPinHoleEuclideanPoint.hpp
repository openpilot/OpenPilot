/*
 * observationPinHoleEuclideanPoint.hpp
 *
 *  Created on: Apr 15, 2010
 *      Author: agonzale
 */

#ifndef OBSERVATIONPINHOLEEUCLIDEANPOINT_HPP_
#define OBSERVATIONPINHOLEEUCLIDEANPOINT_HPP_

#include "rtslam/observationAbstract.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/imageManagerPoint.hpp"
#include "boost/shared_ptr.hpp"

namespace jafar {
	namespace rtslam {

		class ObservationPinHoleEuclideanPoint;
		typedef boost::shared_ptr<ObservationPinHoleEuclideanPoint> obs_ph_euc_ptr_t;


		/**
		 * Class for Pin-Hole observations of Euclidean 3D points.
		 * \author jsola@laas.fr
		 * \ingroup rtslam
		 */
		class ObservationPinHoleEuclideanPoint: public ObservationAbstract,
		    public SpecificChildOf<SensorPinHole> ,
		    public SpecificChildOf<LandmarkEuclideanPoint> ,
		    public ChildOf<ImageManagerPoint>{


			// Define the function linkToParentPinHole.
			ENABLE_LINK_TO_SPECIFIC_PARENT(SensorAbstract,SensorPinHole,PinHole,ObservationAbstract);

			// Define the functions pinHole() and pinHolePtr().
			ENABLE_ACCESS_TO_SPECIFIC_PARENT(SensorPinHole,pinHole);

			// Define the function linkToParentEUC.
			ENABLE_LINK_TO_SPECIFIC_PARENT(LandmarkAbstract,LandmarkEuclideanPoint,EUC,ObservationAbstract);

			// Define the functions euc() and eucPtr().
			ENABLE_ACCESS_TO_SPECIFIC_PARENT(LandmarkEuclideanPoint,euc);

			// Define the function linkToWeakParentDataManager().
			ENABLE_LINK_TO_WEAK_SPECIFIC_PARENT(
					DataManagerAbstract,
					ImageManagerPoint,
					ObservationAbstract,
					ObservationPinHoleEuclideanPoint,
					DataManager)
			;

			// Define the functions imageManager() and imageManagerPtr().
		ENABLE_ACCESS_TO_PARENT(ImageManagerPoint,imageManager)
			;

			public:

			ObservationPinHoleEuclideanPoint(const sensor_ptr_t & pinholePtr, const landmark_ptr_t & eucPtr, int patchSize);
			~ObservationPinHoleEuclideanPoint(void){
					UNREGISTER_FROM_WEAK_SPECIFIC_PARENT(DataManagerAbstract,ObservationAbstract);
					UNREGISTER_FROM_WEAK_SPECIFIC_PARENT(ImageManagerPoint,ObservationPinHoleAnchoredHomogeneousPoint);
				}

			void setup(const sensor_ptr_t & pinholePtr, const landmark_ptr_t & eucPtr, const vec & _noiseStd, int patchSize);

				virtual std::string typeName() {
					return "Obs. Pinhole Euclidean point";
				}


//				void setup(double _pixNoise = 1.0);


				/**
				 * Projection function, with Jacobians and non-observable part.
				 */
				virtual void project_func(const vec7 & sg, const vec & lmk, vec & meas, vec & nobs);
				/**
				 * Projection function, with Jacobians and non-observable part.
				 */
				virtual void project_func(const vec7 & sg, const vec & lmk, vec & meas, vec & nobs, mat & EXP_sg, mat & EXP_lmk);
				/**
				 * Retro-projection function, with Jacobians
				 */
				virtual void backProject_func(const vec7 & sg, const vec & meas, const vec & nobs, vec & lmk);
				/**
				 * Retro-projection function, with Jacobians
				 */
				virtual void backProject_func(const vec7 & sg, const vec & meas, const vec & nobs, vec & lmk, mat & EUC_sg,
				    mat & LMK_meas, mat & LMK_nobs);


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
					return measurement.matchScore;
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

			public:
				double pixelNoise;

		};

	}
}

#endif /* OBSERVATIONPINHOLEEUCLIDEANPOINT_HPP_ */
