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
#include "rtslam/landmarkFactory.hpp"

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
				landmark_factory_ptr_t lmkFactory;
			public:
				MapManagerAbstract(landmark_factory_ptr_t lmkFactory):
					lmkFactory(lmkFactory) {}
				virtual ~MapManagerAbstract(void) {
				}
				/**
				 Does map has enough space to init a new landmark ?
				*/
				virtual bool mapSpaceForInit() {
					return mapPtr()->unusedStates(lmkFactory->sizeInit());
				}
				/**
				 Return the pointer to the created observation that correspond to the dmaOrigin.
				*/
				observation_ptr_t createNewLandmark(data_manager_ptr_t dmaOrigin);
				void reparametrizeLandmark(landmark_ptr_t lmkIter);
				LandmarkList::iterator reparametrizeLandmark(LandmarkList::iterator lmkIter)
				{ // FIXME do better than this! will crash if only one element.
					landmark_ptr_t lmkPtr = *lmkIter;
					lmkIter++;
					reparametrizeLandmark(lmkPtr);
					lmkIter--;
					return lmkIter;
				}
				void unregisterLandmark(landmark_ptr_t lmkPtr, bool liberateFilter = true);
				LandmarkList::iterator unregisterLandmark(LandmarkList::iterator lmkIter, bool liberateFilter = true)
				{ // FIXME do better than this! will crash if only one element.
					landmark_ptr_t lmkPtr = *lmkIter;
					lmkIter++;
					unregisterLandmark(lmkPtr, liberateFilter);
					lmkIter--;
					return lmkIter;
				}

				/**
					Manage when the landmarks are removed from the map or reparametrized
				*/
				virtual void manage(void) = 0;
				/**
					Returns if the given observation must be exclusive in the FeatureManager
					(ie we believe there are good chances to find it again very soon),
					or if new landmarks can be initialized ignoring this observation
					(ie we believe there are few chances to find it again very soon)
				*/
				virtual bool isExclusive(observation_ptr_t obsPtr) = 0;
		};

		
		/**
			This class is a default implementation of MapManagerAbstract,
			that only reparametrize and kill landmarks with a too large search area
		*/
		class MapManager: public MapManagerAbstract {
			protected:
				double reparTh;    ///< linearity threshold for reparametrization
				double killSizeTh; ///< maximum search size, if bigger it will be deleted
			protected:
				virtual void manageReparametrization();
				virtual void manageDefaultDeletion();
				virtual void manageDeletion() {}; // to overload
			public:
				MapManager(landmark_factory_ptr_t lmkFactory, double reparTh = 0.1, double killSizeTh = 100000):
					MapManagerAbstract(lmkFactory), reparTh(reparTh), killSizeTh(killSizeTh) {}
				virtual ~MapManager(void) {}
								
				virtual void manage()
				{
					manageDefaultDeletion();
					manageDeletion();
					manageReparametrization();
				}
				
				virtual bool isExclusive(observation_ptr_t obsPtr)
				{
					return true;
				}
		};

		
		/**
			Map manager with a very short memory policy. It deletes very quickly
			lost landmarks, so that it won't be able to close much loops, but can
			track more landmarks and be faster.
		*/
		class MapManagerOdometry: public MapManager {
			public:
				MapManagerOdometry(landmark_factory_ptr_t lmkFactory, double reparTh, double killSizeTh):
					MapManager(lmkFactory, reparTh, killSizeTh) {}
				virtual void manageDeletion();
				virtual bool isExclusive(observation_ptr_t obsPtr);
		};
		
		
		/**
			Map manager made for doing slam as long as possible while optimizing
			the use of the map. When the map is full, lower quality and spatially
			redundant landmarks are removed to make room for new landmarks.
		*/
		class MapManagerGlobal: public MapManager {
			protected:
				double killSearchTh;      ///< minimum number of times the landmark must have been searched to be deleted for match or consistency reasons
				double killMatchTh;       ///< ratio match/search threshold
				double killConsistencyTh; ///< ratio consistency/search threshold
			public:
				MapManagerGlobal(landmark_factory_ptr_t lmkFactory, double reparTh, double killSizeTh,
				                double killSearchTh, double killMatchTh, double killConsistencyTh):
				  MapManager(lmkFactory, reparTh, killSizeTh),
				  killSearchTh(killSearchTh), killMatchTh(killMatchTh), killConsistencyTh(killConsistencyTh) {}
				virtual void manageDeletion();
				virtual bool mapSpaceForInit();
		};
		
		
		/**
			Map manager made for managing a spatially local map in a hierarchical
			multimap framework. When the map is full, it is supposed to be closed.
		*/
		class MapManagerLocal: public MapManager {
			public:
				MapManagerLocal(landmark_factory_ptr_t lmkFactory, double reparTh, double killSizeTh):
					MapManager(lmkFactory, reparTh, killSizeTh) {}
				virtual void manageDeletion()
				{
					// TODO
				}
		};
		
		
	}
}

#endif // #ifndef MAPMANAGER_HPP_
