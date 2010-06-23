/*
 * \file dataManagerAbstract.hpp
 *
 *  Created on: Apr 22, 2010
 *      Author: nmansard
 *      \ingroup rtslam
 */

#ifndef DATAMANAGERABSTRACT_HPP_
#define DATAMANAGERABSTRACT_HPP_

#include "rtslam/parents.hpp"
#include "rtslam/sensorAbstract.hpp"

namespace jafar {
  namespace rtslam {

    class ObservationAbstract;


    /**
     * \ingroup rtslam
     */
    class DataManagerAbstract
      : public ParentOf<ObservationAbstract>
      , public ChildOf<SensorAbstract>
      , public boost::enable_shared_from_this<DataManagerAbstract>
    {
    public:
      // define the function linkToParentSensor().
      ENABLE_LINK_TO_PARENT(SensorAbstract,Sensor,DataManagerAbstract);
      // define the functions sensorPtr() and sensor().
      ENABLE_ACCESS_TO_PARENT(SensorAbstract,sensor);
      // define the type ObservationList, and the function observationList().
      ENABLE_ACCESS_TO_CHILDREN(ObservationAbstract,Observation,observation);

      virtual ~DataManagerAbstract(void) {}

      virtual void processData( boost::shared_ptr<RawAbstract> data ) = 0;
    };


    template< class RawSpec,class ObservationSpec,class SensorSpec,class LandmarkSpec >
    class DataManagerActiveSearch
      : public DataManagerAbstract
      , public SpecificChildOf<SensorSpec>
    {
    protected:
      std::list< boost::weak_ptr<ObservationSpec> > observationSpecList;

    public:
      // Define the function linkToParentSensorSpec.
      ENABLE_LINK_TO_SPECIFIC_PARENT(SensorAbstract,SensorSpec,
				     SensorSpec,DataManagerAbstract);
      // Define the functions sensorSpec() and sensorSpecPtr().
      ENABLE_ACCESS_TO_SPECIFIC_PARENT(SensorSpec,sensorSpec);

      void createObservation( boost::shared_ptr<LandmarkSpec> lmkPtr )
      {
	boost::shared_ptr<ObservationSpec> obs_ptr
	  ( new ObservationSpec(sensorSpecPtr(),lmkPtr) );
	obs_ptr->linkToParentDataManager( shared_from_this() );
	observationSpecList.push_back( obs_ptr );
	obs_ptr->linkToSensor( sensorSpecPtr() );
      }

    };

  }
}//namespace jafar::rtslam
#endif /* DATAMANAGERABSTRACT_HPP_ */
