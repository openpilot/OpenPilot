/**
 * sensorPinHole.cpp
 *
 * \date 10/03/2010
 *      Author: jsola
 *
 *  \file sensorPinHole.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/pinhole.hpp"

#include "rtslam/sensorPinHole.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		///////////////////////////////////////
		// Class sensor pin hole
		///////////////////////////////////////

		SensorPinHole::SensorPinHole(const robot_ptr_t & _robPtr, bool inFilter) :
			SensorAbstract(_robPtr, inFilter){
			type("Pin-hole-camera");
		}

		void SensorPinHole::set_parameters(const jblas::vec4 & k, const jblas::vec & d, const jblas::vec & c) {
			intrinsic = k;
			distortion.resize(d.size());
			distortion = d;
			correction.resize(c.size());
			correction = c;
		}

		vec2 SensorPinHole::projectPoint(const vec3 & v){
			return pinhole::projectPoint(intrinsic, distortion, v);
		}

		void SensorPinHole::projectPoint(const vec3 & v, vec2 & u, mat23 & U_v){
			pinhole::projectPoint(intrinsic, distortion, v, u, U_v);
		}

	}
}

