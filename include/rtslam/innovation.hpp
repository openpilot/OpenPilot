/**
 * \file innovation.hpp
 *
 * \date 25/03/2010
 * \author jsola@laas.fr
 *
 *
 *  This file defines the class Innovation.
 *
 * \ingroup rtslam
 */

//test

#include "rtslam/expectation.hpp"
#include "rtslam/measurement.hpp"
#include "rtslam/gaussian.hpp"
#include "jmath/ublasExtra.hpp"

#ifndef INNOVATION_HPP_
#define INNOVATION_HPP_

namespace jafar {
	namespace rtslam {

		// encore


		/** Base class for all Gaussian innovations defined in the module rtslam.
		 * \author jsola@laas.fr
		 *
		 * It implements the trivial innovation model:
		 * - inn = meas - exp.
		 *
		 * which is, after all,
		 * - z = y - h(x)
		 *
		 * so usual in Kalman filtering.
		 *
		 * It also returns the Jacobian matrices:
		 * - INN_exp = dz/dh = -I
		 * - INN_meas = dz/dy = I
		 *
		 * Derive this class and overload the methods if you need other non-trivial innovation
		 * models (useful for line landmarks).
		 *
		 *\ingroup rtslam
		 */
		class Innovation: public Gaussian {
			public:

				jblas::sym_mat iP_; ///<        The inverse of the innovation covariances matrix.
				double mahalanobis_; ///<       The Mahalanobis distance from the measurement to the expectation.

				/**
				 * Size construction.
				 * Use this constructor for usual innovations with equal expectation, measurement and innovation sizes.
				 * \param _size the innovation size
				 */
				Innovation(const size_t _size);

				virtual ~Innovation(){} ///< mandatory virtual destructor

				/**
				 * the inverse of the innovation covariance.
				 */
				void invertCov();


				/**
				 * The Mahalanobis distance.
				 */
				double mahalanobis();

		};

	}
}

#endif /* INNOVATION_HPP_ */
