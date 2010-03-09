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

				size_t ifull_max = 0;
				for (size_t i = 0; i < ifull.size(); i++)
					if (ifull(i) > ifull_max)
						ifull_max = ifull(i);

				jblas::vecb is_in_partial(ifull_max + 1);

				is_in_partial.clear();
				for (size_t i = 0; i < ipartial.size(); i++)
					is_in_partial(ipartial(i)) = true;

				jblas::ind_array icomp(ifull.size() - ipartial.size());
				size_t j = 0;
				for (size_t i = 0; i < ifull.size(); i++) {
					if (!is_in_partial(ifull(i))) {
						icomp(j) = ifull(i);
						j++;
					}
				}
				cout << endl;
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
			jblas::ind_array ia_bool(jblas::vecb & vb) {

				int nb = 0;
				for (size_t i = 0; i < vb.size(); i++) {
					if (vb(i))
						nb += 1;
				}

				jblas::ind_array ia(nb);

				int j = 0;

				for (size_t i = 0; i < vb.size(); i++) {
					if (vb(i)) {
						ia(j) = i;
						j += 1;
					}
				}

				return ia;
			}

		}
	}
}
