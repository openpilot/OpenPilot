/**
 * \file observationPinHoleAnchoredHomogeneous.hpp
 *
 * Header file for observations of Anchored Homogeneous Points (AHP) from pin-hole cameras.
 *
 * \date 14/02/2010
 * \author jsola
 *
 * \ingroup rtslam
 */

#ifndef OBSERVATIONPINHOLEANCHOREDHOMOGENEOUS_HPP_
#define OBSERVATIONPINHOLEANCHOREDHOMOGENEOUS_HPP_

#include "rtslam/observationAbstract.hpp"
#include "rtslam/sensorPinhole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
//#include "rtslam/parents.hpp"
#include "boost/shared_ptr.hpp"

namespace jafar {
	namespace rtslam {

		class ObservationPinHoleAnchoredHomogeneousPoint;
		typedef boost::shared_ptr<ObservationPinHoleAnchoredHomogeneousPoint> obs_ph_ahp_ptr_t;

		
		class ObservationModelPinHoleAnchoredHomogeneousPoint: public ObservationModelAbstract
		{
			public:
				typedef SensorPinhole sensor_spec_t;
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
					boost::shared_ptr<SensorPinhole> sptr = SPTR_CAST<SensorPinhole>( ptr );
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

				ObservationModelPinHoleAnchoredHomogeneousPoint() { init_sizes(); }
				ObservationModelPinHoleAnchoredHomogeneousPoint(const sensor_ptr_t & pinholePtr);
			
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
				virtual void backProject_func(const vec7 & sg, const vec & meas, const vec & nobs, vec & lmk, mat & LMK_sg,
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
		 * Class for Pin-Hole observations of Anchored Homogeneous 3D points.
		 * \author jsola
		 * \ingroup rtslam
		 */
		class ObservationPinHoleAnchoredHomogeneousPoint: public ObservationAbstract,
 	    public SpecificChildOf<LandmarkAnchoredHomogeneousPoint>
		{
			public:
			// Define the function linkToParentAHP.
			ENABLE_LINK_TO_SPECIFIC_PARENT(LandmarkAbstract,LandmarkAnchoredHomogeneousPoint,AHP,ObservationAbstract)
				;
				// Define the functions ahp() and ahpPtr().
			ENABLE_ACCESS_TO_SPECIFIC_PARENT(LandmarkAnchoredHomogeneousPoint,ahp)
				;

			boost::shared_ptr<ObservationModelPinHoleAnchoredHomogeneousPoint> modelSpec;
			void linkToPinHole( ObservationModelPinHoleAnchoredHomogeneousPoint::sensor_spec_ptr_t ptr ) { modelSpec->linkToPinHole(ptr); }
			ObservationModelPinHoleAnchoredHomogeneousPoint::sensor_spec_ptr_t pinHolePtr( void )  { return modelSpec->pinHolePtr(); }
			void linkToSensorSpecific( sensor_ptr_t ptr ) { modelSpec->linkToSensorSpecific(ptr); }
		
		public:

				ObservationPinHoleAnchoredHomogeneousPoint(const sensor_ptr_t & pinholePtr, const landmark_ptr_t & ahpPtr);
				~ObservationPinHoleAnchoredHomogeneousPoint(void) {
//					cout << "Deleted observation: " << id() << ": " << typeName() << endl;
				}

				virtual std::string typeName() const {
					return "Pinhole-Anch-homog-point";
				}

				void setup(double dmin);

//				void setup(double _pixNoise = 1.0);

				/**
				 * Predict appearance
				 */
				virtual bool predictAppearance_func();
			

				virtual double getMatchScore(){
					return measurement.matchScore;
				}

				double computeLinearityScore(){
					return lmkAHP::linearityScore(sensorPtr()->globalPose(), landmarkPtr()->state.x(), landmarkPtr()->state.P());
				}
				
				virtual void desc_image(image::oimstream& os) const;

			public:
				double pixelNoise;

		};

	}
}

#endif /* OBSERVATIONPINHOLEANCHOREDHOMOGENEOUS_HPP_ */
