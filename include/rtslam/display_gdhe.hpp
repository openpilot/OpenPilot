/**
 * \file display_gdhe.hpp
 * \date 25/03/2010
 * \author croussil
 * File defining a display architecture for gdhe
 * \ingroup rtslam
 */

#ifndef DISPLAY_GDHE__HPP_
#define DISPLAY_GDHE__HPP_
#include "rtslam/display.hpp"
#ifdef HAVE_MODULE_GDHE

#include "gdhe/client.hpp"

/*
TODO:
- correctly setup scene size
- setup window size
- compression of small movements for trajectories
- draw a cross at target position

---------------------
- [ok] scale ahp spheres to section of ellipses
- [ok] set label shift for ellipses -> difficult to improve in 3d...
- [ok] remove ahp landmark after reparam
- [ok] remove lost landmarks...
- [ok] init is white, not yellow
- [ok] add ellipses and segments for ahp
- [ok] add id
- [ok] find why some euclidean ellipses are not toward the camera...
- [ok] set robot ellipse
*/


namespace jafar {
namespace rtslam {
namespace display {
	
	class WorldGdhe;
	class MapGdhe;
	class RobotGdhe;
	class SensorGdhe;
	class LandmarkGdhe;
	class ObservationGdhe;

	class ViewerGdhe: public Viewer<WorldGdhe,MapGdhe,RobotGdhe,SensorGdhe,LandmarkGdhe,ObservationGdhe,
	                                boost::variant<gdhe::Object*> >
	{
		public:
			double ellipsesScale;
			std::string robot_model;
			gdhe::Client client;
			double extent;
		public:
			ViewerGdhe(std::string _robot_model = "", double _ellipsesScale = 3.0, std::string _host="localhost"):
				ellipsesScale(_ellipsesScale), robot_model(_robot_model), client(_host)
			{
				//client.launch_server();
				client.connect();
				client.clear();
				client.setCameraTarget(0.04,0,0.15);
				client.setCameraPos(80, 20, 0.5);
			}
			void setConvertTempPath(std::string path) { client.setConvertTempPath(path); }
			void dump(std::string filename)
			{
				client.dump(filename);
			}
	};


	class WorldGdhe : public WorldDisplay
	{
			ViewerGdhe *viewerGdhe;
		public:
			WorldGdhe(ViewerAbstract *viewer_, rtslam::WorldAbstract *_slamWor, WorldDisplay *garbage);
			void bufferize() {}
			void render() {}
	};

	class MapGdhe : public MapDisplay
	{
			// bufferized data
			jblas::vec poseQuat;
			// gdhe objects
			ViewerGdhe *viewerGdhe;
			gdhe::Frame *frame;
		public:
			MapGdhe(ViewerAbstract *viewer_, rtslam::MapAbstract *_slamMap, WorldGdhe *_dispWorld);
			~MapGdhe();
			void bufferize();
			void render();
	};

	class RobotGdhe : public RobotDisplay
	{
			// bufferized data
			jblas::vec poseQuat;
			jblas::sym_mat poseQuatUncert;
			// gdhe objects
			ViewerGdhe *viewerGdhe;
			gdhe::Robot *robot;
			gdhe::EllipsoidWire *uncertEll;
			gdhe::Trajectory *traj;
		public:
			RobotGdhe(ViewerAbstract *viewer_, rtslam::RobotAbstract *_slamRob, MapGdhe *_dispMap);
			~RobotGdhe();
			void bufferize();
			void render();
	};

	class SensorGdhe : public SensorDisplay
	{
			ViewerGdhe *viewerGdhe;
		public:
			SensorGdhe(ViewerAbstract *viewer_, rtslam::SensorExteroAbstract *_slamSen, RobotGdhe *_dispRob);
			void bufferize() {}
			void render() {}
	};

	class LandmarkGdhe : public LandmarkDisplay
	{
			// buffered data
			ObservationAbstract::Events events_;
/*			bool predicted_;
			bool visible_;
			bool measured_;
			bool matched_;
			bool updated_;
*/		
			jblas::vec state_;
			jblas::sym_mat cov_;
			unsigned int id_;
			LandmarkAbstract::type_enum lmkType_;
			// gdhe objects
			ViewerGdhe *viewerGdhe;
			typedef std::list<gdhe::Object*> ItemList;
			ItemList items_;
		public:
			LandmarkGdhe(ViewerAbstract *viewer_, rtslam::LandmarkAbstract *_slamLmk, MapGdhe *_dispMap);
			~LandmarkGdhe();
			void bufferize();
			void render();
	};

	class ObservationGdhe : public ObservationDisplay
	{
			ViewerGdhe *viewerGdhe;
		public:
			ObservationGdhe(ViewerAbstract *viewer_, rtslam::ObservationAbstract *_slamObs, SensorGdhe *_dispSen);
			void bufferize() {}
			void render() {}
	};


}}}

#endif
#endif

