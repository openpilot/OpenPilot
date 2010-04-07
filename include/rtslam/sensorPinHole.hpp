/**
 * \file sensorPinHole.hpp
 *
 * Header file for pin-hole sensor.
 *
 *  Created on: 14/02/2010
 *      \author: jsola
 *
 * \ingroup rtslam
 */

#ifndef SENSORPINHOLE_HPP_
#define SENSORPINHOLE_HPP_

//#include "image/Image.hpp"
#include "rtslam/rtSlam.hpp"

#include "rtslam/sensorAbstract.hpp"
#include "rtslam/gaussian.hpp"
#include "iostream"

namespace jafar {
	namespace rtslam {

		class SensorPinHole;

		typedef boost::shared_ptr<SensorPinHole> pinhole_ptr_t;


		/**
		 * Class for pin-hole cameras.
		 * This model accepts radial distortion model
		 * \ingroup rtslam
		 */
		class SensorPinHole: public SensorAbstract {

			public:

				/**
				 * Constructor for selectable LOCAL or REMOTE pose, from robot and selector flag.
				 * \param _rob the robot to install to.
				 * \param inFilter flag indicating in the sensor pose is filtered or not.
				 */
				SensorPinHole(const robot_ptr_t & _robPtr, bool inFilter = false);

				/**
				 * Pin-hole sensor setup.
				 * \param k the vector of intrinsic parameters <c>k = [u_0, v_0, a_u, a_v]</c>.
				 * \param d the radial distortion parameters vector <c>d = [d_2, d_4, ...] </c>.
				 * \param c the radial distortion correction parameters vector <c>c = [c_2, c_4, ...] </c>.
				 * \param hsize the horizontal image size.
				 * \param vsize the vertical image size.
				 */
				void set_parameters(const jblas::vec4 & k, const jblas::vec & d, const jblas::vec & c);

				static size_t size(void) {
					return 7;
				}

				static vec2 projectPoint(const vec4 & k, const vec & d, const vec3 & v);
				static void projectPoint(const vec4 & k, const vec & d, const vec3 & v, vec2 & u, mat23 & U_v);
				vec2 projectPoint(const vec3 & v);
				void projectPoint(const vec3 & v, vec2 & u, mat23 & U_v);

				/**
				 * Pin hole normalized projection for 3D points.
				 * \param p the point to project
				 * \return the projected point in the normalized image plane (at focal distance \e f = 1).
				 */
				static jblas::vec2 projectPointToNormalizedPlane(const jblas::vec3 & p);

				/**
				 * Pin hole normalized projection for 3D points, with Jacobian.
				 * \param v the point to project
				 * \param up the projected point in the normalized image plane (at focal distance \e f = 1).
				 * \param UP_v the Jacobian of \a up wrt \a v.
				 */
				static void projectPointToNormalizedPlane(const jblas::vec3 & v, vec2 & up, mat23 & UP_v);

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
				static jblas::vec2 distortPoint(const jblas::vec & d, const jblas::vec2 & up);

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
				static void distortPoint(const jblas::vec & d, const jblas::vec2 & up, vec2 & ud, mat22 & UD_up);

				/**
				 * Rectangular pixellization.
				 * Transforms a projected point in metric coordinates into a pixel value
				 * \param k the intrinsic parameters vector, \a k = [u_0, v_0, a_u, a_v]
				 * \param ud the projected point
				 * \return the pixel value
				 */
				static jblas::vec2 pixellizePoint(const jblas::vec4 & k, const jblas::vec2 & ud);

				/**
				 * Rectangular pixellization.
				 * Transforms a projected point in metric coordinates into a pixel value
				 * \param k the intrinsic parameters vector, \a k = [u_0, v_0, a_u, a_v]
				 * \param ud the projected point
				 * \param u the pixel value
				 * \param U_ud the Jacobian of \a u wrt \a ud
				 */
				static void pixellizePoint(const jblas::vec4 & k, const jblas::vec2 & ud, jblas::vec2 & u, jblas::mat22 & U_ud);

//			private:
				jblas::vec4 intrinsic;
				jblas::vec distortion;
				jblas::vec correction;

		};

	}
}

#endif /* SENSORPINHOLE_HPP_ */
