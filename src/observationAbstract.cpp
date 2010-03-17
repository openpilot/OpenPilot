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

		bool Expectation::isVisible() {
			return visible_;
		} // landmark is visible (in Field Of View).

		double Expectation::infoGain() {
			return infoGain_;
		}	 // expected "information gain" of performing an update with this observation.

		void Expectation::appearance(appearance_t _appearance) {
			appearance_ = _appearance;
		}


		///////////////////////////
		// INNOVATION
		///////////////////////////

		/*
		 * Size constructor
		 */
		Innovation::Innovation(const size_t _size) :
			Gaussian(_size), iP_(_size), INN_meas(_size, _size), INN_exp(_size, _size) {
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
			Gaussian(_size), score(0) {
		}

		void Measurement::appearance(appearance_t _appearance) {
			appearance_ = _appearance;
		}

//		Measurement::appearance_t Measurement::appearance();


		//////////////////////////
		// OBSERVATION ABSTRACT
		//////////////////////////

		/*
		 * Destructor
		 */
		ObservationAbstract::~ObservationAbstract() {
			categoryName("OBSERVATION");
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
		inline void ObservationAbstract::associate(SensorAbstract & sen, LandmarkAbstract & lmk) {
			sensor = &sen;
			landmark = &lmk;
		}

	} // namespace rtslam
} // namespace jafar
