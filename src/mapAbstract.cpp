/**
 * mapAbstract.cpp
 *
 *  Created on: 10/03/2010
 *      Author: jsola
 *
 *  \file mapAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/mapAbstract.hpp"

using namespace std;

namespace jafar {
	namespace rtslam {

		/**
		 * Constructor
		 */
		MapAbstract::MapAbstract(size_t _max_size) :
			max_size(_max_size), current_size(0), used_states(max_size), filter(max_size) {
			used_states.clear();
		}

		jblas::ind_array MapAbstract::getFreeSpace(size_t N) {
			size_t available_space = max_size - current_size;
			if (N <= available_space) {
				jblas::ind_array res = jmath::ublasExtra::ia_popfront(used_states, N);
				current_size += N;
				return res;
			}
			else {
				jblas::ind_array res(0);
				return res;
			}
		}

		/**
		 * Liberate the space indicated.
		 * \param _ia the space to liberate.
		 */
		void MapAbstract::liberateSpace(const jblas::ind_array & _ia) {
			for (size_t i = 0; _ia.size(); i++)
				used_states(_ia(i)) = false;
			current_size += _ia.size();
		}

	}
}
