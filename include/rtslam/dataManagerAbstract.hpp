/*
 * \file dataManagerAbstract.hpp
 *
 *  Created on: June 22, 2010
 *      Author: nmansard
 *      \ingroup rtslam
 */

#ifndef DATAMANAGERABSTRACT_HPP_
#define DATAMANAGERABSTRACT_HPP_

#include "rtslam/parents.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/mapManager.hpp"

namespace jafar {
  namespace rtslam {

    class ObservationAbstract;

    /**
     * \ingroup rtslam
     */
    class DataManagerAbstract
      : public ParentOf<ObservationAbstract>
      , public ChildOf<SensorAbstract>
      , public ChildOf<MapManagerAbstract>
      , public boost::enable_shared_from_this<DataManagerAbstract>
    {
    public:
      // define the function linkToParentSensor().
      ENABLE_LINK_TO_PARENT(SensorAbstract,Sensor,DataManagerAbstract);
      // define the functions sensorPtr() and sensor().
      ENABLE_ACCESS_TO_PARENT(SensorAbstract,sensor);
      // define the function linkToParentMapManager().
      ENABLE_LINK_TO_PARENT(MapManagerAbstract,MapManager,DataManagerAbstract);
      // define the functions mapManagerPtr() and mapManager().
      ENABLE_ACCESS_TO_PARENT(MapManagerAbstract,mapManager);
      // define the type ObservationList, and the function observationList().
      ENABLE_ACCESS_TO_CHILDREN(ObservationAbstract,Observation,observation);

      virtual ~DataManagerAbstract(void) {}

      virtual void process( boost::shared_ptr<RawAbstract> data ) = 0;
    };

  }
}//namespace jafar::rtslam
#endif /* DATAMANAGERABSTRACT_HPP_ */
