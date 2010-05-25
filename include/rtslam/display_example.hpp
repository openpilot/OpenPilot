/**
 * \file display_example.hpp
 * \author croussil@laas.fr
 * \date 25/03/2010
 * Example of file defining an empty display architecture
 * \ingroup rtslam
 */

#ifndef DISPLAY_EXAMPLE__HPP_
#define DISPLAY_EXAMPLE__HPP_

#include "rtslam/display.hpp"


namespace jafar {
namespace rtslam {
namespace display {

class WorldEx : public WorldDisplay
{
	public:
		WorldEx(rtslam::WorldAbstract *_slamWor, rtslam::WorldAbstract *garbage): 
			WorldDisplay(_slamWor, garbage) {}
		void bufferize() {}
		void render() {}
};

class MapEx : public MapDisplay
{
	public:
		MapEx(rtslam::MapAbstract *_slamMap, WorldEx *_dispWorld): 
			MapDisplay(_slamMap, _dispWorld) {}
		void bufferize() {}
		void render() {}
};

class RobotEx : public RobotDisplay
{
	public:
		RobotEx(rtslam::RobotAbstract *_slamRob, MapEx *_dispMap): 
			RobotDisplay(_slamRob, _dispMap) {}
		void bufferize() {}
		void render() {}
};

class SensorEx : public SensorDisplay
{
	public:
		SensorEx(rtslam::SensorAbstract *_slamSen, RobotEx *_dispRob): 
			SensorDisplay(_slamSen, _dispRob) {}
		void bufferize() {}
		void render() {}
};

class LandmarkEx : public LandmarkDisplay
{
	public:
		LandmarkEx(rtslam::LandmarkAbstract *_slamLmk, MapEx *_dispMap): 
			LandmarkDisplay(_slamLmk, _dispMap) {}
		void bufferize() {}
		void render() {}
};

class ObservationEx : public ObservationDisplay
{
	public:
		ObservationEx(rtslam::ObservationAbstract *_slamObs, SensorEx *_dispSen): 
			ObservationDisplay(_slamObs, _dispSen) {}
		void bufferize() {}
		void render() {}
};

// if you don't need some types of objects, don't declare them above, and put <Object>Display instead of <Object>Ex below
typedef Viewer<WorldEx,MapEx,RobotEx,SensorEx,LandmarkEx,ObservationEx> ViewerEx;

}}}

#endif

