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
			 * Return the N first indices of an indirect array.
			 * \param ia the indirect array
			 * \param N the number of indices to return
			 * \return the N first indices in \a ia
			 */
			jblas::ind_array ia_head(const jblas::ind_array & ia, const size_t N);

			/**
			 * Create array from a boolean vector.
			 * The size of the returned indirect array is the number of true entries in the boolean vector.
			 * The indices in the array are the positions of the true values in the boolean vector.
			 * \param vb a boolean vector
			 * \return the indirect array
			 */
			jblas::ind_array ia_set(const jblas::vecb & vb);

			/**
			 * Create indirect array from boolean vector and take the head N elements
			 * The size of the returned array is the number of requested elements.
			 * It is an error if not enough elements are present in the input boolean vector.
			 * \param vb the boolean vector.
			 * \param N the number of elements to be returned.
			 * \return the indirect array.
			 */
			jblas::ind_array ia_head(const jblas::vecb & vb, const size_t N);

			/**
			 * Create indirect array from boolean vector and take the head N elements, and clear these N elements form the boolean.
			 * The size of the returned array is the number of requested elements.
			 * It is an error if not enough elements are present in the input boolean vector.
			 * \param vb the boolean vector.
			 * \param N the number of elements to be returned.
			 * \return the indirect array.
			 */
			jblas::ind_array ia_popfront(jblas::vecb & vb, const size_t N);

			/**
			 * Create indirect array from range.
			 * \param r a range.
			 * \return the indirect array.
			 */
			jblas::ind_array ia_range(const ublas::range & r);

			/**
			 * Create indirect array from start and end indices.
			 * The indirect array goes from \a begin to \(end-1).
			 * \param begin the first index
			 * \param end the last index plus one.
			 * \return the indirect array.
			 */
			jblas::ind_array ia_range(size_t begin, size_t end);

		}
	}
}

#endif /* INDIRECTARRAY_HPP_ */
