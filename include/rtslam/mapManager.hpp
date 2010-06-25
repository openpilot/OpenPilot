/*
 * \file mapManager.hpp
 *
 *  Created on: Apr 22, 2010
 *      Author: nmansard
 *      \ingroup rtslam
 */

#ifndef MAPMANAGER_HPP_
#define MAPMANAGER_HPP_

#include "rtslam/parents.hpp"
#include "rtslam/mapAbstract.hpp"

namespace jafar {
  namespace rtslam {

    class LandmarkAbstract;
    class DataManagerAbstract;

    class MapManagerAbstract
      : public ParentOf<LandmarkAbstract>
      , public ParentOf<DataManagerAbstract>
      , public ChildOf<MapAbstract>
      , public boost::enable_shared_from_this<MapManagerAbstract>
    {
    public:
      // define the function linkToParentMap().
      ENABLE_LINK_TO_PARENT(MapAbstract,Map,MapManagerAbstract);
      // define the functions mapPtr() and map().
      ENABLE_ACCESS_TO_PARENT(MapAbstract,map);
      // define the type LandmarkList, and the function landmarkList().
      ENABLE_ACCESS_TO_CHILDREN(LandmarkAbstract,Landmark,landmark);
      // define the type DataManagerList, and the function dataManagerList().
      ENABLE_ACCESS_TO_CHILDREN(DataManagerAbstract,DataManager,dataManager);

    protected:
      boost::shared_ptr<ObservationFactory> obsFactory;
    public:
      void setObservationFactory( boost::shared_ptr<ObservationFactory> of ) { obsFactory=of; }
      boost::shared_ptr<ObservationFactory> observationFactory( void ) { return obsFactory; }

    protected:
      virtual landmark_ptr_t createLandmarkInit( void ) = 0;
      virtual landmark_ptr_t createLandmarkAdvanced( void ) = 0;

    public:
      virtual ~MapManagerAbstract( void ) {}

      /* Return the pointer to the created observation that correspond to the
       * dmaOrigin. */
      observation_ptr_t createNewLandmark( data_manager_ptr_t dmaOrigin );
      void reparametrizeLandmark( landmark_ptr_t lmk );
      void killLandmark( landmark_ptr_t lmk );
      void manageMap( void );
  };


    template< class LandmarkInit, class LandmarkAdvanced>
    class MapManager
      :public MapManagerAbstract
    {
    protected:
      virtual landmark_ptr_t createLandmarkInit( void )
      {
	return boost::shared_ptr<LandmarkInit>( new LandmarkInit(mapPtr()) );
      }
      virtual landmark_ptr_t createLandmarkAdvanced( void )
      {
	return boost::shared_ptr<LandmarkAdvanced>( new LandmarkAdvanced(mapPtr()) );
      }

    public:
      virtual ~MapManager( void ) {}


    };

  };
};

#endif // #ifndef MAPMANAGER_HPP_
