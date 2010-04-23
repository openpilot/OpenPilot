/*
 * dataManagerAbstract.hpp
 *
 *  Created on: Apr 22, 2010
 *      Author: nmansard
 */

#ifndef DATAMANAGERABSTRACT_HPP_
#define DATAMANAGERABSTRACT_HPP_

#include "rtslam/parents.hpp"

class ObservationAbstract;

class DataManagerAbstract
: public WeakParentOf<ObservationAbstract>
{
	ENABLE_ACCESS_TO_CHILDREN(ObservationAbstract,Observation,observation);

};


#endif /* DATAMANAGERABSTRACT_HPP_ */
