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
			vec2 up;
			up(0) = v(0) / v(2);
			up(1) = v(1) / v(2);
			return up;
		}

		// Pin-hole canonical projection with jacobian
		void ObservationPinHolePoint::project0(const jblas::vec3 & v, vec2 & up,
		    mat & UP_v) {

			up = project0(v);

			UP_v(0, 0) = 1 / v(2);
			UP_v(0, 1) = 0;
			UP_v(0, 2) = - v(0) / (v(2) * v(2));
			UP_v(1, 0) = 0;
			UP_v(1, 1) = 1 / v(2);
			UP_v(1, 2) = - v(1) / (v(2) * v(2));

		}

		// Radial distortion: ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...) * u
		jblas::vec2 ObservationPinHolePoint::distort(const jblas::vec & d, const jblas::vec2 & up) {
			size_t n = d.size();
			if (n == 0)
				return up;
			else {
				double r2 = up(0) * up(0) + up(1) * up(1); // this is the norm squared: r2 = ||u||^2
				double s = 1.0;
				double r2i = 1.0;
				for (size_t i = 0; i < n; i++) { //.. here we are doing:
					r2i = r2i * r2; //................. r2i = r^(2*(i+1))
					s += d(i) * r2i; //................ s = 1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...
				}
				return s * up; // finally ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...) * u;
			}
		}

		// Radial distortion: ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...) * u with jacobians
		void ObservationPinHolePoint::distort(const jblas::vec & d, const jblas::vec2 & up, vec2 & ud, mat & UD_up) {
			size_t n = d.size();
			jblas::vec2 R2_up;
			jblas::vec2 S_up;

			if (n == 0) {
				ud = up;
				UD_up = jblas::identity_mat(2);
			}

			else {
				double r2 = up(0) * up(0) + up(1) * up(1); // this is the norm squared: r2 = ||u||^2
				double s = 1.0;
				double r2i = 1.0;
				double r2im1 = 1.0; //r2*(i-1)
				double S_r2 = 0.0;

				for (size_t i = 0; i < n; i++) { //.. here we are doing:
					r2i = r2i * r2; //................. r2i = r^(2*(i+1))
					s += d(i) * r2i; //................ s = 1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...

					S_r2 = S_r2 + (i+1) * d(i) * r2im1; //jacobian of s wrt r2 : S_r2 = d_0 + 2 * d1 * r^2 + 3 * d_2 * r^4 +  ...
					r2im1 = r2im1 * r2;
				}

				ud = s * up; // finally ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...) * u;

				R2_up(0) = 2 * up(0);
				R2_up(1) = 2 * up(1);

				S_up(0) = R2_up(0) * S_r2;
				S_up(1) = R2_up(1) * S_r2;

				UD_up(0,0) = S_up(0) * up(0) + s;
				UD_up(0,1) = S_up(1) * up(0);
				UD_up(1,0) = S_up(0) * up(1);
				UD_up(1,1) = S_up(1) * up(1) + s;
			}

		}

		// Pixellization from k = [u_0, v_0, a_u, a_v]
		jblas::vec2 ObservationPinHolePoint::pixellize(const jblas::vec4 & k,
		    const jblas::vec2 & ud) {
			double u_0 = k(0);
			double v_0 = k(1);
			double a_u = k(2);
			double a_v = k(3);
			vec2 u;
			u(0) = u_0 + a_u * ud(0);
			u(1) = v_0 + a_v * ud(1);
			return u;
		}

		// Pixellization from k = [u_0, v_0, a_u, a_v] with jacobians
		void ObservationPinHolePoint::pixellize(const jblas::vec4 & k,
		    const jblas::vec2 & ud, jblas::vec2 & u, jblas::mat & U_ud) {
			//double u_0 = k(0);
			//double v_0 = k(1);
			double a_u = k(2);
			double a_v = k(3);

			u = pixellize(k, ud);

			U_ud(0, 0) = a_u;
			U_ud(0, 1) = 0;
			U_ud(1, 0) = 0;
			U_ud(1, 1) = a_v;

		}

		vec2 ObservationPinHolePoint::project(const vec4 & k, const vec & d,
		    const vec3 v) {
			return pixellize(k, distort(d, project0(v)));
		}

		void ObservationPinHolePoint::project(const vec4 & k, const vec & d,
		    const vec3 v, vec2 & u, mat & U_v) {

			vec2 up;
			vec2 ud;

			mat UP_v(2,3);
			mat UD_up(2,2);
			mat U_ud(2,2);

			project0(v, up, UP_v);
			distort(d, up, ud, UD_up);
			pixellize(k, ud, u, U_ud);

			mat U_v1(2,3);

			U_v1 = ublas::prod(UD_up, UP_v);
			U_v = ublas::prod(U_ud, U_v1);

			//U_v = ublas::prod(U_ud, ublas::prod<mat>(UD_up, UP_v));

		}

	}
}
