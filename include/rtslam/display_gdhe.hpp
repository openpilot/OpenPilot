/**
 * \file display_gdhe.hpp
 * \author croussil@laas.fr
 * \date 25/03/2010
 * File defining a display architecture for gdhe
 * \ingroup rtslam
 */

#ifndef DISPLAY_GDHE__HPP_
#define DISPLAY_GDHE__HPP_

#include "rtslam/display.hpp"


namespace jafar {
namespace rtslam {
namespace display {

class WorldGdhe : public WorldDisplay
{
	public:
		WorldGdhe(rtslam::WorldAbstract *_slamWor, rtslam::WorldAbstract *garbage): 
			WorldDisplay(_slamWor, garbage) {}
		void bufferize() {}
		void render() {}
};

class MapGdhe : public MapDisplay
{
	public:
		MapGdhe(rtslam::MapAbstract *_slamMap, WorldGdhe *_dispWorld): 
			MapDisplay(_slamMap, _dispWorld) {}
		void bufferize() {}
		void render() {}
};

class RobotGdhe : public RobotDisplay
{
	public:
		RobotGdhe(rtslam::RobotAbstract *_slamRob, MapGdhe *_dispMap): 
			RobotDisplay(_slamRob, _dispMap) {}
		void bufferize() {}
		void render() {}
};

class SensorGdhe : public SensorDisplay
{
	public:
		SensorGdhe(rtslam::SensorAbstract *_slamSen, RobotGdhe *_dispRob): 
			SensorDisplay(_slamSen, _dispRob) {}
		void bufferize() {}
		void render() {}
};

class LandmarkGdhe : public LandmarkDisplay
{
	public:
		LandmarkGdhe(rtslam::LandmarkAbstract *_slamLmk, MapGdhe *_dispMap): 
			LandmarkDisplay(_slamLmk, _dispMap) {}
		void bufferize() {}
		void render() {}
};

class ObservationGdhe : public ObservationDisplay
{
	public:
		ObservationGdhe(rtslam::ObservationAbstract *_slamObs, SensorGdhe *_dispSen): 
			ObservationDisplay(_slamObs, _dispSen) {}
		void bufferize() {}
		void render() {}
};

typedef Viewer<WorldGdhe,MapGdhe,RobotGdhe,SensorGdhe,LandmarkGdhe,ObservationGdhe> ViewerGdhe;

}}}

#endif

