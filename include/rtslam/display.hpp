/**
 * \file display.hpp
 * \author croussil@laas.fr
 * \date 25/03/2010
 * File defining a generic display architecture
 * \ingroup rtslam
 */

#ifndef DISPLAY_HPP_
#define DISPLAY_HPP_

#include "rtslam/observationPinHoleEuclideanPoint.hpp"
#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/robotAbstract.hpp"

#include "kernel/IdFactory.hpp"
#include "boost/variant.hpp"
#include <boost/type_traits/is_same.hpp>

namespace jafar {
namespace rtslam {
namespace display {

	class ViewerAbstract;
	
	/** **************************************************************************
	This is the base class for objects that will store display data for a viewer
	\ingroup rtslam
	*/
	class DisplayDataAbstract
	{
		// bufferized slam object data
		// +
		// display objects
			ViewerAbstract *viewer;
		public:
			DisplayDataAbstract(ViewerAbstract *viewer_): viewer(viewer_) {}
			virtual ~DisplayDataAbstract() {}
			//virtual void bufferize() = 0; // not virtual, we want to allow inlining, and we are using templates
			//virtual void render() = 0; 
	};
	
	/** **************************************************************************
	\ingroup rtslam
	*/
	class WorldDisplay : public DisplayDataAbstract
	{
		public:
			rtslam::WorldAbstract *slamWor_; // cannot use shared_ptr or it will never be freed
			WorldDisplay(ViewerAbstract *viewer_, rtslam::WorldAbstract *_slamWor, WorldDisplay *_garbage):
				DisplayDataAbstract(viewer_), slamWor_(_slamWor) {}
	};
	
	/** **************************************************************************
	\ingroup rtslam
	*/
	class MapDisplay : public DisplayDataAbstract
	{
		public:
			rtslam::MapAbstract *slamMap_;
			WorldDisplay *dispWorld_;
			MapDisplay(ViewerAbstract *viewer_, rtslam::MapAbstract *_slamMap, WorldDisplay *_dispWorld): 
				DisplayDataAbstract(viewer_), slamMap_(_slamMap), dispWorld_(_dispWorld) {}
	};

	/** **************************************************************************
	This is the base class for robot objects that will store display data for a viewer.
	When writing a new viewer, you must create a type that inherits from this.
	\ingroup rtslam
	*/
	class RobotDisplay : public DisplayDataAbstract
	{
		public:
			rtslam::RobotAbstract *slamRob_;
			MapDisplay *dispMap_;
			RobotDisplay(ViewerAbstract *viewer_, rtslam::RobotAbstract *_slamRob, MapDisplay *_dispMap): 
				DisplayDataAbstract(viewer_), slamRob_(_slamRob), dispMap_(_dispMap) {}
	};

	/** **************************************************************************
	This is the base class for sensor objects that will store display data for a viewer.
	When writing a new viewer, you must create a type that inherits from this.
	\ingroup rtslam
	*/
	class SensorDisplay : public DisplayDataAbstract
	{
		public:
			rtslam::SensorAbstract *slamSen_;
			RobotDisplay *dispRobot_;
			SensorAbstract::type_enum type_;
			SensorDisplay(ViewerAbstract *viewer_, rtslam::SensorAbstract *_slamSen, RobotDisplay *_dispRobot): 
				DisplayDataAbstract(viewer_), slamSen_(_slamSen), dispRobot_(_dispRobot), type_(_slamSen->type) {}
	};

	/** **************************************************************************
	This is the base class for landmark objects that will store display data for a viewer.
	When writing a new viewer, you must create a type that inherits from this.
	\ingroup rtslam
	*/
	class LandmarkDisplay : public DisplayDataAbstract
	{
		public:
			rtslam::LandmarkAbstract *slamLmk_;
			MapDisplay *dispMap_;
			enum Type { ltPoint };
			enum Phase { init, converged };
			Type  type_;
			Phase phase_;
			static Type convertType(rtslam::LandmarkAbstract *_slamLmk)
			{
				switch (_slamLmk->getGeomType())
				{
					case rtslam::LandmarkAbstract::POINT:
						return ltPoint;
					default:
						JFR_ERROR(RtslamException, RtslamException::UNKNOWN_FEATURE_TYPE, "Don't know how to display this type of landmark" << _slamLmk->getGeomType());
				}
			}
			static Phase convertPhase(rtslam::LandmarkAbstract *_slamLmk)
			{
				switch (_slamLmk->type)
				{
					case rtslam::LandmarkAbstract::PNT_EUC:
						return converged;
					default:
						return init;
				}
			}
			LandmarkDisplay(ViewerAbstract *viewer_, rtslam::LandmarkAbstract *_slamLmk, MapDisplay *_dispMap): 
				DisplayDataAbstract(viewer_), slamLmk_(_slamLmk), dispMap_(_dispMap),
				type_(convertType(_slamLmk)), phase_(convertPhase(_slamLmk)) {}
	};


	/** **************************************************************************
	This is the base class for observation objects that will store display data for a viewer.
	When writing a new viewer, you must create a type that inherits from this.
	\ingroup rtslam
	*/
	class ObservationDisplay : public DisplayDataAbstract
	{
		public:
			rtslam::ObservationAbstract *slamObs_;
			SensorDisplay *dispSen_;
			SensorAbstract::type_enum sensorType_;
			LandmarkDisplay::Type  landmarkGeomType_;
			LandmarkDisplay::Phase landmarkPhase_;
			ObservationDisplay(ViewerAbstract *viewer_, rtslam::ObservationAbstract *_slamObs, SensorDisplay *_dispSen): 
				DisplayDataAbstract(viewer_), slamObs_(_slamObs), dispSen_(_dispSen)
			{
				sensorType_ = _slamObs->sensorPtr()->type;
				landmarkGeomType_ = LandmarkDisplay::convertType(&*_slamObs->landmarkPtr());
				landmarkPhase_ = LandmarkDisplay::convertPhase(&*_slamObs->landmarkPtr());
			}
	};

	// TODO can I give the viewer instead of the parent ? it may decide
	/** **************************************************************************
	This is the base class for a viewer. 
	*/
	//template<class Render = RenderAbstract>
	class ViewerAbstract
	{
		protected:
			typedef boost::variant<rtslam::world_ptr_t, rtslam::map_ptr_t, rtslam::robot_ptr_t, 
				rtslam::sensor_ptr_t, rtslam::landmark_ptr_t, rtslam::observation_ptr_t> SlamObjectPtr;
			typedef std::vector<SlamObjectPtr> SlamObjectsList;
			SlamObjectsList slamObjects_; ///< all the slam objects at the time of bufferization (that must be displayed)

			template<class DisplayType, class ParentDisplayType, class SlamPtrType, class ParentSlamPtrType>
			inline void bufferizeObject(SlamPtrType slamObject, ParentSlamPtrType parentSlam, unsigned int id)
			{
				// add the object to the list
				slamObjects_.push_back(slamObject);
				// if the array hasn't been created
				if (slamObject->displayData.size() <= id)
				{
					int oldsize = slamObject->displayData.size();
					slamObject->displayData.resize(id+1);
					for(unsigned int i = oldsize; i <= id; ++i)
						slamObject->displayData[i] = NULL;
				}
				// if the object hasn't been created
				if (slamObject->displayData[id] == NULL)
				{
					ParentDisplayType *parentDisp;
					if (boost::is_same<ParentDisplayType,MapDisplay>::value)
						parentDisp = NULL;
					else
					{
						if (parentSlam->displayData.size() <= id) 
							parentDisp = NULL;
						else
							parentDisp = PTR_CAST<ParentDisplayType*>(parentSlam->displayData[id]);
					}
					slamObject->displayData[id] = new DisplayType(this, &*slamObject, parentDisp);
				}
				// bufferize the object
				DisplayType *objDisp = PTR_CAST<DisplayType*>(slamObject->displayData[id]);
				objDisp->bufferize();
			}
			
		public:
			static IdFactory& idFactory()
			{
				static IdFactory idFactory_;
				return idFactory_;
			}
			//static kernel::IdFactory idFactory_;
			//kernel::IdFactory::storage_t id_;
			
		public:
			//ViewerAbstract(): id_(idFactory().getId()-1) {}
			//virtual ~ViewerAbstract() { idFactory().releaseId(id_); }
			virtual ~ViewerAbstract() {}
			/**
			Put the objects in slamObjects_, bufferize all display objects and construct them if necessary.
			Don't forget to clear slamObjects_ first.
			*/
			//virtual void bufferize(rtslam::map_ptr_t _map) = 0;
//			virtual void render() = 0;
	};
	
	
/**
release and garbageCollect are thread safe
but garbageCollect is not thread safe with itself, 
but it is not useful (would need another mutex)
*/
template <typename T>
class ThreadSafeGarbageCollector
{
	private:
		int gcn;
		std::vector<T> gc[2];
		boost::mutex m;
		struct Destroy : public boost::static_visitor<>
			{ template<typename U> void operator()(U *item) const { delete item; } };
	public:
		ThreadSafeGarbageCollector(): gcn(0) {}
		template<typename U> void release(U *item)
		{
			boost::unique_lock<boost::mutex> lock(m);
			gc[gcn].push_back(item);
		}
		void garbageCollect()
		{
			boost::unique_lock<boost::mutex> lock(m);
			std::vector<T> &mygc = gc[gcn];
			gcn = 1-gcn;
			lock.unlock();
			
			for(typename std::vector<T>::iterator it = mygc.begin(); it != mygc.end(); ++it)
				boost::apply_visitor(Destroy(), *it);
			mygc.clear();
		}
};

	
	/** **************************************************************************
	This is the base class for a viewer that can render the scene.
	When writing a new viewer, it must be inherited from this.
	*/
	template<class WorldDisplayType, class MapDisplayType, class RobotDisplayType, 
		class SensorDisplayType, class LandmarkDisplayType, class ObservationDisplayType, class GarbageType>
	class Viewer : public ViewerAbstract, public ThreadSafeGarbageCollector<GarbageType>
	{
		protected:
			//template<class RobotDisplayType, class SensorDisplayType, class LandmarkDisplayType, class ObservationDisplayType>
			class Render : public boost::static_visitor<>// : public RenderAbstract
			{
				public:
					typedef Viewer<WorldDisplayType, MapDisplayType, RobotDisplayType, 
						SensorDisplayType, LandmarkDisplayType, ObservationDisplayType, GarbageType> MyViewer;
					
					void operator()(rtslam::world_ptr_t const &wor) const {
						if (!boost::is_same<WorldDisplayType,WorldDisplay>::value) {
							WorldDisplayType &worDisp = *PTR_CAST<WorldDisplayType*>(wor->displayData[MyViewer::id()]);
							worDisp.render();
						}
					}
					void operator()(rtslam::map_ptr_t const &map) const {
						if (!boost::is_same<MapDisplayType,MapDisplay>::value) {
							MapDisplayType &mapDisp = *PTR_CAST<MapDisplayType*>(map->displayData[MyViewer::id()]);
							mapDisp.render();
						}
					}
					void operator()(rtslam::robot_ptr_t const &rob) const {
						if (!boost::is_same<RobotDisplayType,RobotDisplay>::value) {
							RobotDisplayType &robDisp = *PTR_CAST<RobotDisplayType*>(rob->displayData[MyViewer::id()]);
							robDisp.render();
						}
					}
					void operator()(rtslam::sensor_ptr_t const &sen) const {
						if (!boost::is_same<SensorDisplayType,SensorDisplay>::value) {
							SensorDisplayType &senDisp = *PTR_CAST<SensorDisplayType*>(sen->displayData[MyViewer::id()]);
							senDisp.render();
						}
					}
					void operator()(rtslam::landmark_ptr_t const &lmk) const {
						if (!boost::is_same<LandmarkDisplayType,LandmarkDisplay>::value) {
							LandmarkDisplayType &lmkDisp = *PTR_CAST<LandmarkDisplayType*>(lmk->displayData[MyViewer::id()]);
							lmkDisp.render();
						}
					}
					void operator()(rtslam::observation_ptr_t const &obs) const {
						if (!boost::is_same<ObservationDisplayType,ObservationDisplay>::value) {
							ObservationDisplayType &obsDisp = *PTR_CAST<ObservationDisplayType*>(obs->displayData[MyViewer::id()]);
							obsDisp.render();
						}
					}
			};
			
			
		public:
			static IdFactory::storage_t& id()
			{
				static IdFactory::storage_t id = ViewerAbstract::idFactory().getId()-1;
				return id;
			}
			
		public:
			~Viewer() { this->garbageCollect(); }
			inline void clear()
			{
				slamObjects_.clear();
			}
			
			//virtual void garbageCollect() = 0;
			
			/**
			This function bufferizes all the objects
			*/
			inline void bufferize(rtslam::world_ptr_t wor)
			{
				// bufferize world
				if (!boost::is_same<WorldDisplayType,WorldDisplay>::value) // bufferize world
					bufferizeObject<WorldDisplayType, WorldDisplayType, world_ptr_t, world_ptr_t>(wor, wor, id());
				// bufferize maps
				for(WorldAbstract::MapList::iterator map = wor->mapList().begin(); map != wor->mapList().end(); ++map)
					bufferize(*map, wor);
			}
			
			inline void bufferize(rtslam::map_ptr_t map, rtslam::world_ptr_t wor)
			{
				// bufferize map
				if (!boost::is_same<MapDisplayType,MapDisplay>::value)
					bufferizeObject<MapDisplayType, WorldDisplayType, map_ptr_t, world_ptr_t>(map, wor, id());
				// bufferize robots
				for(MapAbstract::RobotList::iterator rob = map->robotList().begin(); rob != map->robotList().end(); ++rob)
					bufferize(*rob,map);
				// bufferize landmarks
				for(MapAbstract::MapManagerList::iterator mm = map->mapManagerList().begin(); mm!=map->mapManagerList().end(); ++mm )
				  for(MapManagerAbstract::LandmarkList::iterator lmk = (*mm)->landmarkList().begin(); lmk != (*mm)->landmarkList().end(); ++lmk)
				    bufferize(*lmk,map);
			}
			
			inline void bufferize(rtslam::robot_ptr_t rob, rtslam::map_ptr_t map)
			{
				// bufferize robot
				if (!boost::is_same<RobotDisplayType,RobotDisplay>::value)
					bufferizeObject<RobotDisplayType, MapDisplayType, robot_ptr_t, map_ptr_t>(rob, map, id());
				// bufferize sensors
				for(RobotAbstract::SensorList::iterator sen = rob->sensorList().begin(); sen != rob->sensorList().end(); ++sen)
					bufferize(*sen,rob);
			}

			inline void bufferize(rtslam::sensor_ptr_t sen, rtslam::robot_ptr_t rob)
			{
				// bufferize sensor
				if (!boost::is_same<SensorDisplayType,SensorDisplay>::value)
					bufferizeObject<SensorDisplayType, RobotDisplayType, sensor_ptr_t, robot_ptr_t>(sen, rob, id());
				// bufferize observations
				for(SensorAbstract::DataManagerList::iterator dma = sen->dataManagerList().begin(); dma!=sen->dataManagerList().end();++dma)
				  for(LandmarkAbstract::ObservationList::iterator obs = (*dma)->observationList().begin(); obs != (*dma)->observationList().end(); ++obs)
				    bufferize(*obs,sen);
			}

			inline void bufferize(rtslam::observation_ptr_t obs, rtslam::sensor_ptr_t sen)
			{
				// bufferize observationbufferizeObject
				if (!boost::is_same<ObservationDisplayType,ObservationDisplay>::value) // bufferize observation
					bufferizeObject<ObservationDisplayType, SensorDisplayType, observation_ptr_t, sensor_ptr_t>(obs, sen, id());
			}

			inline void bufferize(rtslam::landmark_ptr_t lmk, rtslam::map_ptr_t map)
			{
				// bufferize landmark
				if (!boost::is_same<LandmarkDisplayType,LandmarkDisplay>::value) // bufferize landmark
					bufferizeObject<LandmarkDisplayType, MapDisplayType, landmark_ptr_t, map_ptr_t>(lmk, map, id());
			}
			
			

			/**
			Render the scene.
			@param _clear 
			*/
			void render()
			{
				/*
				We need to clear at the end, because we must clear before bufferize, 
				and bufferize is called by the slam thread so it cannot call it.
				But then when if a landmark is destroyed, it is immediatly destroyed
				as it is not retained by this list, so the display object is also destroyed,
				and it is not thread safe. So when the display object is destroyed, display lib objects
				are just moved to the garbage collector, and destroyed by this thread before the next rendering.
				Anyway destroying the object can be long so it is better to do it here.
				Maybe it is not neccessary anymore to have a vector of variants here, but just
				a vector of display data.
				*/
				this->garbageCollect(); // strange, the "this" is necessary...
				for(SlamObjectsList::iterator it = slamObjects_.begin(); it != slamObjects_.end(); ++it)
				{
					boost::apply_visitor(Render(), *it);
				}
				// clear viewer
				clear();
			}
			
	};

	/*
		typedef enum lmk_state {
			lmk_state_init,
			lmk_state_converged
		} lmk_state ;

		typedef enum lmk_events {
			lmk_events_not_predicted,
			lmk_events_predicted,
			lmk_events_matched,
			lmk_events_updated
		} lmk_events ;
*/
		typedef enum color {
			color_transparency,
			color_yellow,
			color_magenta,
			color_blue,
			color_red,
			color_cyan,
			color_pink,
			color_orange

		} color ;

		typedef struct colorRGB  {
				int R, G, B;
				void set(int R_, int G_, int B_) { R=R_; G=G_; B=B_; }
		} colorRGB ;

		class ColorManager
		{
			public:
				
				static color getColorObject_prediction(LandmarkDisplay::Phase lmk_phase, ObservationAbstract::Events &events)
				{
					switch(lmk_phase)
					{
						case LandmarkDisplay::init: {
							if (events.updated) return color_red;
							if (events.matched) return color_magenta;
							if (events.predicted) return color_magenta;
							return color_yellow;
						}
						case LandmarkDisplay::converged: {
							if (events.updated) return color_cyan;
							if (events.matched) return color_blue;
							if (events.predicted) return color_blue;
							return color_transparency;
						}
						default:
							std::cout << "getColorObject_prediction: Warning: undefined phase " << lmk_phase << std::endl ;
							return color_transparency;
					}
				}
				
				static color getColorObject_measure(LandmarkDisplay::Phase lmk_phase, ObservationAbstract::Events &events)
				{
					switch(lmk_phase)
					{
						case LandmarkDisplay::init: {
							if (events.updated) return color_yellow;
							if (events.matched) return color_yellow;
							if (events.predicted) return color_orange;
							return color_yellow;
						}
						case LandmarkDisplay::converged: {
							if (events.updated) return color_yellow;
							if (events.matched) return color_yellow;
							if (events.predicted) return color_orange;
							return color_transparency;
						}
						default:
							std::cout << "getColorObject_measure: Warning: undefined phase " << lmk_phase << std::endl ;
							return color_transparency;
					}
				}
			};

		/*
		static color getColorObject_prediction(lmk_state lmk_state, lmk_events lmk_events) {

			if (lmk_state==lmk_state_init) {
				switch (lmk_events) {
					case lmk_events_not_predicted:
						return color_transparency;
					case lmk_events_predicted:
						return color_magenta;
					case lmk_events_matched:
						return color_magenta;
					case lmk_events_updated:
						return color_red;
					default:
						std::cout << __FILE__ << ":" << __LINE__ << "Unknown lmk lmk_events " << lmk_events << std::endl;
						break;
				}
			}
			if (lmk_state==lmk_state_converged) {
				switch (lmk_events) {
					case lmk_events_not_predicted:
						return color_transparency;
					case lmk_events_predicted:
						return color_blue;
					case lmk_events_matched:
						return color_blue;
					case lmk_events_updated:
						return color_cyan;
					default:
						std::cout << __FILE__ << ":" << __LINE__ << "Unknown lmk lmk_events " << lmk_events << std::endl;
						break;
				}
			}
			cout << "warning color undefined for object types in display" << endl ;
			return color_transparency;
		}

		static color getColorObject_measure    (lmk_state lmk_state, lmk_events lmk_events) {
			if (lmk_state==lmk_state_init) {
				switch (lmk_events) {
					case lmk_events_not_predicted:
						return color_yellow;
					case lmk_events_predicted:
						return color_orange;
					case lmk_events_matched:
						return color_yellow;
					case lmk_events_updated:
						return color_yellow;
					default:
						std::cout << __FILE__ << ":" << __LINE__ << " Unknown lmk lmk_events " << lmk_events << std::endl;
						break;
				}
			}
			if (lmk_state==lmk_state_converged) {
				switch (lmk_events) {
					case lmk_events_not_predicted:
						return color_transparency;
					case lmk_events_predicted:
						return color_orange;
					case lmk_events_matched:
						return color_yellow;
					case lmk_events_updated:
						return color_yellow;
					default:
						std::cout << __FILE__ << ":" << __LINE__ << " Unknown lmk lmk_events " << lmk_events << std::endl;
						break;
				}
			}
			cout << "warning color undefined for object types in display" << endl ;
			return color_transparency;
		}
*/
		static colorRGB getColorRGB(color colorOrigin)  {
			colorRGB result ;
			result.set(0,0,0);
			switch (colorOrigin) {
				case color_blue:
					result.set(0,0,255);
					break;
				case color_cyan:
					result.set(0,255,255);
					break;
				case color_magenta:
					result.set(255,0,255);
					break;
				case color_red:
					result.set(255,0,0);
					break;
				case color_transparency:
					result.set(255,255,255);
					break;
				case color_yellow:
					result.set(255,255,0);
					break;
				case color_pink:
						result.set(253,103,223);
						break;
				case color_orange:
						result.set(255,128,0);
						break;
				default:
					//std::cout << __FILE__ << ":" << __LINE__ << " Unknown lmk lmk_events " << lmk_events << std::endl;
					break;
			}
			return result ;
		}

}}}


#endif
