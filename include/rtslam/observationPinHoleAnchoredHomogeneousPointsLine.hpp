/**
 * \file observationPinHoleAnchoredHomogeneousPointsLine.hpp
 *
 * Header file for observations of Anchored Homogeneous Points Lines (AHPL) from pin-hole cameras.
 *
 * \date 22/02/2011
 *     \author bhautboi@laas.fr
 *
 * \ingroup rtslam
 */

#ifndef OBSERVATIONPINHOLEANCHOREDHOMOGENEOUSPOINTSLINE_HPP
#define OBSERVATIONPINHOLEANCHOREDHOMOGENEOUSPOINTSLINE_HPP
#include "rtslam/observationAbstract.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPointsLine.hpp"
//#include "rtslam/parents.hpp"
#include "boost/shared_ptr.hpp"

namespace jafar {
   namespace rtslam {

      class ObservationPinHoleAnchoredHomogeneousPointsLine;
      typedef boost::shared_ptr<ObservationPinHoleAnchoredHomogeneousPointsLine> obs_ph_ahpl_ptr_t;

      class ObservationModelPinHoleAnchoredHomogeneousPointsLine: public ObservationModelAbstract
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
            void init_sizes() { exp_size = 4; prior_size = 2; }
         public:

            ObservationModelPinHoleAnchoredHomogeneousPointsLine() { init_sizes(); }
            ObservationModelPinHoleAnchoredHomogeneousPointsLine(const sensor_ptr_t & pinholePtr);

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
       * Class for Pin-Hole observations of Lines based on Anchored Homogeneous 3D points.
       * \author bhautboi@laas.fr
       * \ingroup rtslam
       */
      class ObservationPinHoleAnchoredHomogeneousPointsLine: public ObservationAbstract,
       public SpecificChildOf<LandmarkAnchoredHomogeneousPointsLine>
      {
         public:
         // Define the function linkToParentAHP.
         ENABLE_LINK_TO_SPECIFIC_PARENT(LandmarkAbstract,LandmarkAnchoredHomogeneousPointsLine,AHP,ObservationAbstract)
            ;
            // Define the functions ahp() and ahpPtr().
         ENABLE_ACCESS_TO_SPECIFIC_PARENT(LandmarkAnchoredHomogeneousPointsLine,ahp)
            ;

         boost::shared_ptr<ObservationModelPinHoleAnchoredHomogeneousPointsLine> modelSpec;
         void linkToPinHole( ObservationModelPinHoleAnchoredHomogeneousPointsLine::sensor_spec_ptr_t ptr ) { modelSpec->linkToPinHole(ptr); }
         ObservationModelPinHoleAnchoredHomogeneousPointsLine::sensor_spec_ptr_t pinHolePtr( void )  { return modelSpec->pinHolePtr(); }
         void linkToSensorSpecific( sensor_ptr_t ptr ) { modelSpec->linkToSensorSpecific(ptr); }

      public:

            ObservationPinHoleAnchoredHomogeneousPointsLine(const sensor_ptr_t & pinholePtr, const landmark_ptr_t & ahplPtr);
            ~ObservationPinHoleAnchoredHomogeneousPointsLine(void) {
//					cout << "Deleted observation: " << id() << ": " << typeName() << endl;
            }

            virtual std::string typeName() const {
               return "Pinhole-Anch-homog-points-line";
            }


            void setup(double reparTh, int killSizeTh, int killSearchTh, double killMatchTh, double killConsistencyTh, double dmin);

//				void setup(double _pixNoise = 1.0);

            /**
             * Predict appearance
             */
            virtual bool predictAppearance_func();


            virtual double getMatchScore(){
               return measurement.matchScore;
            }

            virtual bool voteForReparametrizingLandmark();

            virtual void desc_image(image::oimstream& os) const;

            virtual void computeInnovation();
      //      virtual void computeInnovationMean(vec &inn, const vec &meas, const vec &exp) const;

         public:
            double pixelNoise;
      };

   }
}

#endif // OBSERVATIONPINHOLEANCHOREDHOMOGENEOUSPOINTSLINE_HPP
