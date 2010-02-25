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

#include "blocks.hpp"
#include "gaussian.hpp"
#include "sensorAbstract.hpp"
#include "landmarkAbstract.hpp"

namespace jafar
{

	namespace rtslam
	{

		/** Base class for all landmark appearances defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class AppearanceAbstract
		{
				/**
				 * Mandatory virtual destructor
				 */
				virtual ~AppearanceAbstract(void);
		};

		/** Base class for all Gaussian expectations defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Expectation: public Gaussian
		{
			public:
				AppearanceAbstract * appearance;
				jblas::mat EXP_rob, EXP_sen, EXP_lmk; // Jacobians of the expectation wrt robot state, sensor state, lmk state.
				jblas::vec nonObs; // expected value of the non-observable part.
				bool visible; // landmark is visible (in Field Of View).
				double infoGain; // expected "information gain" of performing an update with this observation.

				void computeVisibility(void) = 0;
				void estimateInfoGain(void);
		};

		/** Base class for all Gaussian measurements defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Measurement: public Gaussian
		{
			public:
				AppearanceAbstract * appearance;
				double score;  ///< matching quality score
		};

		/** Base class for all Gaussian innovations defined in the module rtslam.
		 *
		 * It implements the trivial innovation model inn = meas - exp. It also returns the Jacobian matrices.
		 *
		 * Derive this class if you need other non-trivial innovation models (useful for line lendmarks).
		 *
		 * @ingroup rtslam
		 */
		class Innovation: public Gaussian
		{
			protected:
				/// The inverse of the innovation covariances matrix.
				jblas::sym_mat iP;
				/// The Mahalanobis distance from the measurement to the expectation.
				double mahalanobis;
				/// The Jacobian of the innovation wrt the measurement.
				jblas::mat INN_meas;
				/// The Jacobian of the innovation wrt the expectation.
				jblas::mat INN_exp;
			public:

				/**
				 * the inverse of the innovation covariance.
				 */
				void invertCov(void)
				{
					ublas::lu_inv(P, iP);
				}

				/**
				 * The Mahalanobis distance.
				 */
				void mahalanobis(void)
				{
					iP = invCov(P);
					mahalanobis = ublas::inner_prod(x, (jblas::vec) ublas::prod(iP, x));
				}

				/**
				 * The trivial innovation function  inn = meas - exp .
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 */
				void func(jblas::vec& exp_mean /// The expected mean value.
				    , jblas::vec& meas_mean /// The measured mean value.
				)
				{
					x = meas_mean - exp_mean;
				}

				/**
				 * The trivial innovation function inn = meas - exp. It updates the Jacobian matrices.
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 */
				void func_with_Jacobians(Expectation& exp_mean /// The expected mean value.
				    , Measurement& meas_mean /// The measured mean value.
				)
				{
					func(exp_mean, meas_mean);
					Inn_meas = identity_mat(size_exp);
					Inn_exp = -1.0 * identity_mat(size_exp);
				}

				/**
				 * Compute full innovation
				 * Derive the class and overload this method to use other, non-trivial innovation functions.
				 */
				void compute(Expectation& exp /// The expected Gaussian.
				    , Measurement& meas /// The measured Gaussian.
				)
				{
					func(exp, meas); // We do not request trivial Jacobians here. Jacobians are the identity.
					P = meas.P + exp.P; // Derived classes: P = Inn_meas*meas.P*Inn_meas.transpose() + Inn_exp*exp.P*Inn_exp.transpose();
				}
		};

		/** Base class for all observations defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class ObservationAbstract
		{
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
				struct
				{
						int nSearch; ///< Number of searches
						int nMatch;  ///< number of matches
						int nInlier; ///< Number of times declared inlier
				} counters;

				/**
				 * Events
				 */
				struct
				{
						bool visible;  ///< Landmark is visible
						bool measured; ///< Feature is measured
						bool matched;  ///< Feature is matched
						bool updated;  ///< Landmark is updated
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
				virtual bool isVisible(void){return events.visible;};

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
