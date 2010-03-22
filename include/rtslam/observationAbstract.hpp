/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2010
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      observationAbstract.h
 * Project:   RT-Slam
 * Author:    Joan Sola
 *
 * Version control
 * ===============
 *
 *  $Id$
 *
 * Description
 * ============
 *
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/**
 * \file observationAbstract.hpp
 * File defining the abstract observation class
 * \author jsola@laas.fr
 * \ingroup rtslam
 */

#ifndef __ObservationAbstract_H__
#define __ObservationAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <jmath/jblas.hpp>

#include "rtslam/rtSlam.hpp"
// include parents
#include "rtslam/objectAbstract.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/landmarkAbstract.hpp"

#include "rtslam/gaussian.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		/** Base class for all landmark appearances defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class AppearanceAbstract {
			public:
				/**
				 * Mandatory virtual destructor
				 */
				virtual ~AppearanceAbstract() {
				}
		};


		/** Base class for all Gaussian expectations defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Expectation: public Gaussian {

			public:

				typedef boost::shared_ptr<AppearanceAbstract> appearance_ptr_t;

				/**
				 * size constructor
				 */
				Expectation(const size_t _size);

				/**
				 * sizes constructor
				 */
				Expectation(const size_t _size, const size_t _size_nonobs);

				jblas::mat EXP_rob, EXP_sen, EXP_lmk; // Jacobians of the expectation wrt robot state, sensor state, lmk state.

				jblas::vec nonObs; // expected value of the non-observable part.

				void computeVisibility();

				void estimateInfoGain();

				inline bool isVisible(); // landmark is visible (in Field Of View).

				inline double infoGain(); // expected "information gain" of performing an update with this observation.

				inline void appearance(appearance_ptr_t _appearance);

			private:
				bool visible_;
				bool infoGain_;
				appearance_ptr_t appearance_;
		};


		/** Base class for all Gaussian measurements defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Measurement: public Gaussian {
			public:

				typedef boost::shared_ptr<AppearanceAbstract> appearance_ptr_t;

				inline void appearance(appearance_ptr_t _appearance);
				inline appearance_ptr_t appearance();
				Measurement(size_t _size);

				double score; ///< matching quality score

			private:
				appearance_ptr_t appearance_;

		};


		/** Base class for all Gaussian innovations defined in the module rtslam.
		 * It implements the trivial innovation model inn = meas - exp.
		 * It also returns the Jacobian matrices.
		 * Derive this class if you need other non-trivial innovation
		 * models (useful for line lendmarks).
		 *
		 * @ingroup rtslam
		 */
		class Innovation: public Gaussian {
			protected:
				/// The inverse of the innovation covariances matrix.
				jblas::sym_mat iP_;
				/// The Mahalanobis distance from the measurement to the expectation.
				double mahalanobis_;
				/// The Jacobian of the innovation wrt the measurement.
				jblas::mat INN_meas;
				/// The Jacobian of the innovation wrt the expectation.
				jblas::mat INN_exp;

			public:


				/**
				 * Size construction.
				 * \param _size the innovation size
				 */
				Innovation(const size_t _size);

				/**
				 * Sizes construction.
				 * \param _size the innovation size
				 * \param _size_meas the measurement size
				 * \param _size_exp the expectation size
				 */
				Innovation(const size_t _size, const size_t _size_meas, const size_t _size_exp);

				/**
				 * the inverse of the innovation covariance.
				 */
				void invertCov() {
					jafar::jmath::ublasExtra::lu_inv(P(), iP_);
				}


				/**
				 * The Mahalanobis distance.
				 */
				double mahalanobis() {
					invertCov();
					mahalanobis_ = ublas::inner_prod(x(), (jblas::vec) ublas::prod(iP_, x()));
					return mahalanobis_;
				}


				/**
				 * The trivial innovation function  inn = meas - exp.
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 * \param exp_mean the expectation mean
				 * \param meas_mean the measurement mean
				 */
				template<class V1, class V2>
				void compute(V1& exp_mean, V2& meas_mean) {
					x() = meas_mean - exp_mean;
				}


				/**
				 * The trivial innovation function inn = meas - exp.
				 * It updates the Jacobian matrices.
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 * \param exp_mean the expectation mean
				 * \param meas_mean the measurement mean
				 */
				template<class V1, class V2>
				void compute_with_Jacobians(V1& exp_mean, V2& meas_mean) {
					func(exp_mean, meas_mean);
					INN_meas = jblas::identity_mat(exp_mean.size());
					INN_exp = -1.0 * jblas::identity_mat(exp_mean.size());
				}


				/**
				 * Compute full innovation, with covariances matrix.
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 * \param exp_mean the expectation
				 * \param meas_mean the measurement
				 */
				void compute(Expectation& exp, Measurement& meas) {
					compute(exp.x(), meas.x()); // We do not request trivial Jacobians here. Jacobians are the identity.
					P() = meas.P() + exp.P(); // Derived classes: P = Inn_meas*meas.P*Inn_meas.transpose() + Inn_exp*exp.P*Inn_exp.transpose();
				}
		};


		/**
		 * Base class for all observations defined in the module rtslam.
		 * \author jsola
		 * \ingroup rtslam
		 */
		class ObservationAbstract: public ObjectAbstract {

				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::ObservationAbstract & obs);

			public:

				virtual ~ObservationAbstract(){}

				/**
				 * Size constructor
				 */
				ObservationAbstract(size_t _size);

				/**
				 * Sizes constructor
				 */
				ObservationAbstract(size_t _size_meas, size_t _size_exp, size_t _size_inn);

				//				/**
				//				 * Sensor and landmark constructor
				//				 */
				//				ObservationAbstract(SensorAbstract & _sen, LandmarkAbstract & _lmk);

				/**
				 *  Mother Sensor where it was acquired from
				 */
				sensor_ptr_t sensor;

				/**
				 * Father Landmark where it points to
				 */
				landmark_ptr_t landmark;

				Expectation expectation;

				Measurement measurement;

				Innovation innovation;

				/**
				 * Counters
				 */
				struct {
						int nSearch; ///< Number of searches
						int nMatch; ///< number of matches
						int nInlier; ///< Number of times declared inlier
				} counters;

				/**
				 * Events
				 */
				struct {
						bool visible; ///< Landmark is visible
						bool measured; ///< Feature is measured
						bool matched; ///< Feature is matched
						bool updated; ///< Landmark is updated
				} events;

				/**
				 * Associate to sensor and landmark.
				 * This sets several parameters such as identifiers and pointers to sensor and landmark ancestors.
				 * \param sen the sensor
				 * \param lmk the landmark
				 */
				inline void associate(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr);

				/**
				 * Link to sensor.
				 */
				void linkToSensor(sensor_ptr_t _sensorPtr);

				/**
				 * Link to landmark
				 */
				void linkToLandmark(landmark_ptr_t _lmkPtr);


				/**
				 * Project and get Jacobians
				 */
				//				virtual void project() = 0;

				/**
				 * Is visible
				 * \return true if visible
				 */
				virtual bool isVisible() {
					return events.visible;
				}


				/**
				 * match
				 */
				//				virtual void match();

				/**
				 * Individual consistency check
				 */
				//				virtual void isIndividuallyConsistent();

				/**
				 * Back-project
				 */
				//				virtual void back_project() = 0;

		};

	}

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
