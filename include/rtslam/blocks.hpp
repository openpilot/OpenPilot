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
//#include "boost/numeric/ublas/storage.hpp"
#include "rtslam/gaussian.hpp"

// Shortcut for namespace ublas
namespace ublas = boost::numeric::ublas;

namespace jafar {
	namespace rtslam {

		/**
		 * Base class for all parameter sets in module rtslam.
		 * \ingroup rtslam
		 */
		class ParametersAbstract {
			public:
				/**
				 * Mandatory virtual destructor.
				 */
				inline virtual ~ParametersAbstract(void) {
				}

		};


	}
}

#endif /* BLOCKS_H_ */
