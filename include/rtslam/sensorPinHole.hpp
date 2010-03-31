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

#include "rtslam/sensorAbstract.hpp"
#include "rtslam/gaussian.hpp"
#include "iostream"

namespace jafar {
	namespace rtslam {

		/**
		 * Class for pin-hole parameters.
		 */
		class ParamsPinHole : public ParametersAbstract {
			public:
			vec4 intrinsic; ///< intrinsic vector [u0, v0, au, av]
			vec distortion; ///< radial distortion [d2, d4, ...] (model: ud = (1 + d2 r^2 + d2 r^4 + ...) * up)
			vec correction; ///< radial correction [c2, c4, ...] (See Joan Sola's thesis for correction model and calibration)
			ParamsPinHole(const vec4 & k, const vec & d, const vec & c);
		};


		/**
		 * Class for pin-hole cameras.
		 * This model accepts radial distortion model
		 * \ingroup rtslam
		 */
		class SensorPinHole: public SensorAbstract {

			public:

				/**
				 * Empty constructor
				 */
				SensorPinHole();

				/**
				 * Constructor from Gaussian pose.
				 * \param _pose the Gaussian pose.
				 */
				SensorPinHole(const Gaussian & _pose);

				/**
				 * Constructor from mean pose.
				 * \param _pose the pose 7-vector.
				 */
				SensorPinHole(const jblas::vec7 & _pose);

				/**
				 * Construction from map.
				 * \param _map the map.
				 */
				SensorPinHole(MapAbstract & _map);

				/**
				 * Constructor for selectable LOCAL or REMOTE pose, from robot and selector flag.
				 * \param _rob the robot to install to.
				 * \param inFilter flag indicating in the sensor pose is filtered or not.
				 */
				SensorPinHole(RobotAbstract & _rob, bool inFilter = false);

				/**
				 * Pin-hole sensor setup.
				 * \param k the vector of intrinsic parameters <c>k = [u_0, v_0, a_u, a_v]</c>.
				 * \param d the radial distortion parameters vector <c>d = [d_2, d_4, ...] </c>.
				 * \param c the radial distortion correction parameters vector <c>c = [c_2, c_4, ...] </c>.
				 * \param hsize the horizontal image size.
				 * \param vsize the vertical image size.
				 */
				void set_parameters(const jblas::vec4 & k, const jblas::vec & d, const jblas::vec & c, const size_t hsize,
				    const size_t vsize);

				static size_t size(void) {
					return 7;
				}

				ParamsPinHole* params;
			private:
				jblas::vec4 intrinsic;
				jblas::vec distortion;
				jblas::vec correction;
		};

	}
}

#endif /* SENSORPINHOLE_HPP_ */
