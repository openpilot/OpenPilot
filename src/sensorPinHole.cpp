/**
 * sensorPinHole.cpp
 *
 *  Created on: 10/03/2010
 *      Author: jsola
 *
 *  \file sensorPinHole.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/sensorPinHole.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		/*
		 * Constructor from Gaussian pose and params
		 */
		SensorPinHole::SensorPinHole(Gaussian & _pose, const jblas::vec4 & k, const jblas::vec & d, const jblas::vec & c,
		    const size_t hsize, const size_t vsize) :
			SensorAbstract(_pose), parameters(k, d, c), image(hsize, vsize) {
			type("Pin-hole-camera");
		}

		/*
		 * Constructor from mean pose and params
		 */
		SensorPinHole::SensorPinHole(const jblas::vec & _pose, const jblas::vec4 & k, const jblas::vec & d,
		    const jblas::vec & c, const size_t hsize, const size_t vsize) :
			SensorAbstract(_pose), parameters(k, d, c), image(hsize, vsize) {
			type("Pin-hole-camera");
		}

		/*
		 * Construction from map, indirect array and params
		 */
		SensorPinHole::SensorPinHole(MapAbstract & map, const jblas::ind_array & ias, const jblas::vec4 & k,
		    const jblas::vec & d, const jblas::vec & c, const size_t hsize, const size_t vsize) :
			SensorAbstract(map, ias), parameters(k, d, c), image(hsize, vsize) {
			type("Pin-hole-camera");
		}

	}
}

