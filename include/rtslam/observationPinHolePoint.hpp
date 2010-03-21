/**
 * observationPinHolePoint.hpp
 *
 *  Created on: 14/03/2010
 *      Author: jsola
 *
 *  \file observationPinHolePoint.hpp
 *
 *  ## Add a description here ##
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


				/**
				 * Project a director vector.
				 * \param p a 3D director vector.
				 * \param U_r the Jacobian wrt the robot pose
				 * \param U_s the Jacobian wrt the sensor pose
				 * \param U_p the Jacobian wrt the point
				 */
				void projectDir(const jblas::vec3 & p, jblas::mat & U_r, jblas::mat & U_s, jblas::mat & U_p);
				vec2 projectDir(const vec4 & k, const vec & d, const vec3 v);

			private:
				/**
				 * Pin hole normalized projection for 3D points.
				 * \param p the point to project
				 * \return the projected point in the normalized image plane (at focal distance \e f = 1).
				 */
				jblas::vec2 project0(const jblas::vec3 & p);

				/**
				 * Radial distortion.
				 * This follows the model
				 * - ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + etc) * u
				 *
				 * with r = norm(u)^2.
				 * \param d the radial distortion parameters d = [d_0, d_1, ...]
				 * \param u the undistorted pixel
				 * \return the distorted pixel
				 */
				jblas::vec2 distort(const jblas::vec & d, const jblas::vec & u);

				/**
				 * Rectangular pixellization.
				 * Transforms a projected point in metric coordinates into a pixel value
				 * \param k the intrinsic parameters vector, \a k = [u_0, v_0, a_u, a_v]
				 * \param ud the projected point
				 * \return the pixel value
				 */
				jblas::vec2 pixellize(const jblas::vec4 & k, const jblas::vec4 & ud);
				void project0(const jblas::vec3 & v, jblas::vec2 & u, jblas::mat & U_v);
				void distort(const jblas::vec & d, const jblas::vec & u, jblas::vec2 & ud, jblas::mat & UD_u);
				void pixellize(const jblas::vec4 & k, const jblas::vec4 & ud, jblas::vec2 & u, jblas::mat & U_ud);

		};

	}
}

#endif /* OBSERVATIONPINHOLEPOINT_HPP_ */
