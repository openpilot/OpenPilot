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

	/** **************************************************************************
	This is the base class for objects that will store display data for a viewer
	\ingroup rtslam
	*/
	class DisplayDataAbstract
	{
		// bufferized slam object data
		// +
		// display objects
		public:
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
			WorldDisplay(rtslam::WorldAbstract *_slamWor, WorldDisplay *_garbage):
				slamWor_(_slamWor) {}
	};
	
	/** **************************************************************************
	\ingroup rtslam
	*/
	class MapDisplay : public DisplayDataAbstract
	{
		public:
			rtslam::MapAbstract *slamMap_;
			WorldDisplay *dispWorld_;
			MapDisplay(rtslam::MapAbstract *_slamMap, WorldDisplay *_dispWorld): 
				slamMap_(_slamMap), dispWorld_(_dispWorld) {}
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
			RobotDisplay(rtslam::RobotAbstract *_slamRob, MapDisplay *_dispMap): 
				slamRob_(_slamRob), dispMap_(_dispMap) {}
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
			enum Type { stCameraPinhole, stCameraBarreto };
			Type type_;
			static Type convertType(rtslam::SensorAbstract *_slamSen)
			{
				// FIXME SensorAbstract should provide a type enum to avoid these dynamic casts
				if (dynamic_cast<SensorPinHole*>(_slamSen))
					return stCameraPinhole;
				else
					JFR_ERROR(RtslamException, RtslamException::UNKNOWN_SENSOR_TYPE, "Don't know how to display this type of sensor");
			}
			SensorDisplay(rtslam::SensorAbstract *_slamSen, RobotDisplay *_dispRobot): 
				slamSen_(_slamSen), dispRobot_(_dispRobot), type_(convertType(_slamSen)) {}
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
			LandmarkDisplay(rtslam::LandmarkAbstract *_slamLmk, MapDisplay *_dispMap): 
				slamLmk_(_slamLmk), dispMap_(_dispMap), type_(convertType(_slamLmk)) {}
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
			SensorDisplay::Type sensorType_;
			LandmarkDisplay::Type  landmarkGeomType_;
			LandmarkDisplay::Phase landmarkPhase_;
			ObservationDisplay(rtslam::ObservationAbstract *_slamObs, SensorDisplay *_dispSen): 
				slamObs_(_slamObs), dispSen_(_dispSen)
			{
				sensorType_ = SensorDisplay::convertType(&*_slamObs->sensorPtr());
				landmarkGeomType_ = LandmarkDisplay::convertType(&*_slamObs->landmarkPtr());
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
							parentDisp = static_cast<ParentDisplayType*>(parentSlam->displayData[id]);
					}
					slamObject->displayData[id] = new DisplayType(&*slamObject, parentDisp);
				}
				// bufferize the object
				DisplayType *objDisp = static_cast<DisplayType*>(slamObject->displayData[id]);
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
	
	
	/** **************************************************************************
	This is the base class for a viewer that can render the scene.
	When writing a new viewer, it must be inherited from this.
	*/
	template<class WorldDisplayType, class MapDisplayType, class RobotDisplayType, 
		class SensorDisplayType, class LandmarkDisplayType, class ObservationDisplayType>
	class Viewer : public ViewerAbstract
	{
		protected:
			//template<class RobotDisplayType, class SensorDisplayType, class LandmarkDisplayType, class ObservationDisplayType>
			class Render : public boost::static_visitor<>// : public RenderAbstract
			{
				public:
					typedef Viewer<WorldDisplayType, MapDisplayType, RobotDisplayType, 
						SensorDisplayType, LandmarkDisplayType, ObservationDisplayType> MyViewer;
					
					void operator()(rtslam::world_ptr_t const &wor) const {
						if (!boost::is_same<WorldDisplayType,WorldDisplay>::value) {
							WorldDisplayType &worDisp = *static_cast<WorldDisplayType*>(wor->displayData[MyViewer::id()]);
							worDisp.render();
						}
					}
					void operator()(rtslam::map_ptr_t const &map) const {
						if (!boost::is_same<MapDisplayType,MapDisplay>::value) {
							RobotDisplayType &mapDisp = *static_cast<RobotDisplayType*>(map->displayData[MyViewer::id()]);
							mapDisp.render();
						}
					}
					void operator()(rtslam::robot_ptr_t const &rob) const {
						if (!boost::is_same<RobotDisplayType,RobotDisplay>::value) {
							RobotDisplayType &robDisp = *static_cast<RobotDisplayType*>(rob->displayData[MyViewer::id()]);
							robDisp.render();
						}
					}
					void operator()(rtslam::sensor_ptr_t const &sen) const {
						if (!boost::is_same<SensorDisplayType,SensorDisplay>::value) {
							SensorDisplayType &senDisp = *static_cast<SensorDisplayType*>(sen->displayData[MyViewer::id()]);
							senDisp.render();
						}
					}
					void operator()(rtslam::landmark_ptr_t const &lmk) const {
						if (!boost::is_same<LandmarkDisplayType,LandmarkDisplay>::value) {
							LandmarkDisplayType &lmkDisp = *static_cast<LandmarkDisplayType*>(lmk->displayData[MyViewer::id()]);
							lmkDisp.render();
						}
					}
					void operator()(rtslam::observation_ptr_t const &obs) const {
						if (!boost::is_same<ObservationDisplayType,ObservationDisplay>::value) {
							ObservationDisplayType &obsDisp = *static_cast<ObservationDisplayType*>(obs->displayData[MyViewer::id()]);
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
			//ViewerRender(): ViewerAbstract() {}
			inline void clear()
			{
				slamObjects_.clear();
			}
			
			/**
			This function bufferizes all the objects
			*/
			inline void bufferize(rtslam::world_ptr_t wor)
			{
				// clear viewer
				clear();
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
					bufferizeObject<RobotDisplayType, MapDisplay, robot_ptr_t, map_ptr_t>(rob, map, id());
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
				// bufferize observation
				if (!boost::is_same<ObservationDisplayType,ObservationDisplay>::value) // bufferize observation
					bufferizeObject<ObservationDisplayType, SensorDisplayType, observation_ptr_t, sensor_ptr_t>(obs, sen, id());
			}

			inline void bufferize(rtslam::landmark_ptr_t lmk, rtslam::map_ptr_t map)
			{
				// bufferize landmark
				if (!boost::is_same<LandmarkDisplayType,LandmarkDisplay>::value) // bufferize landmark
					bufferizeObject<LandmarkDisplayType, MapDisplay, landmark_ptr_t, map_ptr_t>(lmk, map, id());
			}
			
			

			/**
			Render the scene.
			@param _clear 
			*/
			inline void render()
			{
				for(SlamObjectsList::iterator it = slamObjects_.begin(); it != slamObjects_.end(); ++it)
					boost::apply_visitor(Render(), *it);
				//clear(); // we're not in a hurry, it will be done at next render
			}
			
	};

		typedef enum lmk_state {
			lmk_state_init,
			lmk_state_converged
		} lmk_state ;
		typedef enum lmk_state_advanced {
			lmk_state_advanced_not_predicted,
			lmk_state_advanced_predicted,
			lmk_state_advanced_matched,
			lmk_state_advanced_updated
		} lmk_state_advanced ;
		typedef enum color {
			color_transparency,
			color_yellow,
			color_magenta,
			color_blue,
			color_red,
			color_cyan,
			color_rose
		} color ;
		typedef struct colorRGB  {
				int R;
				int G;
				int B;
		} colorRGB ;
		static color getColorObject_prediction(lmk_state lmk_state, lmk_state_advanced lmk_state_advanced) {

			if (1==1)
				return color_rose ;

			if (lmk_state==lmk_state_init) {
				switch (lmk_state_advanced) {
					case lmk_state_advanced_not_predicted:
						return color_yellow;
						break;
					case lmk_state_advanced_predicted:
						return color_magenta;
						break;
					case lmk_state_advanced_matched:
						return color_magenta;
						break;
					case lmk_state_advanced_updated:
						return color_red;
						break;
					default:
						break;
				}
			}
			if (lmk_state==lmk_state_converged) {
				switch (lmk_state_advanced) {
					case lmk_state_advanced_not_predicted:
						return color_red;
						break;
					case lmk_state_advanced_predicted:
						return color_blue;
						break;
					case lmk_state_advanced_matched:
						return color_blue;
						break;
					case lmk_state_advanced_updated:
						return color_cyan;
						break;
					default:
						break;
				}
			}
			cout << "warning color undefined for object types in display" << endl ;
			return color_transparency;
		}
		static color getColorObject_measure    (lmk_state lmk_state, lmk_state_advanced lmk_state_advanced) {

			if (1==1)
				return color_rose ;

			if (lmk_state==lmk_state_init) {
				switch (lmk_state_advanced) {
					case lmk_state_advanced_not_predicted:
						return color_yellow;
						break;
					case lmk_state_advanced_predicted:
						return color_transparency;
						break;
					case lmk_state_advanced_matched:
						return color_red;
						break;
					case lmk_state_advanced_updated:
						return color_red;
						break;
					default:
						break;
				}
			}
			if (lmk_state==lmk_state_converged) {
				switch (lmk_state_advanced) {
					case lmk_state_advanced_not_predicted:
						return color_transparency;
						break;
					case lmk_state_advanced_predicted:
						return color_transparency;
						break;
					case lmk_state_advanced_matched:
						return color_cyan;
						break;
					case lmk_state_advanced_updated:
						return color_cyan;
						break;
					default:
						break;
				}
			}
			cout << "warning color undefined for object types in display" << endl ;
			return color_transparency;
		}
		colorRGB getColorRGB(color colorOrigin)  {
			colorRGB result ;
			result.R = 0 ;
			result.G = 0 ;
			result.B = 0 ;
			switch (colorOrigin) {
				case color_blue:
					result.B = 255 ;
					break;
				case color_cyan:
					result.G = 255 ;
					result.B = 255 ;
					break;
				case color_magenta:
					result.R = 219 ;
					result.B = 115 ;
					break;
				case color_red:
					result.R = 255 ;
					break;
				case color_transparency:
					break;
				case color_yellow:
					result.R = 255 ;
					result.G = 255 ;
					break;
				case color_rose:
					result.R = 253 ;
					result.G = 103 ;
					result.B = 223 ;
					break;
				default:
					break;
			}
			return result ;
		}

}}}


#endif
