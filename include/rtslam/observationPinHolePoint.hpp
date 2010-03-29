/**
 *
 *  \file observationPinHolePoint.hpp
 *
 *  Header file for observations of points from pin-hole camera sensors.
 *
 *  Created on: 14/03/2010
 *     \author: jsola@laas.fr
 *
 * \ingroup rtslam
 */

#ifndef OBSERVATIONPINHOLEPOINT_HPP_
#define OBSERVATIONPINHOLEPOINT_HPP_

#include "rtslam/observationAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jblas;


		/**
		 * Class of observations of points from pin-hole cameras
		 * \author jsola
		 * \ingroup rtslam
		 */
		class ObservationPinHolePoint: public ObservationAbstract {

			public:


				/**
				 * Director vector.
				 * This is v = R'(m - (T-p0)*rho), the director vector in sensor frame.
				 */
				jblas::vec3 dirVec;

				/**
				 * Constructor
				 */
				ObservationPinHolePoint() :
					ObservationAbstract(2) {
				}

				// TODO implement this
				void project_func(){
				}

				/**
				 * Project a director vector.
				 * \param k the intrinsic parameters vector.
				 * \param d the radial distortion parameters vector.
				 * \param v the vector to project, in sensor frame.
				 * \return the projected pixel.
				 */
				vec2 project_func(const vec4 & k, const vec & d, const vec3 v);

				/**
				 * Project a director vector, return Jacobians.
				 * \param k the intrinsic parameters vector.
				 * \param d the radial distortion parameters vector.
				 * \param v the vector to project, in sensor frame.
				 * \param u the projected pixel.
				 * \param U_v the Jacobian of \a u wrt \a v.
				 */
				void project_func(const vec4 & k, const vec & d, const vec3 v, vec2 & u, mat & U_v);

			private:
				/**
				 * Pin hole normalized projection for 3D points.
				 * \param p the point to project
				 * \return the projected point in the normalized image plane (at focal distance \e f = 1).
				 */
				jblas::vec2 project0(const jblas::vec3 & p);

				/**
				 * Pin hole normalized projection for 3D points, with Jacobian.
				 * \param v the point to project
				 * \param up the projected point in the normalized image plane (at focal distance \e f = 1).
				 * \param UP_v the Jacobian of \a up wrt \a v.
				 */
				void project0(const jblas::vec3 & v, vec2 & up, mat & UP_v);

				/**
				 * Radial distortion.
				 * This follows the model
				 * - ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + etc) * u
				 *
				 * with r = norm(u)^2.
				 * \param d the radial distortion parameters d = [d_0, d_1, ...]
				 * \param up the undistorted pixel
				 * \return the distorted pixel
				 */
				jblas::vec2 distort(const jblas::vec & d, const jblas::vec2 & up);

				/**
				 * Radial distortion.
				 * This follows the model
				 * - ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + etc) * u
				 *
				 * with r = norm(u)^2.
				 * \param d the radial distortion parameters d = [d_0, d_1, ...]
				 * \param up the undistorted pixel
				 * \param ud the distorted pixel
				 * \param UD_up the Jacobian of \a ud wrt \a up
				 */
				void distort(const jblas::vec & d, const jblas::vec2 & up, vec2 & ud, mat & UD_up);

				/**
				 * Rectangular pixellization.
				 * Transforms a projected point in metric coordinates into a pixel value
				 * \param k the intrinsic parameters vector, \a k = [u_0, v_0, a_u, a_v]
				 * \param ud the projected point
				 * \return the pixel value
				 */
				jblas::vec2 pixellize(const jblas::vec4 & k, const jblas::vec2 & ud);

				/**
				 * Rectangular pixellization.
				 * Transforms a projected point in metric coordinates into a pixel value
				 * \param k the intrinsic parameters vector, \a k = [u_0, v_0, a_u, a_v]
				 * \param ud the projected point
				 * \param u the pixel value
				 * \param U_ud the Jacobian of \a u wrt \a ud
				 */
				void pixellize(const jblas::vec4 & k, const jblas::vec2 & ud, jblas::vec2 & u, jblas::mat & U_ud);

		};

	}
}

#endif /* OBSERVATIONPINHOLEPOINT_HPP_ */
