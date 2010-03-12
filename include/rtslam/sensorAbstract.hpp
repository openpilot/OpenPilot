/*
 * sensorAbstract.h
 *
 *  Created on: Jan 28, 2010
 *      Author: jsola
 */

/**
 * \file sensorAbstract.hpp
 * File defining the abstract sensor class
 * \ingroup rtslam
 */

#ifndef SENSORABSTRACT_H_
#define SENSORABSTRACT_H_

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <list>

#include "jmath/jblas.hpp"
#include "rtslam/blocks.hpp"
//include parents
#include "rtslam/robotAbstract.hpp"
//#include "rtslam/mapAbstract.hpp"

namespace jafar {
	namespace rtslam {

		using namespace std;

		// Forward declarations of children
		class ObservationAbstract;

		/**
		 * Base class for all raw data in module rtslam.
		 * \ingroup rtslam
		 */
		class RawAbstract {
			public:
				/**
				 * Mandatory virtual destructor.
				 */
				inline virtual ~RawAbstract(void) {
				}
				/**
				 * Acquire raw data
				 */
				inline virtual void acquire(void) {
				}
		};

		/**
		 * Base class for all sensors defined in the module rtslam.
		 * \ingroup rtslam
		 */
		class SensorAbstract {

			public:

				// Mandatory virtual destructor.
				inline virtual ~SensorAbstract(void) {
				}


				RobotAbstract* robot;
				std::list<ObservationAbstract*> observationsList;

				Gaussian pose;

				/**
				 * Local pose constructor - only mean
				 * Creates a sensor with its own pose information
				 * \param _pose a pose vector
				 */
				//				template<class V>
				SensorAbstract(const jblas::vec & _pose);

				/**
				 * Local pose constructor - full Gaussian.
				 * Creates a sensor with its own pose information.
				 * \param _pose a Gaussian pose
				 */
				SensorAbstract(const Gaussian & _pose);

				/**
				 * Remote pose constructor.
				 * Creates a sensor with the pose indexed in a map.
				 * \param map the map
				 * \param ias the indirect array of indices to the map
				 */
				SensorAbstract(MapAbstract & map, const jblas::ind_array & ias);

				inline void setup(size_t _id, std::string & _name, std::string & _type) {
					id_ = _id;
					name_ = _name;
					type_ = _type;
				}

				inline void id(size_t _id) {
					id_ = _id;
				}
				inline void type(std::string _type) {
					type_ = _type;
				}
				inline void name(std::string _name) {
					name_ = _name;
				}
				inline size_t id(void) {
					return id_;
				}
				inline std::string type(void) {
					return type_;
				}
				inline std::string name(void) {
					return name_;
				}


				/**
				 * Install sensor in robot
				 * \param sen the sensor
				 */
				void installToRobot(RobotAbstract & rob);


				/*
				 * Acquire raw data
				 */
				virtual void acquireRaw() {
					// raw.acquire();
				}

				//				/*
				//				 * Project all landmarks
				//				 */
				//				void projectAllLandmarks(void);
				//
				//				/*
				//				 * Select most informative observations to update
				//				 */
				//				std::list<size_t> selectMostInformative(size_t numOfLmks);
				//
				//				/*
				//				 * Try to match landmarks
				//				 */
				//				virtual std::list<size_t> match(std::list<size_t>) = 0;

				/**
				 * Operator << for class SensorAbstract.
				 * It shows information of the sensor.
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::SensorAbstract & sen) {
					s << "SENSOR " << sen.id() << ": " << sen.name() << " of type " << sen.type() << endl;
					s << ".pose:  " << sen.pose << endl;
					return s;
				}

			private:
				std::size_t id_;
				std::string name_;
				std::string type_;


		};

	}
}

#endif /* SENSORABSTRACT_H_ */
