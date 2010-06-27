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
      virtual landmark_ptr_t createLandmarkInit( void ) = 0;
      virtual landmark_ptr_t createLandmarkConverged( void ) = 0;

    public:
      virtual ~MapManagerAbstract( void ) {}

      virtual bool mapSpaceForInit() = 0;
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
      virtual landmark_ptr_t createLandmarkConverged( void )
      {
	return boost::shared_ptr<LandmarkAdvanced>( new LandmarkAdvanced(mapPtr()) );
      }

    public:
      virtual ~MapManager( void ) {}
      virtual bool mapSpaceForInit(){
      	return mapPtr()->unusedStates(LandmarkInit::size());
      }

    };

  };
};

#endif // #ifndef MAPMANAGER_HPP_
