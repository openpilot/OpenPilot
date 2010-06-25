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
 	    public SpecificChildOf<LandmarkAnchoredHomogeneousPoint>
		{
			public:
			// Define the function linkToParentAHP.
			ENABLE_LINK_TO_SPECIFIC_PARENT(LandmarkAbstract,LandmarkAnchoredHomogeneousPoint,AHP,ObservationAbstract)
				;
				// Define the functions ahp() and ahpPtr().
			ENABLE_ACCESS_TO_SPECIFIC_PARENT(LandmarkAnchoredHomogeneousPoint,ahp)
				;

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
		    ObservationAbstract::linkToSensor(ptr);
		  }
		  sensor_spec_ptr_t pinHolePtr( void )
		  {
		    sensor_spec_ptr_t sptr = sensorSpecWPtr.lock();
		    if (!sptr) {
		      std::cerr << __FILE__ << ":" << __LINE__
				<< " ObsSpec::sensor threw weak" << std::endl;
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

		public:

				ObservationPinHoleAnchoredHomogeneousPoint(const sensor_ptr_t & pinholePtr, const landmark_ptr_t & ahpPtr);
				~ObservationPinHoleAnchoredHomogeneousPoint(void) {
					cout << "Deleted observation: " << id() << ": " << typeName() << endl;
				}

				virtual std::string typeName() {
					return "Obs. Pinhole Anc. homog. point";
				}


				void setup(const sensor_ptr_t & pinholePtr, const landmark_ptr_t & ahpPtr, double _noiseStd, int patchSize);

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
				virtual void backProject_func(const vec7 & sg, const vec & meas, const vec & nobs, vec & lmk, mat & LMK_sg,
				                      mat & LMK_meas, mat & LMK_nobs);

				/**
				 * Predict visibility.
				 *
				 * Visibility can only be established after project_func() has been called.
				 *
				 * \return true if landmark is predicted visible.
				 */
				bool predictVisibility();

				/**
				 * Predict appearance
				 */
				virtual void predictAppearance();

				virtual double getMatchScore(){
					return measurement.matchScore;
				}

				virtual bool voteForReparametrizingLandmark();

			public:
				double pixelNoise;

		};

	}
}

#endif /* OBSERVATIONPINHOLEANCHOREDHOMOGENEOUS_HPP_ */
