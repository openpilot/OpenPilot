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
				jblas::vec2 std_est; ///< estimation of std dev based on correl max curv
				
				friend std::ostream& operator <<(std::ostream & s, Measurement const & m_) {
					s << (rtslam::Gaussian&)(m_) << std::endl << "  .est std dev: " << m_.std_est;
					return s;
				}
		};


	}
}

#endif /* MEASUREMENT_HPP_ */
