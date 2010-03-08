/**
 * indirectArray.hpp
 *
 *  Created on: 06/03/2010
 *      Author: jsola
 *
 *  \file indirectArray.hpp
 *
 *  ## Add a description here ##
 *
 * \ingroup jmath
 */

#ifndef INDIRECTARRAY_HPP_
#define INDIRECTARRAY_HPP_

#include <iostream>

#include "boost/numeric/ublas/operation.hpp"
#include "boost/numeric/ublas/io.hpp"
#include "jmath/jblas.hpp"

namespace std {
	/**
	 * Operator << for class for ind_array
	 * FIXME: This operator has been defined in namespace STD.
	 * FIXME: This is not necessarily beautiful but we cannot put it under jafar::jmath.
	 */
	template<class A>
	ostream& operator <<(ostream & s, boost::numeric::ublas::indirect_array<A> & ia_) {
		s << "[" << ia_.size() << "]{";
		for (size_t i = 0; i < ia_.size() - 1; i++)
			s << ia_(i) << ",";
		s << ia_(ia_.size() - 1) << "}";
		return s;
	}
}

namespace jafar {
	namespace jmath {
		namespace ublasExtra {

			/**
			 * Find the elements in indirect array \a ifull that are not contained in \a ipartial.
			 * \param ifull the full indirect array
			 * \param ipartial the partial indirect array
			 * \return the complementary indirect array
			 */
			jblas::ind_array ia_complement(const jblas::ind_array & ifull, const jblas::ind_array & ipartial);

			/**
			 * Make the union of two indirect arrays.
			 * \param ia1 an indirect array
			 * \param ia2 an indirect array
			 * \return the union of \a ia1 and \a ia2, with non-repeating indices, sorted from smallest to largest.
			 */
			jblas::ind_array ia_union(const jblas::ind_array & ia1, const jblas::ind_array & ia2);

			/**
			 * Return the N first indices of an indirect array
			 * \param ia the indirect array
			 * \param N the number of indices to return
			 * \return the N first indices in \a ia
			 */
			jblas::ind_array ia_head(const jblas::ind_array & ia, const size_t N);


		}
	}
}

#endif /* INDIRECTARRAY_HPP_ */
