/**
 * mapManagerAbstract.cpp
 *
 * \date 10/03/2010
 * \author nmansard@laas.fr
 *
 *  \file mapManagerAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include <boost/shared_ptr.hpp>

#include "rtslam/rtSlam.hpp"
#include "rtslam/mapManager.hpp"
#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/observationFactory.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/dataManagerAbstract.hpp"

namespace jafar {
  namespace rtslam {
    using namespace std;



    observation_ptr_t MapManagerAbstract::
    createNewLandmark( data_manager_ptr_t dmaOrigin )
    {
      landmark_ptr_t newLmk = createLandmarkInit();
      newLmk->setId();
      newLmk->linkToParentMapManager( shared_from_this() );
      observation_ptr_t resObs;

      for( MapManagerAbstract::DataManagerList::iterator
	     iterDMA = dataManagerList().begin();
	   iterDMA!=dataManagerList().end();++iterDMA )
	{
	  data_manager_ptr_t dma = *iterDMA;
	  observation_ptr_t newObs
	    = dma->observationFactory()->create( dma->sensorPtr(),newLmk );

	  /* Insert the observation in the graph. */
	  newObs->linkToParentDataManager( dma );
	  newObs->linkToParentLandmark( newLmk );
	  newObs->linkToSensor( dma->sensorPtr() );
	  newObs->linkToSensorSpecific(dma->sensorPtr());
	  newObs->setId();




	  /* Store for the return the obs corresponding to the dma origin. */
	  if(dma == dmaOrigin) resObs = newObs;
	}
      return resObs;
    }


    void MapManagerAbstract::
    manageMap( void )
    {
      // foreach lmk
      for( LandmarkList::iterator iterLmk = landmarkList().begin();
	   iterLmk!=landmarkList().end(); ++iterLmk )
      {
	landmark_ptr_t lmk = *iterLmk;
	if(0)//TODO-NMSD lmk->needToDie() )
	  {
	    iterLmk++;
	    killLandmark(lmk);
	    iterLmk--;
	  }
	else if(0)//TODO-NMSD  lmk->needToReparametrize() )
	  {
	    iterLmk++;
	    reparametrizeLandmark(lmk);
	    iterLmk--;
	  }
      }
    }


  };
};
