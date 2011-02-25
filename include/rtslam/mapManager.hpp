/**
 * \file mapManager.hpp
 *
 * \date 22/04/2010
 * \author nmansard
 * \ingroup rtslam
 */

#ifndef MAPMANAGER_HPP_
#define MAPMANAGER_HPP_

#include "rtslam/parents.hpp"
#include "rtslam/mapAbstract.hpp"

namespace jafar {
	namespace rtslam {

		class LandmarkAbstract;
		class DataManagerAbstract;

		/**
			This class is the abstract class for map managers, that manages
			the life of landmarks at the map level (creation, 
			reparametrization, deletion, etc).
		*/
		class MapManagerAbstract:
					public ParentOf<LandmarkAbstract>,
					public ParentOf<DataManagerAbstract>,
					public ChildOf<MapAbstract>,
					public boost::enable_shared_from_this<MapManagerAbstract>
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
				virtual landmark_ptr_t createLandmarkInit(void) = 0;
				virtual landmark_ptr_t createLandmarkConverged(landmark_ptr_t lmkinit,
					jblas::ind_array &_icomp) = 0;
				/* Compute the size of the complement between init and
				 * converged lmk, ie the difference of the size of the states. */
				virtual size_t sizeComplement(void) = 0;

			public:
				virtual ~MapManagerAbstract(void) {
				}

				virtual bool mapSpaceForInit() = 0;
				/* Return the pointer to the created observation that correspond to the
				 * dmaOrigin. */
				observation_ptr_t createNewLandmark(data_manager_ptr_t dmaOrigin);
				void reparametrizeLandmark(landmark_ptr_t lmk);
				void unregisterLandmark(landmark_ptr_t lmk, bool liberateFilter = true);
				void manage(void);
				void manageMap(void);
		};

		
		/**
			This class is a generic implementation of MapManagerAbstract, that
			works with every type of landmark given they are provided as template
			parameter.
		*/
		template<class LandmarkInit, class LandmarkConverged>
		class MapManager: public MapManagerAbstract {
			protected:
				virtual landmark_ptr_t createLandmarkInit(void) {
					return boost::shared_ptr<LandmarkInit>(new LandmarkInit(mapPtr()));
				}
				virtual landmark_ptr_t createLandmarkConverged(landmark_ptr_t lmkinit,
					jblas::ind_array &_icomp)
				{
					return boost::shared_ptr<LandmarkConverged>(
						new LandmarkConverged(mapPtr(), lmkinit, _icomp));
				}
				virtual size_t sizeComplement(void) {
					return (LandmarkInit::size() - LandmarkConverged::size());
				}
			public:
				virtual ~MapManager(void) {
				}
				virtual bool mapSpaceForInit() {
					return mapPtr()->unusedStates(LandmarkInit::size());
				}
		};

	}
}

#endif // #ifndef MAPMANAGER_HPP_
