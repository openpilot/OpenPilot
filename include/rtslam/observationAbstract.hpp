/**
 * \file observationAbstract.hpp
 * File defining the abstract observation class
 * \author jsola
 * \ingroup rtslam
 */

#ifndef __ObservationAbstract_H__
#define __ObservationAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <jmath/jblas.hpp>
#include <boost/smart_ptr.hpp>

#include "rtslam/rtSlam.hpp"
// include parents
#include "rtslam/parents.hpp"
#include "rtslam/objectAbstract.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/appearanceAbstract.hpp"

#include "rtslam/gaussian.hpp"
#include "rtslam/innovation.hpp"
#include "rtslam/dataManagerAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace jblas;

		/**
		An observation model only contains pseudo-static functions
		A reference to the sensor is kepts because it has a lot of parameters that
		cannot easily be abstracted, and to avoid to cast an abstract sensor object every time.
		*/
		class ObservationModelAbstract
		{
			public:
				typedef boost::weak_ptr<SensorAbstract> sensor_wptr_t;
			protected:
				sensor_wptr_t sensorWPtr;
			public:
				void linkToSensor( sensor_ptr_t ptr )
				{
					sensorWPtr = ptr;
				}
				sensor_ptr_t sensorPtr( void )
				{
					sensor_ptr_t sptr = sensorWPtr.lock();
					if (!sptr) {
						std::cerr << __FILE__ << ":" << __LINE__ << " ObsSpec::sensor threw weak" << std::endl;
						throw "WEAK";
					}
					return sptr;
				}
				// Cast to specific sensor type in the specialized task.
				virtual void linkToSensorSpecific( sensor_ptr_t ptr ) = 0;
			public:
				/**
				 * Project.
				 *
				 * This projects the landmark into the sensor space.
				 * \param sg global sensor pose
				 * \param lmk landmark state vector
				 * \param exp expectation vector
				 * \param nobs non-observable vector
				 */
				virtual void
				project_func(const vec7 & sg, const vec & lmk, vec & meas, vec & nobs) = 0;

				/**
				 * Project and get Jacobians.
				 *
				 * This projects the landmark into the sensor space, and gives the Jacobians of this projection
				 * wrt. the global sensor pose and the landmark.
				 * \param sg global sensor pose
				 * \param lmk landmark state vector
				 * \param exp expectation vector
				 * \param nobs non-observable vector
				 * \param EXP_sg Jacobian of \a exp wrt \a sg
				 * \param EXP_lmk Jacobian of \a exp wrt \a lmk
				 */
				virtual void
				project_func(const vec7 & sg, const vec & lmk, vec & exp, vec & nobs, mat & EXP_sg, mat & EXP_lmk) = 0;
			
				/**
				 * Back-project function
				 */
				virtual void backProject_func(const vec7 & sg, const vec & meas, const vec & nobs, vec & lmk) = 0;

				/**
				 * Back-project function
				 */
				virtual void backProject_func(const vec7 & sg, const vec & meas, const vec & nobs, vec & lmk, mat & LMK_sg,
				                              mat & LMK_meas, mat & LMK_nobs) = 0;

				virtual bool predictVisibility_func(jblas::vec x, jblas::vec nobs) = 0;
		};


		/**
		 * Base class for all observations defined in the module rtslam.
		 * \author jsola
		 *
		 * In this class, the Jacobians are sparse. The states that contribute to the observation are available through an indirect array
		 * - this->ia_rsl
		 *
		 * where we use the subindex *_rsl to mean that it corresponds to robot, sensor and landmark map states.
		 *
		 * With this we have, for example:
		 * - expectation.x() = h( ublas::project(x, ia_rsl) )
		 * - expectation.P() = EXP_rsl * ublas::project(P, ia_rsl, ia_rsl) * EXP_rsl'
		 *
		 * \ingroup rtslam
		 */
		class ObservationAbstract: public ObjectAbstract,
		    public ChildOf<LandmarkAbstract> ,
		    public boost::enable_shared_from_this<ObservationAbstract>,
		    public ChildOf<DataManagerAbstract> {

				friend std::ostream& operator <<(std::ostream & s, ObservationAbstract const & obs);
				friend image::oimstream& operator <<(image::oimstream & s, ObservationAbstract const & obs);

				// define the function linkToParentLandmark().
				ENABLE_LINK_TO_PARENT(LandmarkAbstract,Landmark,ObservationAbstract)
				;
				// define the functions landmarkPtr() and landmark().
				ENABLE_ACCESS_TO_PARENT(LandmarkAbstract,landmark)
				;
				// define the function linkToParentDataManager().
				ENABLE_LINK_TO_PARENT(DataManagerAbstract,DataManager,ObservationAbstract)
				;
				// define the functions dataManagerPtr() and dataManager().
				ENABLE_ACCESS_TO_PARENT(DataManagerAbstract,dataManager)
				;

				void linkToSensor( sensor_ptr_t ptr ) { model->linkToSensor(ptr); }
				sensor_ptr_t sensorPtr( void ) { return model->sensorPtr(); }
				const sensor_ptr_t sensorPtr( void ) const { return model->sensorPtr(); }
				void linkToSensorSpecific( sensor_ptr_t ptr ) { model->linkToSensorSpecific(ptr); }

			public:

				boost::shared_ptr<ObservationModelAbstract> model;
				
				enum type_enum {
					PNT_PH_EUC, ///< Pin hole Euclidean point
               PNT_PH_AH, ///< Pin hole Anchored homogeneous point
               PNT_PH_AHPL ///< Pin hole Anchored homogeneous points line
				};


				/**
				 * Size constructor
				 * \param _size size of measurement space (used for measurement, expectation and innovation).
				 */
				ObservationAbstract(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr, const size_t _size,
				                    const size_t size_nonobs = 0);

				/**
				 * Sizes constructor
				 */
				ObservationAbstract(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr, const size_t _size_meas,
				                    const size_t _size_exp, const size_t _size_inn, const size_t _size_nonobs = 0);

				virtual ~ObservationAbstract();

				void setPrior(const Gaussian & _prior);

				// Data
				Expectation expectation;
				Measurement measurement;
				Innovation innovation;
				Gaussian prior;
				appearance_ptr_t predictedAppearance;
				appearance_ptr_t observedAppearance;
				jblas::sym_mat noiseCovariance;

				// indirect arrays
				ind_array ia_rsl; ///<    Ind. array of mapped indices of robot, sensor and landmark (ie, sensor might or might not be there).

			public:
				// Jacobians
				mat SG_rs;     ///< Jacobian of global sensor pose wrt. robot and sensor mapped states
				mat EXP_sg;    ///< Jacobian of expectation wrt. global sensor pose
				mat EXP_l;     ///< Jacobian of expectation wrt. landmark state
				mat EXP_rsl;   ///< Jacobian of the expectation wrt. the mapped states of robot, sensor and landmark.
				mat INN_meas;  ///< Jacobian of the innovation wrt. the measurement.
				mat INN_exp;   ///< Jacobian of the innovation wrt. the expectation.
				mat INN_rsl;   ///< Jacobian of the innovation wrt. the mapped states of robot, sensor and landmark.
				mat LMK_sg;    ///< Jacobian of the landmark wrt. the global sensor pose.
				mat LMK_meas;  ///< Jacobian of the landmark wrt. the measurement.
				mat LMK_prior; ///< Jacobian of the landmark wrt. the prior.
				mat LMK_rs;    ///< Jacobian of the landmark wrt. the robot and sensor mapped states.

			public:
				/**
				 * Counters
				 */
				struct Counters {
						int nSearch; ///< Number of searches
						int nMatch;  ///< Number of matches
						int nInlier; ///< Number of times declared inlier
						int nSearchSinceLastInlier; ///< Number of frames the landmark was searched since last time it was inlier
						int nFrameSinceLastVisible; ///< Number of frames since last time it was visible
				} counters;

				/**
				 * Events
				 */
				struct Events {
						bool predicted;    ///< Landmark is not new and has been predicted
						bool visible;      ///< Landmark is visible
						bool measured;     ///< Feature is measured (we tried to match it)
						bool matched;      ///< Feature is successfully matched
						bool updated;      ///< Landmark is updated
				} events;
				
				/**
				 * Tasks
				 */
				struct Tasks {
						bool predictedApp; ///< Appearance has been predicted for predicted pos
				} tasks;

				int searchSize;

				type_enum type;

				void setId(){
					id(landmarkPtr()->id());
				}

				virtual std::string typeName() const {return "Abstract";}
				std::string categoryName() const {
					return "OBSERVATION";
				}

				/**
				 * Project and get expectation covariances
				 */
				void project();

				/**
				 * Only project (without covariances)
				 */
				void projectMean();
				
				/**
				 * Is visible
				 * \return true if visible
				 */
				virtual bool isVisible() {
					return events.visible;
				}
				
				/**
				 * Back-project
				 */
				void backProject();

				/**
				 * Compute innovation from measurement and expectation.
				 *
				 * This is the trivial innovation function  inn = meas - exp.
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 */
				virtual void computeInnovation();
				virtual void computeInnovationMean(vec &inn, const vec &meas, const vec &exp) const;
				virtual double computeRelevance();

				/**
				 * Predict information gain.
				 *
				 * This is the trivial information gain computed with det(expectation.P()).
				 * Derive this class and overload this method to implement other formulas.
				 */
				virtual void predictInfoGain();

				/**
				 * Individual compatibility test.
				 *
				 * This is the trivial individual compatibility test based on the Mahalanobis distance.
				 *
				 * \param MahaDistSquare the Mahalanobis distance square to test against.
				 *
				 * Use the chi-square table below to choose a value of MahaDistSquare:
				 * \code
				 * PROB:   0.995     0.975      0.20    0.10    0.05    0.025   0.02    0.01    0.005   0.002   0.001
				 * DOF: +--------------------- CHI SQUARE VALUES -- OR -- MAHADISTSQUARE VALUES ---------------------
				 *  1   |  0.0000393 0.000982   1.642   2.706   3.841   5.024   5.412   6.635   7.879   9.550  10.828
				 *  2   |  0.0100    0.0506     3.219   4.605   5.991   7.378   7.824   9.210  10.597  12.429  13.816
				 *  3   |  0.0717    0.216      4.642   6.251   7.815   9.348   9.837  11.345  12.838  14.796  16.266
				 *  4   |  0.207     0.484      5.989   7.779   9.488  11.143  11.668  13.277  14.860  16.924  18.467
				 *  5   |  0.412     0.831      7.289   9.236  11.070  12.833  13.388  15.086  16.750  18.907  20.515
				 *  6   |  0.676     1.237      8.558  10.645  12.592  14.449  15.033  16.812  18.548  20.791  22.458
				 *  7   |  0.989     1.690      9.803  12.017  14.067  16.013  16.622  18.475  20.278  22.601  24.322
				 *  8   |  1.344     2.180     11.030  13.362  15.507  17.535  18.168  20.090  21.955  24.352  26.124
				 *  9   |  1.735     2.700     12.242  14.684  16.919  19.023  19.679  21.666  23.589  26.056  27.877
				 * 10   |  2.156     3.247     13.442  15.987  18.307  20.483  21.161  23.209  25.188  27.722  29.588
				 * 11   |  2.603     3.816     14.631  17.275  19.675  21.920  22.618  24.725  26.757  29.354  31.264
				 * 12   |  3.074     4.404     15.812  18.549  21.026  23.337  24.054  26.217  28.300  30.957  32.909
				 * 13   |  3.565     5.009     16.985  19.812  22.362  24.736  25.472  27.688  29.819  32.535  34.528
				 * 14   |  4.075     5.629     18.151  21.064  23.685  26.119  26.873  29.141  31.319  34.091  36.123
				 * 15   |  4.601     6.262     19.311  22.307  24.996  27.488  28.259  30.578  32.801  35.628  37.697
				 * 16   |  5.142     6.908     20.465  23.542  26.296  28.845  29.633  32.000  34.267  37.146  39.252
				 * 17   |  5.697     7.564     21.615  24.769  27.587  30.191  30.995  33.409  35.718  38.648  40.790
				 * 18   |  6.265     8.231     22.760  25.989  28.869  31.526  32.346  34.805  37.156  40.136  42.312
				 * 19   |  6.844     8.907     23.900  27.204  30.144  32.852  33.687  36.191  38.582  41.610  43.820
				 * 20   |  7.434     9.591     25.038  28.412  31.410  34.170  35.020  37.566  39.997  43.072  45.315
				 * \endcode
				 */
				virtual bool compatibilityTest(const double mahaDist);

				/**
				 * Clear all event flags
				 */
				void clearFlags();
				void clearCounters();


				/**
				 * Predict visibility.
				 *
				 * Implement this in the derived class.
				 *
				 * Visibility can only be established after project_func() has been called.
				 *
				 * \return true if landmark is predicted visible.
				 */
				virtual bool predictVisibility()
				{
					events.visible = model->predictVisibility_func(expectation.x(), expectation.nonObs);
					return events.visible;
				}
				
				
				/**
				 * Predict appearance
				 */
				virtual bool predictAppearance(bool force = false)
				{
					if (force || !tasks.predictedApp)
					{
//JFR_DEBUG("predictAppearance");
						if (predictAppearance_func())
						{
							tasks.predictedApp = true;
							return true;
						}
						return false;
					}
					return true;
				}
				
				virtual bool updateDescriptor()
				{
					return landmarkPtr()->descriptorPtr->addObservation(this->shared_from_this());
				}
				virtual void updateVisibilityMap()
				{
					landmarkPtr()->visibilityMap.addObservation(this->shared_from_this());
				}
				
				virtual bool isDescriptorValid()
				{
					return landmarkPtr()->descriptorPtr->isPredictionValid(this->shared_from_this());
				}

				virtual bool predictAppearance_func() = 0;

				virtual double getMatchScore() = 0;

				void update() ;
#if 0
				virtual bool voteForKillingLandmark();
#endif
				/// return a linearity score for the associated converged type. 0 means fully linear. <0 means already converged.
				virtual double computeLinearityScore() = 0; 

				virtual void transferInfoObs(observation_ptr_t & obs);

				virtual void desc_image(image::oimstream& os) const {}
				
		};

	}

}


#define ENABLE_LINK_TO_SENSOR_SPEC(className)            \
	protected:\
	  sensor_spec_wptr_t sensorSpecWPtr;\
	public:\
	  void linkTo##className( sensor_spec_ptr_t ptr )\
	  {\
	    sensorSpecWPtr = ptr;\
	    ObservationAbstract::linkToSensor(ptr);\
	  }


#define ENABLE_ACCESS_TO_SENSOR_SPEC(accessName)            \
	  sensor_spec_ptr_t accessName##Ptr( void )\
	  {\
	    sensor_spec_ptr_t sptr = sensorSpecWPtr.lock();\
	    if (!sptr) {\
	      std::cerr << __FILE__ << ":" << __LINE__\
			<< " ObsSpec::sensor threw weak" << std::endl;\
	      throw "WEAK";\
	    }\
	    return sptr;\
	  }



#endif // #ifndef __ObservationAbstract_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
