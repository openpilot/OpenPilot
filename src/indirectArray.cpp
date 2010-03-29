/**
 * indirectArray.cpp
 *
 *  Created on: 06/03/2010
 *      Author: jsola
 *
 *  \file indirectArray.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup jmath
 */

#include "jmath/indirectArray.hpp"
#include "jmath/jmathException.hpp"
#include "iostream"

using namespace std;

namespace jafar {
	namespace jmath {
		namespace ublasExtra {


			/**
			 * Complement of ind. array
			 */
			jblas::ind_array ia_complement(const jblas::ind_array & ifull, const jblas::ind_array & ipartial) {

				size_t i_max = 0;
				for (size_t i = 0; i < ifull.size(); i++)
					if (ifull(i) > i_max)
						i_max = ifull(i);
				for (size_t i = 0; i < ipartial.size(); i++)
					if (ipartial(i) > i_max)
						i_max = ipartial(i);

				jblas::vecb is_in_full(i_max + 1);
				is_in_full.clear();
				for (size_t i = 0; i < ifull.size(); i++)
					is_in_full(ifull(i)) = true;

				jblas::vecb is_in_partial(i_max + 1);
				is_in_partial.clear();
				for (size_t i = 0; i < ipartial.size(); i++)
					is_in_partial(ipartial(i)) = true;

				jblas::vecb is_in_comp(i_max + 1);
				for (size_t i = 0; i <= i_max; i++)
					is_in_comp(i) = (is_in_full(i) && !is_in_partial(i));

				jblas::ind_array icomp = ia_bool(is_in_comp);

				return icomp;
			}


			/**
			 * Union of ind. arrays
			 */
			jblas::ind_array ia_union(const jblas::ind_array & ia1, const jblas::ind_array & ia2) {


				// get max index
				size_t max_index = 0;
				for (size_t i = 0; i < ia1.size(); i++)
					if (ia1(i) > max_index)
						max_index = ia1(i);

				for (size_t i = 0; i < ia2.size(); i++)
					if (ia2(i) > max_index)
						max_index = ia2(i);


				// explore inputs
				size_t repetitions = 0;
				jblas::vecb used_indices(max_index + 1);
				used_indices.clear();
				for (size_t i = 0; i < ia1.size(); i++) {
					if (used_indices(ia1(i)))
						repetitions++; // repeated index
					used_indices(ia1(i)) = true;
				}
				for (size_t i = 0; i < ia2.size(); i++) {
					if (used_indices(ia2(i)))
						repetitions++; // repeated index
					used_indices(ia2(i)) = true;
				}

				// write output
				jblas::ind_array icat(ia1.size() + ia2.size() - repetitions);
				size_t j = 0;
				for (size_t i = 0; i <= max_index; i++) {
					if (used_indices(i)) {
						icat(j) = i;
						j++;
					}
				}
				return icat;
			}


			/**
			 * Return the N first indices of an indirect array
			 */
			jblas::ind_array ia_head(const jblas::ind_array & ia, const size_t N) {
				JFR_PRECOND(N <= ia.size(),"indirectArray.hpp: head(): requested size too big.");
				jblas::ind_array res(N);
				for (size_t i = 0; i < N; i++)
					res(i) = ia(i);
				return res;
			}


			/**
			 * Create array from a boolean vector
			 */
			jblas::ind_array ia_bool(const jblas::vecb & vb) {
				int nb = 0;
				for (size_t i = 0; i < vb.size(); i++)
					if (vb(i))
						nb++;
				jblas::ind_array ia(nb);
				int j = 0;
				for (size_t i = 0; i < vb.size(); i++)
					if (vb(i)) {
						ia(j) = i;
						j++;
					}
				return ia;
			}


			/**
			 * Create indirect array from boolean vector and take the head N elements
			 */
			jblas::ind_array ia_head(const jblas::vecb & vb, const size_t N) {
				JFR_PRECOND( (vb.size() >= N), "Boolean vector smaller than requested elements.");
				jblas::ind_array res(N);
				size_t i = 0;
				size_t j = 0;
				for (i = 0; i < vb.size(); i++) {
					if (vb(i)) {
						res(j) = i;
						j++;
						if (j >= N)
							break;
					}
				}
				JFR_POSTCOND( (j == N), "Boolean vector with insufficient true entries.");
				return res;
			}


			/*
			 * Create indirect array from boolean vector and take the head N elements, and clear these N elements form the boolean.
			 */
			jblas::ind_array ia_popfront(jblas::vecb & vb, const size_t N) {
				JFR_PRECOND( (vb.size() >= N), "Boolean vector smaller than requested elements.");
				jblas::ind_array res(N);
				size_t i = 0;
				size_t j = 0;
				for (i = 0; i < vb.size(); i++) {
					if (vb(i)) {
						res(j) = i;
						j++;
						if (j >= N)
							break;
					}
				}
				JFR_POSTCOND( (j == N), "Boolean vector with insufficient true entries.");
				jblas::vecb z(N);
				z.clear();
				ublas::project(vb, res) = z;
				return res;
			}


			/*
			 * Create indirect array from boolean vector with the head N false-elements, and set these N elements to the boolean.
			 */
			jblas::ind_array ia_pushfront(jblas::vecb & vb, const size_t N) {
				JFR_PRECOND( (vb.size() >= N), "Boolean vector smaller than requested elements.");
				jblas::ind_array res(N);
				if (N==0) return res;
				size_t j = 0;
				for (size_t i = 0; i < vb.size(); i++) {
					if (!vb(i)) {
						res(j) = i;
						j++;
						if (j >= N)
							break;
					}
				}
				JFR_POSTCOND( (j == N), "Boolean vector with insufficient true entries.");
				for (size_t i = 0; i < N; i++)
					vb(res(i)) = true;
				return res;
			}


			/*
			 * Create indirect array from range.
			 */
			jblas::ind_array ia_range(const ublas::range & r) {
				jblas::ind_array res(r.size());
				for (size_t i = 0; i < res.size(); i++)
					res(i) = r.start() + i;
				return res;
			}


			/**
			 * Create indirect array from slice.
			 */
			jblas::ind_array ia_slice(const ublas::slice & s) {
				jblas::ind_array res(s.size());
				for (size_t i = 0; i < res.size(); i++)
					res(i) = s.start() + s.stride() * i;
				return res;
			}


			/**
			 * Create indirect array from start and end indices.
			 */
			jblas::ind_array ia_range(size_t begin, size_t end) {
				JFR_PRECOND((begin < end), "Index begin is bigger than end.");
				jblas::ind_array res(end - begin);
				for (size_t i = 0; i < res.size(); i++)
					res(i) = begin + i;
				return res;
			}

		} // namespace ublasExtra
	} // namespace jmath
} // namespace jafar
