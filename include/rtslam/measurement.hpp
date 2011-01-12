/**
 * \file measurement.hpp
 *
 * \date 25/03/2010
 * \author jsola
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "jmath/jblas.hpp"
#include "rtslam/gaussian.hpp"


#ifndef MEASUREMENT_HPP_
#define MEASUREMENT_HPP_


namespace jafar {
	namespace rtslam {


		/** Base class for all Gaussian measurements defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Measurement: public Gaussian {
			public:

				Measurement(size_t _size);

				double matchScore; ///< matching quality score

		};


	}
}

#endif /* MEASUREMENT_HPP_ */
