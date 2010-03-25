/**
 * observationAbstract.cpp
 *
 *  Created on: 10/03/2010
 *      Author: jsola
 *
 *  \file observationAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/observationAbstract.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/landmarkAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;



		//////////////////////////
		// OBSERVATION ABSTRACT
		//////////////////////////

		/*
		 * Operator << for class ObservationAbstract.
		 * It shows different information of the observation.
		 */
		std::ostream& operator <<(std::ostream & s, jafar::rtslam::ObservationAbstract & obs) {
			s << "OBSERVATION " << obs.id() << ": of " << obs.landmark->type() << " from " << obs.sensor->type() << endl;
			s << "Sensor: " << obs.sensor->id() << ", landmark: " << obs.landmark->id() << endl;
			s << ".expectation:  " << obs.expectation << endl;
			s << ".measurement:  " << obs.measurement << endl;
			s << ".innovation:   " << obs.innovation;
			return s;
		}


		/*
		 * Size constructor
		 */
		ObservationAbstract::ObservationAbstract(size_t _size) :
			expectation(_size), measurement(_size), innovation(_size) {
			categoryName("OBSERVATION");
		}

		/*
		 * Sizes constructor
		 */
		ObservationAbstract::ObservationAbstract(size_t _size_meas, size_t _size_exp, size_t _size_inn) :
			expectation(_size_exp), measurement(_size_meas), innovation(_size_inn) {
			categoryName("OBSERVATION");
		}

		/*
		 * Associate to sensor and landmark.
		 * This sets several parameters such as identifiers and pointers to sensor and landmark ancestors.
		 * \param sen the sensor
		 * \param lmk the landmark
		 */
		inline void ObservationAbstract::associate(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr) {
			sensor = senPtr;
			landmark = lmkPtr;
		}

		void ObservationAbstract::linkToSensor(sensor_ptr_t _sensorPtr){
			sensor = _sensorPtr;
		}

		void ObservationAbstract::linkToLandmark(landmark_ptr_t _lmkPtr){
			landmark = _lmkPtr;
		}


	} // namespace rtslam
} // namespace jafar
