/**
 * kalmanFilter.hpp
 *
 *  Created on: 04/03/2010
 *      Author: jsola
 *
 *  \file kalmanFilter.hpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */




#ifndef KALMANFILTER_HPP_
#define KALMANFILTER_HPP_

#include "rtslam/gaussian.hpp"

namespace jafar
{
	namespace rtslam
	{

		/**
		 * Base class for Kalman filters
		 * \ingroup rtslam
		 */
		class KalmanFilter {
			public:
				jblas::vec x;
				jblas::sym_mat P;
				size_t size;
				boost::posix_time::time_duration curTime;
				jblas::mat K;
				jblas::mat PHt_tmp;
				jblas::mat KH_tmp;

				// TODO: define API for all these functions.
				inline void predict();
				inline void correct();
				inline void project();
				inline void computeInnovation();
				inline void computeK();
				inline void updateP();
				inline void stackCorrection();
				inline void correctAllStacked();

		};

	}
}

#endif /* KALMANFILTER_HPP_ */
