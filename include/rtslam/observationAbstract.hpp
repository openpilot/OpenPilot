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
 * \ingroup rtslam
 */

#ifndef __ObservationAbstract_H__
#define __ObservationAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <jmath/jblas.hpp>

#include "rtslam/blocks.hpp"
#include "rtslam/gaussian.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/landmarkAbstract.hpp"

namespace jafar {

	namespace rtslam {

		// Forward declarations
		// TODO: check if this is OK.
		class SensorAbstract;
		class LandmarkAbstract;

		/** Base class for all landmark appearances defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class AppearanceAbstract {
				/**
				 * Mandatory virtual destructor
				 */
				virtual ~AppearanceAbstract(void);
		};

		/** Base class for all Gaussian expectations defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Expectation: public Gaussian {
			public:
				AppearanceAbstract * appearance;
				jblas::mat EXP_rob, EXP_sen, EXP_lmk; // Jacobians of the expectation wrt robot state, sensor state, lmk state.
				jblas::vec nonObs; // expected value of the non-observable part.
				bool visible; // landmark is visible (in Field Of View).
				double infoGain; // expected "information gain" of performing an update with this observation.

				virtual void computeVisibility(void);
				void estimateInfoGain(void);
		};

		/** Base class for all Gaussian measurements defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Measurement: public Gaussian {
			public:
				AppearanceAbstract * appearance;
				double score; ///< matching quality score
		};

		/** Base class for all Gaussian innovations defined in the module rtslam.
		 *
		 * It implements the trivial innovation model inn = meas - exp. It also returns the Jacobian matrices.
		 *
		 * Derive this class if you need other non-trivial innovation models (useful for line lendmarks).
		 *
		 * @ingroup rtslam
		 */
		class Innovation: public Gaussian {
			protected:
				/// The inverse of the innovation covariances matrix.
				jblas::sym_mat iP;
				/// The Mahalanobis distance from the measurement to the expectation.
				double mahalanobis_;
				/// The Jacobian of the innovation wrt the measurement.
				jblas::mat INN_meas;
				/// The Jacobian of the innovation wrt the expectation.
				jblas::mat INN_exp;
			public:

				/**
				 * the inverse of the innovation covariance.
				 */
				void invertCov(void) {
					jafar::jmath::ublasExtra::lu_inv(P, iP);
				}

				/**
				 * The Mahalanobis distance.
				 */
				double mahalanobis(void) {
					invertCov();
					mahalanobis_ = ublas::inner_prod(x, (jblas::vec) ublas::prod(iP, x));
					return mahalanobis_;
				}

				/**
				 * The trivial innovation function  inn = meas - exp .
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 */
				template<class V1, class V2>
				void func(V1& exp_mean, V2& meas_mean) {
					x = meas_mean - exp_mean;
				}

				/**
				 * The trivial innovation function inn = meas - exp. It updates the Jacobian matrices.
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 */
				template<class V1, class V2>
				void func_with_Jacobians(V1& exp_mean, V2& meas_mean) {
					func(exp_mean, meas_mean);
					INN_meas = jblas::identity_mat(exp_mean.size());
					INN_exp = -1.0 * jblas::identity_mat(exp_mean.size());
				}

				/**
				 * Compute full innovation
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 */
				void compute(Expectation& exp /// The expected Gaussian.
				    , Measurement& meas /// The measured Gaussian.
				) {
					func(exp.x, meas.x); // We do not request trivial Jacobians here. Jacobians are the identity.
					P = meas.P + exp.P; // Derived classes: P = Inn_meas*meas.P*Inn_meas.transpose() + Inn_exp*exp.P*Inn_exp.transpose();
				}
		};

		/** Base class for all observations defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class ObservationAbstract {
			public:

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~ObservationAbstract(void);

				/**
				 *  Mother Sensor where it was acquired from
				 */
				SensorAbstract * sensor;

				/**
				 * Father Landmark where it points to
				 */
				LandmarkAbstract * landmark;

				/**
				 * Expectation
				 */
				Expectation expectation;

				/**
				 * Measurement
				 */
				Measurement measurement;

				/**
				 * Innovation
				 */
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
				 * Project
				 * \return true if valid projection
				 */
				virtual bool project(void) = 0;

				/**
				 * Is visible
				 * \return true if inside FOV
				 */
				virtual bool isVisible(void) {
					return events.visible;
				}
				;

				/**
				 * match
				 * \return true if matched
				 */
				virtual bool match(void);

				/**
				 * Individual consistency check
				 * \return true if consistent
				 */
				virtual bool isIndividuallyConsistent(void);

				/**
				 * Back-project
				 */
				virtual void back_project(void) = 0;

				/**
				 * Operator << for class ObservationAbstract.
				 * It shows different information of the observation.
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::ObservationAbstract & obs) {
//					s << "OBSERVATION of " << obs.landmark->type << " from " << obs.sensor->type << endl;
//					s << "Sensor: " << obs.sensor->id << ", landmark: "	<< obs.landmark->id << endl;
					s << ".expectation:  " << obs.expectation << endl;
					s << ".measurement:  " << obs.measurement << endl;
					s << ".innovation:   " << obs.innovation << endl;
					return s;
				}

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
