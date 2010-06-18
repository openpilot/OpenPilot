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

#include "image/Image.hpp"
#include "rtslam/rtSlam.hpp"

#include "rtslam/sensorAbstract.hpp"
#include "rtslam/gaussian.hpp"
#include "rtslam/sensorImageParameters.hpp"
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

				~SensorPinHole(){}

				SensorImageParameters params;

				void setup(const size_t id, const string & name, const int _width, const int _height, const vec7 & pose, const vec7 & std, const jblas::vec2 & _s, const vec4 & k, const vec & d, const vec & c);

				virtual std::string typeName() {
					return "Pin-hole-camera";
				}

				static size_t size(void) {
					return 7;
				}

				void acquireRaw();
				void releaseRaw();

				raw_ptr_t getRaw() ;

		};

	}
}

#endif /* SENSORPINHOLE_HPP_ */
