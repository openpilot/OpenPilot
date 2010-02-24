/*
 * Blocks.h
 *
 *     Project: rtslam
 *  Created on: Jan 29, 2010
 *      Author: jsola@laas.fr
 */

/**
 * \file blocks.hpp
 * File defining some useful blocks classes that are used here and there in RTSLAM.
 * Initially, this file contains states and poses derived from the Gaussian class.
 * \ingroup rtslam
 */

#ifndef BLOCKS_H_
#define BLOCKS_H_

#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"
#include "gaussian.hpp"

// Shortcut for namespace ublas
namespace ublas = boost::numeric::ublas;

namespace jafar {
	namespace rtslam {

		/**
		 * Base class for all parameter sets in module rtslam.
		 */
		class ParametersAbstract {
				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~ParametersAbstract(void);
		};

		/** Base class for all Gaussian state vectors defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class State: public Gaussian {
		};

		/**
		 * Class for Gaussian poses with selectable Direct or Indirect access (with local or remote storage)
		 * \ingroup rtslam
		 */
		class Pose: public Gaussian {
			public:
				/**
				 * Position
				 */
				jblas::vec3 position(void) {
					jblas::vec3 res;
					res.assign(subvector(x, 0, 3));
					return res;
				}
				/**
				 * Orientation quaternion
				 */
				jblas::vec4 quaternion(void) {
					jblas::vec4 res;
					res.assign(subvector(x, 3, 7));
				}
		};

	}
}

#endif /* BLOCKS_H_ */
