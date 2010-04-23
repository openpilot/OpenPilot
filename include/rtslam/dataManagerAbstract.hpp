/*
 * dataManagerAbstract.hpp
 *
 *  Created on: Apr 22, 2010
 *      Author: nmansard
 */

#ifndef DATAMANAGERABSTRACT_HPP_
#define DATAMANAGERABSTRACT_HPP_

#include "rtslam/parents.hpp"
#include "rtslam/sensorAbstract.hpp"

namespace jafar {
	namespace rtslam {

		class ObservationAbstract;

		class DataManagerAbstract: public WeakParentOf<ObservationAbstract>, public ChildOf<SensorAbstract> {
			ENABLE_ACCESS_TO_WEAK_CHILDREN(ObservationAbstract,Observation,observation)
				;

			virtual ~DataManagerAbstract( void ) { }

		};

	}
}//namespace jafar::rtslam
#endif /* DATAMANAGERABSTRACT_HPP_ */
