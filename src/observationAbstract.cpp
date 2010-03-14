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

namespace jafar {
	namespace rtslam {
		using namespace std;

		/////////////////////////////////
		// EXPECTATION
		/////////////////////////////////

		/*
		 * Size constructor
		 */
		Expectation::Expectation(const size_t _size) :
			Gaussian(_size) {
		}

		/*
		 * sizes constructor
		 */
		Expectation::Expectation(const size_t _size, const size_t _size_nonobs) :
			Gaussian(_size), nonObs(_size_nonobs) {
		}

		///////////////////////////
		// INNOVATION
		///////////////////////////

		/*
		 * Size constructor
		 */
		Innovation::Innovation(const size_t _size) :
			Gaussian(_size), iP_(_size) {
		}

		/*
		 * Sizes construction.
		 */
		Innovation::Innovation(const size_t _size, const size_t _size_meas, const size_t _size_exp) :
			Gaussian(_size), iP_(_size), INN_meas(_size, _size_meas), INN_exp(_size, _size_exp) {
		}

		/////////////////////////
		// MEASUREMENT
		/////////////////////////

		/*
		 * Size constructor
		 */
		Measurement::Measurement(size_t _size) :
			Gaussian(_size) {
		}

		//////////////////////////
		// OBSERVATION ABSTRACT
		//////////////////////////

		/*
		 * Destructor
		 */
		ObservationAbstract::~ObservationAbstract(void) {
		}

		/*
		 * Size constructor
		 */
		ObservationAbstract::ObservationAbstract(size_t _size) :
			expectation(_size), measurement(_size), innovation(_size) {
		}

		/*
		 * Sizes constructor
		 */
		ObservationAbstract::ObservationAbstract(size_t _size_meas, size_t _size_exp, size_t _size_inn) :
			expectation(_size_exp), measurement(_size_meas), innovation(_size_inn) {
		}

		/*
		 * Associate to sensor and landmark.
		 * This sets several parameters such as identifiers and pointers to sensor and landmark ancestors.
		 * \param sen the sensor
		 * \param lmk the landmark
		 */
		inline void ObservationAbstract::associate(SensorAbstract & sen, LandmarkAbstract & lmk) {
			sensor = &sen;
			landmark = &lmk;
		}

	//		/*
	//		 * Sensor and landmark constructor
	//		 */
	//		ObservationAbstract::ObservationAbstract(SensorAbstract & _sen, LandmarkAbstract & _lmk):
	//			expectation(){}

	} // namespace rtslam
} // namespace jafar
