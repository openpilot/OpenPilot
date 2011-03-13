/**
 * \file display_example.hpp
 * \date 25/03/2010
 * \author croussil
 * Example of file defining an empty display architecture
 * Replace all "Ex" by the name of the rendering library it will use
 * \ingroup rtslam
 */

#ifndef DISPLAY_Ex__HPP_
#define DISPLAY_Ex__HPP_

#include "rtslam/display.hpp"


namespace jafar {
namespace rtslam {
namespace display {

	class WorldEx;
	class MapEx;
	class RobotEx;
	class SensorEx;
	class LandmarkEx;
	class ObservationEx;
	
	class ViewerEx: public Viewer<WorldEx,MapEx,RobotEx,SensorEx,LandmarkEx,ObservationEx,boost::variant<int*> >
	{
		public:
			// some configuration parameters
		public:
			ViewerEx()
			{
				// initialize the parameters
			}
	};


	class WorldEx : public WorldDisplay
	{
			ViewerEx *viewerEx;
		public:
			WorldEx(ViewerAbstract *_viewer, rtslam::WorldAbstract *_slamWor, WorldDisplay *garbage): 
				WorldDisplay(_viewer, _slamWor, garbage), viewerEx(PTR_CAST<ViewerEx*>(_viewer)) {}
			void bufferize() {}
			void render() {}
	};

	class MapEx : public MapDisplay
	{
			ViewerEx *viewerEx;
		public:
			MapEx(ViewerAbstract *_viewer, rtslam::MapAbstract *_slamMap, WorldEx *_dispWorld): 
				MapDisplay(_viewer, _slamMap, _dispWorld), viewerEx(PTR_CAST<ViewerEx*>(_viewer)) {}
			void bufferize() {}
			void render() {}
	};

	class RobotEx : public RobotDisplay
	{
			ViewerEx *viewerEx;
		public:
			RobotEx(ViewerAbstract *_viewer, rtslam::RobotAbstract *_slamRob, MapEx *_dispMap): 
				RobotDisplay(_viewer, _slamRob, _dispMap), viewerEx(PTR_CAST<ViewerEx*>(_viewer)) {}
			void bufferize() {}
			void render() {}
	};

	class SensorEx : public SensorDisplay
	{
			ViewerEx *viewerEx;
		public:
			SensorEx(ViewerAbstract *_viewer, rtslam::SensorExteroAbstract *_slamSen, RobotEx *_dispRob): 
				SensorDisplay(_viewer, _slamSen, _dispRob), viewerEx(PTR_CAST<ViewerEx*>(_viewer)) {}
			void bufferize() {}
			void render() {}
	};

	class LandmarkEx : public LandmarkDisplay
	{
			ViewerEx *viewerEx;
		public:
			LandmarkEx(ViewerAbstract *_viewer, rtslam::LandmarkAbstract *_slamLmk, MapEx *_dispMap): 
				LandmarkDisplay(_viewer, _slamLmk, _dispMap), viewerEx(PTR_CAST<ViewerEx*>(_viewer)) {}
			void bufferize() {}
			void render() {}
	};

	class ObservationEx : public ObservationDisplay
	{
			ViewerEx *viewerEx;
		public:
			ObservationEx(ViewerAbstract *_viewer, rtslam::ObservationAbstract *_slamObs, SensorEx *_dispSen): 
				ObservationDisplay(_viewer, _slamObs, _dispSen), viewerEx(PTR_CAST<ViewerEx*>(_viewer)) {}
			void bufferize() {}
			void render() {}
	};


}}}

#endif

