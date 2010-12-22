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
#include "boost/shared_ptr.hpp"

namespace jafar {
	namespace rtslam {

		class ObservationPinHoleEuclideanPoint;
		typedef boost::shared_ptr<ObservationPinHoleEuclideanPoint> obs_ph_euc_ptr_t;

		class ObservationModelPinHoleEuclideanPoint: public ObservationModelAbstract
		{
			public:
				typedef SensorPinHole sensor_spec_t;
				typedef boost::shared_ptr<sensor_spec_t> sensor_spec_ptr_t;
				typedef boost::weak_ptr<sensor_spec_t> sensor_spec_wptr_t;
			protected:
				sensor_spec_wptr_t sensorSpecWPtr;
			public:
				void linkToPinHole( sensor_spec_ptr_t ptr )
				{
					sensorSpecWPtr = ptr;
					ObservationModelAbstract::linkToSensor(ptr);
				}
				sensor_spec_ptr_t pinHolePtr( void )
				{
					sensor_spec_ptr_t sptr = sensorSpecWPtr.lock();
					if (!sptr) {
						std::cerr << __FILE__ << ":" << __LINE__ << " ObsSpec::sensor threw weak" << std::endl;
						throw "WEAK";
					}
					return sptr;
				}
				virtual void linkToSensorSpecific( sensor_ptr_t ptr )
				{
					boost::shared_ptr<SensorPinHole> sptr = SPTR_CAST<SensorPinHole>( ptr );
					if( sptr==NULL )
					{
						std::cerr << __FILE__ << ":" << __LINE__ << " : cast unfair." << std::endl;
						throw "CAST";
					}
					linkToPinHole( sptr );
				}
				
			protected:
				size_t exp_size, prior_size;
				void init_sizes() { exp_size = 2; prior_size = 1; }
			public:
				ObservationModelPinHoleEuclideanPoint()  { init_sizes(); }
				ObservationModelPinHoleEuclideanPoint(const sensor_ptr_t & pinholePtr);

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
				virtual bool predictVisibility_func(jblas::vec x, jblas::vec nobs);
			
		};

		/**
		 * Class for Pin-Hole observations of Euclidean 3D points.
		 * \author jsola@laas.fr
		 * \ingroup rtslam
		 */
		class ObservationPinHoleEuclideanPoint: public ObservationAbstract,
		    public SpecificChildOf<LandmarkEuclideanPoint>
		{
			public:
			// Define the function linkToParentEUC.
			ENABLE_LINK_TO_SPECIFIC_PARENT(LandmarkAbstract,LandmarkEuclideanPoint,EUC,ObservationAbstract);

			// Define the functions euc() and eucPtr().
			ENABLE_ACCESS_TO_SPECIFIC_PARENT(LandmarkEuclideanPoint,euc);

			boost::shared_ptr<ObservationModelPinHoleEuclideanPoint> modelSpec;
			void linkToPinHole( ObservationModelPinHoleEuclideanPoint::sensor_spec_ptr_t ptr ) { modelSpec->linkToPinHole(ptr); }
			ObservationModelPinHoleEuclideanPoint::sensor_spec_ptr_t pinHolePtr( void )  { return modelSpec->pinHolePtr(); }
			void linkToSensorSpecific( sensor_ptr_t ptr ) { modelSpec->linkToSensorSpecific(ptr); }
				
			public:

			ObservationPinHoleEuclideanPoint(const sensor_ptr_t & pinholePtr, const landmark_ptr_t & eucPtr);
			~ObservationPinHoleEuclideanPoint(void){
//				cout << "Deleted observation: " << id() << ": " << typeName() << endl;
					}

			void setup(double reparTh, int killSizeTh, int killSearchTh, double killMatchTh, double killConsistencyTh, double dmin);

				virtual std::string typeName() const {
					return "Pinhole-Euclidean-point";
				}


//				void setup(double _pixNoise = 1.0);


				/**
				 * Predict appearance
				 */
				virtual void predictAppearance_func();

				virtual double getMatchScore(){
					return measurement.matchScore;
				}

				virtual bool voteForReparametrizingLandmark(){
					return false;
				}

			public:
				double pixelNoise;

		};

	}
}

#endif /* OBSERVATIONPINHOLEEUCLIDEANPOINT_HPP_ */
