/**
 * \file sensorPinHole.hpp
 *
 * Header file for pin-hole sensor.
 *
 * \author jsola@laas.fr
 * \date 14/02/2010
 *
 * \ingroup rtslam
 */

#ifndef SENSORPINHOLE_HPP_
#define SENSORPINHOLE_HPP_

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
				 * \param _robPtr the robot to install to.
				 * \param inFilter flag indicating in the sensor pose is filtered or not.
				 */
				SensorPinHole(const robot_ptr_t & _robPtr, filtered_obj_t inFilter = UNFILTERED);

				/**
				 * Selectable LOCAL or REMOTE pose constructor.
				 * Creates a pin-hole sensor with the pose indexed in a map.
				 * \param dummy a marker for simulation. Give value ObjectAbstract::FOR_SIMULATION.
				 * \param _robPtr the robot
				 */
				SensorPinHole(const simulation_t dummy, const robot_ptr_t & _robPtr);

				jblas::vec2 imgSize;
				jblas::vec4 intrinsic;
				jblas::vec distortion;
				jblas::vec correction;

				/**
				 * Pin-hole sensor setup.
				 * \param k the vector of intrinsic parameters <c>k = [u_0, v_0, a_u, a_v]</c>.
				 * \param d the radial distortion parameters vector <c>d = [d_2, d_4, ...] </c>.
				 * \param c the radial distortion correction parameters vector <c>c = [c_2, c_4, ...] </c>.
				 * \param hsize the horizontal image size.
				 * \param vsize the vertical image size.
				 */
				void set_parameters(const jblas::vec4 & k, const jblas::vec & d, const jblas::vec & c);

				/**
				 * Pin-hole sensor setup
				 */
				void setup(const size_t id, const string & name, const vec7 & pose, const vec7 & std, const vec4 & k, const vec & d, const vec & c);


				static size_t size(void) {
					return 7;
				}

				//				/**
//				 * Back-project pixel, with Jacobians.
//				 * \param u the pixel
//				 * \param s the point's depth
//				 * \param p the back-projected 3D point
//				 * \param P_u the Jacobian of p wrt u
//				 * \param P_s the Jacobian of p wrt s
//				 */
//				void backProjectPoint(const vec2 & u, const double s, vec3 & p, mat32 & P_u, mat & P_s){};
//
//				/**
//				 * Back-project pixel onto vector.
//				 * \param u the pixel
//				 * \return the back-projected 3D vector
//				 */
//				vec3 backProjectVector(const vec2 & u){};

//				/**
//				 * Project a point into a pin-hole camera with radial distortion
//				 * \param v the 3D point to project, or the 3D director vector
//				 * \return the projected and distorted point
//				 */
//				vec2 projectPoint(const vec3 & v);
//
//				/**
//				 * Project a point into a pin-hole camera with radial distortion
//				 * \param v the 3D point to project, or the 3D director vector
//				 * \param u the projected and distorted point
//				 * \param U_v the Jacobian of \a u wrt \a v
//				 */
//				void projectPoint(const vec3 & v, vec2 & u, mat23 & U_v);
//
//				/**
//				 * Back-project pixel.
//				 * \param u the pixel
//				 * \param s the point's depth
//				 * \return the back-projected 3D point
//				 */
//				vec3 backProjectPoint(const vec2 & u, const double s = 1.0) {}
//
//				/**
//				 * Back-project pixel, with Jacobians.
//				 * \param u the pixel
//				 * \param s the point's depth
//				 * \param p the back-projected 3D point
//				 * \param P_u the Jacobian of p wrt u
//				 * \param P_s the Jacobian of p wrt s
//				 */
//				void backProjectPoint(const vec2 & u, const double s, vec3 & p, mat32 & P_u, mat & P_s) {}
//
//				/**
//				 * Back-project pixel onto vector.
//				 * \param u the pixel
//				 * \return the back-projected 3D vector
//				 */
//				vec3 backProjectVector(const vec2 & u);
//
//				/**
//				 * Back-project pixel onto vector, with Jacobians.
//				 * \param u the pixel
//				 * \param v the back-projected 3D vector
//				 * \param V_u the Jacobian of v wrt u
//				 */
//				void backProjectVector(const vec2 & u, vec3 & v, mat32 & V_u);


		};

	}
}

#endif /* SENSORPINHOLE_HPP_ */
