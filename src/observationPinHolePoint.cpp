/**
 * \file observationPinHolePoint.cpp
 *
 *  Created on: 21/03/2010
 *     \author: jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/observationPinHolePoint.hpp"
#include "jmath/jblas.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jblas;


		// Pin-hole canonical projection
		vec2 ObservationPinHolePoint::project0(const jblas::vec3 & v) {
			vec2 u;
			u(0) = v(0) / v(2);
			u(1) = v(1) / v(2);
			return u;
		}


		// Radial distortion: ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...) * u
		jblas::vec2 ObservationPinHolePoint::distort(const jblas::vec & d, const jblas::vec2 & u) {
			size_t n = d.size();
			if (n == 0)
				return u;
			else {
				double r2 = u(0) * u(0) + u(1) * u(1); // this is the norm squared: r2 = ||u||^2
				double s = 1.0;
				double r2i = 1.0;
				for (size_t i = 0; i < n; i++) { //.. here we are doing:
					r2i = r2i * r2; //................. r2i = r^(2*(i+1))
					s += d(i) * r2i; //................ s = 1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...
				}
				return s * u; // finally ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...) * u;
			}
		}


		// Pixellization from k = [u_0, v_0, a_u, a_v]
		jblas::vec2 ObservationPinHolePoint::pixellize(const jblas::vec4 & k, const jblas::vec2 & ud) {
			double u_0 = k(0);
			double v_0 = k(1);
			double a_u = k(2);
			double a_v = k(3);
			vec2 u;
			u(0) = u_0 + a_u * ud(0) / ud(2);
			u(1) = v_0 + a_v * ud(1) / ud(2);
			return u;
		}

		vec2 ObservationPinHolePoint::project(const vec4 & k, const vec & d, const vec3 v) {
			return pixellize(k, distort(d, project0(v)));
		}

	}
}
